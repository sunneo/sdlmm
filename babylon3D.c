#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

// SIMD intrinsics for hardware acceleration
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    #define USE_SIMD_X86
    #include <emmintrin.h>  // SSE2
    #ifdef __AVX__
        #include <immintrin.h>  // AVX
    #endif
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
    #define USE_SIMD_ARM
    #include <arm_neon.h>
#endif
typedef struct Vector2{
   float x,y;
}Vector2;

typedef struct Vector3{
   float x,y,z;
}Vector3;

typedef struct Matrix{
   float m[16];
}Matrix;


static Vector2 vector2(float x,float y);
static char* vector2_tostring(const Vector2* vec,char* buf,int size);
static Vector2 vector2_add(const Vector2* a,const Vector2* b);
static Vector2 vector2_subtract(const Vector2* a,const Vector2* b);
static Vector2 vector2_negate(const Vector2* a);
static Vector2 vector2_scale(const Vector2* a,float s);
static int vector2_equals(const Vector2* a,const Vector2* b);
static float vector2_length(const Vector2* a);
static float vector2_lengthSquare(const Vector2* a);
static void vector2_normalize(Vector2* a);
static Vector2 vector2_zero();
static Vector2 vector2_copy(const Vector2* a);
static Vector2 vector2_normalize_copy(const Vector2* a);
static Vector2 vector2_min(const Vector2* a, const Vector2* b);
static Vector2 vector2_max(const Vector2* a, const Vector2* b);
static Vector2 vector2_transform(const Vector2* a, const Matrix* transformation) ;
static float vector2_distanceSquared(const Vector2* a,const Vector2* b);
static float vector2_distance(const Vector2* a,const Vector2* b);

static Vector3 vector3(float x,float y,float z);
static const char* vector3_tostring(const Vector3* a,char* buf,int sz);
static Vector3 vector3_add(const Vector3* a,const Vector3* b);
static Vector3 vector3_subtract(const Vector3* a,const Vector3* b);
static Vector3 vector3_negate(const Vector3* a);
static Vector3 vector3_scale(const Vector3* a,float s);
static int vector3_equals(const Vector3* a,const Vector3* b);
static Vector3 vector3_multiply(const Vector3* a,const Vector3* b);
static Vector3 vector3_divide(const Vector3* a,const Vector3* b);
static float vector3_lengthSquare(const Vector3* a);
static float vector3_length(const Vector3* a);
static void vector3_normalize(Vector3* a);
static Vector3 vector3_fromArray(float* f,int offset);
static Vector3 vector3_zero();
static Vector3 vector3_up();
static Vector3 vector3_copy(const Vector3* a);
static Vector3 vector3_transform_coordinates(const Vector3* vector,const Matrix* transformation);
static Vector3 vector3_transform_normal(const Vector3* vector, const Matrix* transformation) ;
static float vector3_dot(const Vector3* left,const Vector3* right);
static Vector3 vector3_cross(const Vector3* left,const Vector3* right);
static Vector3 vector3_normalize_copy(const Vector3* vector);
static float vector3_distance(const Vector3* value1,const Vector3*  value2);
static float vector3_distanceSquared(const Vector3* value1,const Vector3*  value2);

static Matrix matrix();
static matrix_isIdentity(const Matrix* a);
static float matrix_determinant(const Matrix* a);
static float* matrix_toArray(Matrix* a);
static void matrix_invert(Matrix* a) ;
static Matrix matrix_multiply(const Matrix* a,const Matrix* other) ;
static int matrix_equals(const Matrix* a,const Matrix* value) ;
static Matrix matrix_fromValues(float initialM11,float  initialM12,float  initialM13,float  initialM14,float  initialM21,float  initialM22, float initialM23,float  initialM24,float  initialM31,float  initialM32,float  initialM33,float  initialM34,float  initialM41,float  initialM42,float  initialM43,float  initialM44) ;
static Matrix matrix_Identity();
static Matrix matrix_Zero() ;
static Matrix matrix_Copy(const Matrix* source) ;
static Matrix matrix_RotationX(float angle) ;
static Matrix matrix_RotationY(float angle) ;
static Matrix matrix_RotationZ(float angle);
static Matrix matrix_RotationAxis(Vector3* axis, float angle) ; // FIXME
static Matrix matrix_RotationYawPitchRoll(float yaw,float pitch,float roll) ;
static Matrix matrix_scaling(float x, float y, float z) ;
static Matrix matrix_Translation(float x,float y,float z) ;
static Matrix matrix_LookAtLH(const Vector3* eye,const Vector3* target,const Vector3* up) ;
static Matrix matrix_PerspectiveLH(float width,float height,float znear,float zfar);
static Matrix matrix_PerspectiveFovLH(float fov,float aspect,float znear,float zfar) ;
static Matrix matrix_Transpose(const Matrix* matrix) ;


static Vector2 vector2(float x,float y){
    Vector2 ret;
    ret.x=x; ret.y=y;
    return ret; 
}

static char* vector2_tostring(const Vector2* vec,char* buf,int size){
    snprintf(buf,size,"{X: %f,Y: %f}",vec->x,vec->y);
    return buf;
}

static Vector2 vector2_add(const Vector2* a,const Vector2* b){
    return vector2(a->x+b->x,a->y+b->y);
}
static Vector2 vector2_subtract(const Vector2* a,const Vector2* b){
    return vector2(a->x-b->x,a->y-b->y);
}
static Vector2 vector2_negate(const Vector2* a){
    return vector2(-a->x,-a->y);
}
static Vector2 vector2_scale(const Vector2* a,float s){
    return vector2(a->x*s,a->y*s);
}
static int vector2_equals(const Vector2* a,const Vector2* b){
    return a->x == b->x && a->y == b->y;
}
static float vector2_length(const Vector2* a){
    return sqrt(a->x * a->x + a->y * a->y);
}
static float vector2_lengthSquare(const Vector2* a){
    return a->x * a->x + a->y * a->y;
}
static void vector2_normalize(Vector2* a) {
    float len = vector2_length(a);
    if(len == 0) {  return;   }
    float num = 1.0 / len;
    a->x *= num; a->y *= num;
}

static Vector2 vector2_zero(){
    return vector2(0,0);
}
static Vector2 vector2_copy(const Vector2* a){
    return vector2(a->x,a->y); 
}
static Vector2 vector2_normalize_copy(const Vector2* a){
    Vector2 ret = vector2_copy(a);
    vector2_normalize(&ret);
    return ret;
}

static Vector2 vector2_min(const Vector2* a, const Vector2* b){
    return vector2(a->x<b->x?a->x:b->x,a->y<b->y?a->y:b->y);
}
static Vector2 vector2_max(const Vector2* a, const Vector2* b){
    return vector2(a->x>b->x?a->x:b->x,a->y>b->y?a->y:b->y);
}
        
static Vector2 vector2_transform(const Vector2* a, const Matrix* transformation) {
    return vector2( 
              (a->x * transformation->m[0]) + (a->y * transformation->m[4]),
              (a->x * transformation->m[1]) + (a->y * transformation->m[5])
            );
}
static float vector2_distanceSquared(const Vector2* a,const Vector2* b){
    float x=a->x-b->x;
    float y=a->y-b->y;
    return (x*x)+(y*y);
}
static float vector2_distance(const Vector2* a,const Vector2* b){
    return sqrt(vector2_distanceSquared(a,b));
}




Vector3 vector3(float x,float y,float z){
    Vector3 ret;
    ret.x=x; ret.y=y; ret.z=z;
    return ret;
}
static const char* vector3_tostring(const Vector3* a,char* buf,int sz){
    snprintf(buf,sz,"{X: %f Y:%f Z:%f}",a->x,a->y,a->z);
    return buf;
}

static Vector3 vector3_add(const Vector3* a,const Vector3* b){
    return vector3(a->x+b->x,a->y+b->y,a->z+b->z);
}
static Vector3 vector3_subtract(const Vector3* a,const Vector3* b){
    return vector3(a->x-b->x,a->y-b->y,a->z-b->z);
}
static Vector3 vector3_negate(const Vector3* a){
    return vector3(-a->x,-a->y,-a->z);
}
static Vector3 vector3_scale(const Vector3* a,float s){
    return vector3(a->x*s,a->y*s,a->z*s);
}
static int vector3_equals(const Vector3* a,const Vector3* b){
    return a->x == b->x && a->y == b->y && a->z==b->z;
}
static Vector3 vector3_multiply(const Vector3* a,const Vector3* b){
    return vector3(a->x * b->x, a->y * b->y, a->z * b->z);
}
static Vector3 vector3_divide(const Vector3* a,const Vector3* b){
    return vector3(a->x / b->x, a->y / b->y, a->z / b->z);
}
static float vector3_lengthSquare(const Vector3* a){
        return (a->x * a->x + a->y *a->y + a->z * a->z);
}
static float vector3_length(const Vector3* a){
        return sqrt(a->x * a->x + a->y *a->y + a->z * a->z);
}
static void vector3_normalize(Vector3* a){
        float len = vector3_length(a);
        if(len == 0) {  return;   }
        float num = 1.0 / len;
        a->x *= num;
        a->y *= num;
        a->z *= num;
}
static Vector3 vector3_fromArray(float* f,int offset){
    return vector3(f[offset],f[offset+1],f[offset+2]);
}
Vector3 vector3_zero(){
    return vector3(0,0,0);
}
static Vector3 vector3_up(){
    return vector3(0,1.0,0);
}


static Vector3 vector3_copy(const Vector3* a){
    return vector3(a->x,a->y,a->z);
}
static Vector3 vector3_transform_coordinates(const Vector3* vector,const Matrix* transformation){
    float x = (vector->x * transformation->m[0]) + (vector->y * transformation->m[4]) + (vector->z * transformation->m[8]) + transformation->m[12];
    float y = (vector->x * transformation->m[1]) + (vector->y * transformation->m[5]) + (vector->z * transformation->m[9]) + transformation->m[13];
    float z = (vector->x * transformation->m[2]) + (vector->y * transformation->m[6]) + (vector->z * transformation->m[10]) + transformation->m[14];
    float w = (vector->x * transformation->m[3]) + (vector->y * transformation->m[7]) + (vector->z * transformation->m[11]) + transformation->m[15];
    return vector3(x / w, y / w, z / w);
}
static Vector3 vector3_transform_normal(const Vector3* vector, const Matrix* transformation) {
    float x = (vector->x * transformation->m[0]) + (vector->y * transformation->m[4]) + (vector->z * transformation->m[8]);
    float y = (vector->x * transformation->m[1]) + (vector->y * transformation->m[5]) + (vector->z * transformation->m[9]);
    float z = (vector->x * transformation->m[2]) + (vector->y * transformation->m[6]) + (vector->z * transformation->m[10]);
    return vector3(x, y, z);
}
static float vector3_dot(const Vector3* left,const Vector3* right) {
    return (left->x * right->x + left->y * right->y + left->z * right->z);
}
static Vector3 vector3_cross(const Vector3* left,const Vector3* right)  {
    float x = left->y * right->z - left->z * right->y;
    float y = left->z * right->x - left->x * right->z;
    float z = left->x * right->y - left->y * right->x;
    return vector3(x, y, z);
};
Vector3 vector3_normalize_copy(const Vector3* vector) {
    Vector3 newvector = vector3_copy(vector);
    vector3_normalize(&newvector);
    return newvector;
}

static float vector3_distance(const Vector3* value1,const Vector3*  value2) {
    return sqrt(vector3_distanceSquared(value1, value2));
}

static float vector3_distanceSquared(const Vector3* value1,const Vector3*  value2) {
    float x = value1->x - value2->x;
    float y = value1->y - value2->y;
    float z = value1->z - value2->z;
    return (x * x) + (y * y) + (z * z);
}



static Matrix matrix(){
    Matrix ret;
    memset(&ret,0,sizeof(Matrix));
    return ret;
}
static matrix_isIdentity(const Matrix* a){
    if(a->m[0] != 1.0 || a->m[5] != 1.0 || a->m[10] != 1.0 || a->m[15] != 1.0) return 0;
    if(a->m[12] != 0.0 || a->m[13] != 0.0 || a->m[14] != 0.0 || a->m[4] != 0.0 || a->m[6] != 0.0 || a->m[7] != 0.0 || a->m[8] != 0.0 || a->m[9] != 0.0 || a->m[11] != 0.0 || a->m[12] != 0.0 || a->m[13] != 0.0 || a->m[14] != 0.0)  return 0;
    return 1;
}
static float matrix_determinant(const Matrix* a){
    float temp1 = (a->m[10] * a->m[15]) - (a->m[11] * a->m[14]);
    float temp2 = (a->m[9] * a->m[15]) - (a->m[11] * a->m[13]);
    float temp3 = (a->m[9] * a->m[14]) - (a->m[10] * a->m[13]);
    float temp4 = (a->m[8] * a->m[15]) - (a->m[11] * a->m[12]);
    float temp5 = (a->m[8] * a->m[14]) - (a->m[10] * a->m[12]);
    float temp6 = (a->m[8] * a->m[13]) - (a->m[9] * a->m[12]);
    return ((((a->m[0] * (((a->m[5] * temp1) - (a->m[6] * temp2)) + (a->m[7] * temp3))) - (a->m[1] * (((a->m[4] * temp1) - (a->m[6] * temp4)) + (a->m[7] * temp5)))) + (a->m[2] * (((a->m[4] * temp2) - (a->m[5] * temp4)) + (a->m[7] * temp6)))) - (a->m[3] * (((a->m[4] * temp3) - (a->m[5] * temp5)) + (a->m[6] * temp6))));
}
static float* matrix_toArray(Matrix* a){
    return a->m;
}
static void matrix_invert(Matrix* a) {
    float l1 = a->m[0];
    float l2 = a->m[1];
    float l3 = a->m[2];
    float l4 = a->m[3];
    float l5 = a->m[4];
    float l6 = a->m[5];
    float l7 = a->m[6];
    float l8 = a->m[7];
    float l9 = a->m[8];
    float l10 = a->m[9];
    float l11 = a->m[10];
    float l12 = a->m[11];
    float l13 = a->m[12];
    float l14 = a->m[13];
    float l15 = a->m[14];
    float l16 = a->m[15];
    float l17 = (l11 * l16) - (l12 * l15);
    float l18 = (l10 * l16) - (l12 * l14);
    float l19 = (l10 * l15) - (l11 * l14);
    float l20 = (l9 * l16) - (l12 * l13);
    float l21 = (l9 * l15) - (l11 * l13);
    float l22 = (l9 * l14) - (l10 * l13);
    float l23 = ((l6 * l17) - (l7 * l18)) + (l8 * l19);
    float l24 = -(((l5 * l17) - (l7 * l20)) + (l8 * l21));
    float l25 = ((l5 * l18) - (l6 * l20)) + (l8 * l22);
    float l26 = -(((l5 * l19) - (l6 * l21)) + (l7 * l22));
    float l27 = 1.0 / ((((l1 * l23) + (l2 * l24)) + (l3 * l25)) + (l4 * l26));
    float l28 = (l7 * l16) - (l8 * l15);
    float l29 = (l6 * l16) - (l8 * l14);
    float l30 = (l6 * l15) - (l7 * l14);
    float l31 = (l5 * l16) - (l8 * l13);
    float l32 = (l5 * l15) - (l7 * l13);
    float l33 = (l5 * l14) - (l6 * l13);
    float l34 = (l7 * l12) - (l8 * l11);
    float l35 = (l6 * l12) - (l8 * l10);
    float l36 = (l6 * l11) - (l7 * l10);
    float l37 = (l5 * l12) - (l8 * l9);
    float l38 = (l5 * l11) - (l7 * l9);
    float l39 = (l5 * l10) - (l6 * l9);
    a->m[0] = l23 * l27;
    a->m[4] = l24 * l27;
    a->m[8] = l25 * l27;
    a->m[12] = l26 * l27;
    a->m[1] = -(((l2 * l17) - (l3 * l18)) + (l4 * l19)) * l27;
    a->m[5] = (((l1 * l17) - (l3 * l20)) + (l4 * l21)) * l27;
    a->m[9] = -(((l1 * l18) - (l2 * l20)) + (l4 * l22)) * l27;
    a->m[13] = (((l1 * l19) - (l2 * l21)) + (l3 * l22)) * l27;
    a->m[2] = (((l2 * l28) - (l3 * l29)) + (l4 * l30)) * l27;
    a->m[6] = -(((l1 * l28) - (l3 * l31)) + (l4 * l32)) * l27;
    a->m[10] = (((l1 * l29) - (l2 * l31)) + (l4 * l33)) * l27;
    a->m[14] = -(((l1 * l30) - (l2 * l32)) + (l3 * l33)) * l27;
    a->m[3] = -(((l2 * l34) - (l3 * l35)) + (l4 * l36)) * l27;
    a->m[7] = (((l1 * l34) - (l3 * l37)) + (l4 * l38)) * l27;
    a->m[11] = -(((l1 * l35) - (l2 * l37)) + (l4 * l39)) * l27;
    a->m[15] = (((l1 * l36) - (l2 * l38)) + (l3 * l39)) * l27;
}


static Matrix matrix_multiply(const Matrix* a,const Matrix* other) {
    Matrix result;
    result.m[0] = a->m[0] * other->m[0] + a->m[1] * other->m[4] + a->m[2] * other->m[8] + a->m[3] * other->m[12];
    result.m[1] = a->m[0] * other->m[1] + a->m[1] * other->m[5] + a->m[2] * other->m[9] + a->m[3] * other->m[13];
    result.m[2] = a->m[0] * other->m[2] + a->m[1] * other->m[6] + a->m[2] * other->m[10] + a->m[3] * other->m[14];
    result.m[3] = a->m[0] * other->m[3] + a->m[1] * other->m[7] + a->m[2] * other->m[11] + a->m[3] * other->m[15];
    result.m[4] = a->m[4] * other->m[0] + a->m[5] * other->m[4] + a->m[6] * other->m[8] + a->m[7] * other->m[12];
    result.m[5] = a->m[4] * other->m[1] + a->m[5] * other->m[5] + a->m[6] * other->m[9] + a->m[7] * other->m[13];
    result.m[6] = a->m[4] * other->m[2] + a->m[5] * other->m[6] + a->m[6] * other->m[10] + a->m[7] * other->m[14];
    result.m[7] = a->m[4] * other->m[3] + a->m[5] * other->m[7] + a->m[6] * other->m[11] + a->m[7] * other->m[15];
    result.m[8] = a->m[8] * other->m[0] + a->m[9] * other->m[4] + a->m[10] * other->m[8] + a->m[11] * other->m[12];
    result.m[9] = a->m[8] * other->m[1] + a->m[9] * other->m[5] + a->m[10] * other->m[9] + a->m[11] * other->m[13];
    result.m[10] = a->m[8] * other->m[2] + a->m[9] * other->m[6] + a->m[10] * other->m[10] + a->m[11] * other->m[14];
    result.m[11] = a->m[8] * other->m[3] + a->m[9] * other->m[7] + a->m[10] * other->m[11] + a->m[11] * other->m[15];
    result.m[12] = a->m[12] * other->m[0] + a->m[13] * other->m[4] + a->m[14] * other->m[8] + a->m[15] * other->m[12];
    result.m[13] = a->m[12] * other->m[1] + a->m[13] * other->m[5] + a->m[14] * other->m[9] + a->m[15] * other->m[13];
    result.m[14] = a->m[12] * other->m[2] + a->m[13] * other->m[6] + a->m[14] * other->m[10] + a->m[15] * other->m[14];
    result.m[15] = a->m[12] * other->m[3] + a->m[13] * other->m[7] + a->m[14] * other->m[11] + a->m[15] * other->m[15];
    return result;
}

static int matrix_equals(const Matrix* a,const Matrix* value) {
   return (a->m[0] == value->m[0] && a->m[1] == value->m[1] && a->m[2] == value->m[2] && a->m[3] == value->m[3] && a->m[4] == value->m[4] && a->m[5] == value->m[5] && a->m[6] == value->m[6] && a->m[7] == value->m[7] && a->m[8] == value->m[8] && a->m[9] == value->m[9] && a->m[10] == value->m[10] && a->m[11] == value->m[11] && a->m[12] == value->m[12] && a->m[13] == value->m[13] && a->m[14] == value->m[14] && a->m[15] == value->m[15]);
}



static Matrix matrix_fromValues(float initialM11,float  initialM12,float  initialM13,float  initialM14,float  initialM21,float  initialM22, float initialM23,float  initialM24,float  initialM31,float  initialM32,float  initialM33,float  initialM34,float  initialM41,float  initialM42,float  initialM43,float  initialM44) {
   Matrix result;
   result.m[0] = initialM11;
   result.m[1] = initialM12;
   result.m[2] = initialM13;
   result.m[3] = initialM14;
   result.m[4] = initialM21;
   result.m[5] = initialM22;
   result.m[6] = initialM23;
   result.m[7] = initialM24;
   result.m[8] = initialM31;
   result.m[9] = initialM32;
   result.m[10] = initialM33;
   result.m[11] = initialM34;
   result.m[12] = initialM41;
   result.m[13] = initialM42;
   result.m[14] = initialM43;
   result.m[15] = initialM44;
   return result;
}

static Matrix matrix_Identity() {
   return matrix_fromValues(1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0);
}
static Matrix matrix_Zero() {
   return matrix_fromValues(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
static Matrix matrix_Copy(const Matrix* source) {
   return matrix_fromValues(source->m[0], source->m[1], source->m[2], source->m[3], source->m[4], source->m[5], source->m[6], source->m[7], source->m[8], source->m[9], source->m[10], source->m[11], source->m[12], source->m[13], source->m[14], source->m[15]);
}

static Matrix matrix_RotationX(float angle) {
   Matrix result = matrix_Zero();
   float s = sin(angle);
   float c = cos(angle);
   result.m[0] = 1.0;
   result.m[15] = 1.0;
   result.m[5] = c;
   result.m[10] = c;
   result.m[9] = -s;
   result.m[6] = s;
   return result;
}
static Matrix matrix_RotationY(float angle) {
    Matrix result = matrix_Zero();
    float s = sin(angle);
    float c = cos(angle);
    result.m[5] = 1.0;
    result.m[15] = 1.0;
    result.m[0] = c;
    result.m[2] = -s;
    result.m[8] = s;
    result.m[10] = c;
    return result;
}
static Matrix matrix_RotationZ(float angle) {
    Matrix result = matrix_Zero();
    float s = sin(angle);
    float c = cos(angle);
    result.m[10] = 1.0;
    result.m[15] = 1.0;
    result.m[0] = c;
    result.m[1] = s;
    result.m[4] = -s;
    result.m[5] = c;
    return result;
}


static Matrix matrix_RotationAxis(Vector3* axis, float angle) {
    float s = sin(-angle);
    float c = cos(-angle);
    float c1 = 1 - c;
    vector3_normalize(axis);
    Matrix result = matrix_Zero();
    result.m[0] = (axis->x * axis->x) * c1 + c;
    result.m[1] = (axis->x * axis->y) * c1 - (axis->z * s);
    result.m[2] = (axis->x * axis->z) * c1 + (axis->y * s);
    result.m[3] = 0.0;
    result.m[4] = (axis->y * axis->x) * c1 + (axis->z * s);
    result.m[5] = (axis->y * axis->y) * c1 + c;
    result.m[6] = (axis->y * axis->z) * c1 - (axis->x * s);
    result.m[7] = 0.0;
    result.m[8] = (axis->z * axis->x) * c1 - (axis->y * s);
    result.m[9] = (axis->z * axis->y) * c1 + (axis->x * s);
    result.m[10] = (axis->z * axis->z) * c1 + c;
    result.m[11] = 0.0;
    result.m[15] = 1.0;
    return result;
};
static Matrix matrix_RotationYawPitchRoll(float yaw,float pitch,float roll) {
    Matrix rotationZ=matrix_RotationZ(roll);
    Matrix rotationX=matrix_RotationX(pitch);
    Matrix rotationY=matrix_RotationY(yaw);
    //return Matrix.RotationZ(roll).multiply(Matrix.RotationX(pitch)).multiply(Matrix.RotationY(yaw));
    Matrix res1=matrix_multiply(&rotationZ,&rotationX);
    return matrix_multiply(&res1,&rotationY);;
};
static Matrix matrix_scaling(float x, float y, float z) {
    Matrix result = matrix_Zero();
    result.m[0] = x;
    result.m[5] = y;
    result.m[10] = z;
    result.m[15] = 1.0;
    return result;
}
static Matrix matrix_Translation(float x,float y,float z) {
    Matrix result = matrix_Identity();
    result.m[12] = x;
    result.m[13] = y;
    result.m[14] = z;
    return result;
}

static Matrix matrix_LookAtLH(const Vector3* eye,const Vector3* target,const Vector3* up) {
    Vector3 zaxis = vector3_subtract(target,eye);
    vector3_normalize(&zaxis);
    Vector3 xaxis = vector3_cross(up, &zaxis);
    vector3_normalize(&xaxis);
    Vector3 yaxis = vector3_cross(&zaxis, &xaxis);
    vector3_normalize(&yaxis);
    float ex = -vector3_dot(&xaxis, eye);
    float ey = -vector3_dot(&yaxis, eye);
    float ez = -vector3_dot(&zaxis, eye);
    return matrix_fromValues(xaxis.x, yaxis.x, zaxis.x, 0, xaxis.y, yaxis.y, zaxis.y, 0, xaxis.z, yaxis.z, zaxis.z, 0, ex, ey, ez, 1);
}


static Matrix matrix_PerspectiveLH(float width,float height,float znear,float zfar) {
    Matrix matrix = matrix_Zero();
    matrix.m[0] = (2.0 * znear) / width;
    matrix.m[1] = matrix.m[2] = matrix.m[3] = 0.0;
    matrix.m[5] = (2.0 * znear) / height;
    matrix.m[4] = matrix.m[6] = matrix.m[7] = 0.0;
    matrix.m[10] = -zfar / (znear - zfar);
    matrix.m[8] = matrix.m[9] = 0.0;
    matrix.m[11] = 1.0;
    matrix.m[12] = matrix.m[13] = matrix.m[15] = 0.0;
    matrix.m[14] = (znear * zfar) / (znear - zfar);
    return matrix;
}

static Matrix matrix_PerspectiveFovLH(float fov,float aspect,float znear,float zfar) {
    Matrix matrix = matrix_Zero();
    float tanv = 1.0 / (tan(fov * 0.5));
    matrix.m[0] = tanv / aspect;
    matrix.m[1] = matrix.m[2] = matrix.m[3] = 0.0;
    matrix.m[5] = tanv;
    matrix.m[4] = matrix.m[6] = matrix.m[7] = 0.0;
    matrix.m[8] = matrix.m[9] = 0.0;
    matrix.m[10] = -zfar / (znear - zfar);
    matrix.m[11] = 1.0;
    matrix.m[12] = matrix.m[13] = matrix.m[15] = 0.0;
    matrix.m[14] = (znear * zfar) / (znear - zfar);
    return matrix;
}
static Matrix matrix_Transpose(const Matrix* matrix) {
    Matrix result;
    result.m[0] = matrix->m[0];
    result.m[1] = matrix->m[4];
    result.m[2] = matrix->m[8];
    result.m[3] = matrix->m[12];
    result.m[4] = matrix->m[1];
    result.m[5] = matrix->m[5];
    result.m[6] = matrix->m[9];
    result.m[7] = matrix->m[13];
    result.m[8] = matrix->m[2];
    result.m[9] = matrix->m[6];
    result.m[10] = matrix->m[10];
    result.m[11] = matrix->m[14];
    result.m[12] = matrix->m[3];
    result.m[13] = matrix->m[7];
    result.m[14] = matrix->m[11];
    result.m[15] = matrix->m[15];
    return result;
}

//==============================================================================
typedef struct Camera{
    Vector3 Position;
    Vector3 Target;
}Camera;

typedef struct Face{
    int A,B,C;
}Face;

typedef struct Vertex{
    Vector3 Normal;
    Vector3 Coordinates;
    Vector3 WorldCoordinates;
    Vector3 TextureCoordinates;
}Vertex;

typedef struct Texture{
   int width,height;
   int* internalBuffer;
}Texture;

typedef struct Mesh{
    char name[256];
    Vertex* Vertices;
    Face* faces;
    int faceCount;
    int verticesCount;
    Vector3 Rotation;
    Vector3 Position;
    Texture texture;
}Mesh;

typedef struct Device{
    int workingWidth,workingHeight;
    int* backbuffer;
    int* depthbuffer;
}Device;


static int texture_map(const Texture* tex, float tu,float tv){
   int itu=(int)(tu * tex->width);
   int itv=(int)(tv * tex->height);
   int u = 0;
   int v = 0;
   if(tex->width != 0){
      u = abs((itu % tex->width));
   }
   if(tex->height != 0){
      v = abs((itv % tex->height));
   }
   int pos = (u + v * tex->width);
   if(tex->internalBuffer == NULL){
      return 0;
   }
   return tex->internalBuffer[pos];
}

static Texture* texture_load(const char* filename){
    Texture* ret = (Texture*)malloc(sizeof(Texture));
    ret->internalBuffer=0;
    loadimage(filename,&ret->internalBuffer,&ret->width,&ret->height);
    return ret;
}
static void texture_unload(Texture* tex){
    if(tex->internalBuffer) free(tex->internalBuffer);
    free(tex);
}


static Camera softengine_camera(){
    Camera ret;
    return ret;
}
Mesh* softengine_mesh(const char* name,int verticesCount,int facesCount){
    Mesh* ret = (Mesh*)malloc(sizeof(Mesh));;
    memset(ret,0,sizeof(Mesh));
    strncpy(ret->name,name,256);
    ret->Vertices = (Vertex*)malloc(sizeof(Vertex)*verticesCount);
    ret->faces = (Face*)malloc(sizeof(Face)*facesCount);
    ret->verticesCount=verticesCount;
    ret->faceCount=facesCount;
    ret->Rotation = vector3_zero();
    ret->Position = vector3_zero();
    return ret;
}
Device* device(int width,int height){
    Device* ret = (Device*)malloc(sizeof(Device));
    ret->workingWidth = width;
    ret->workingHeight=height;
    ret->backbuffer = (int*)malloc(sizeof(int)*width*height);
    ret->depthbuffer = (int*)malloc(sizeof(int)*width*height);
    return ret;
}

void device_clear(Device* dev){
    int i,e=dev->workingHeight*dev->workingWidth;
    int* backbuffer=dev->backbuffer;
    int* depthbuffer=dev->depthbuffer;
    
#ifdef USE_SIMD_X86
    // SSE2 optimized clear for x86/x64
    __m128i zero = _mm_setzero_si128();
    __m128i depth_val = _mm_set1_epi32(10000000);
    
    int simd_end = (e / 4) * 4;  // Process 4 integers at a time
    
    for (i = 0; i < simd_end; i += 4) {
        _mm_storeu_si128((__m128i*)&backbuffer[i], zero);
        _mm_storeu_si128((__m128i*)&depthbuffer[i], depth_val);
    }
    
    // Handle remaining elements
    for (i = simd_end; i < e; i++) {
        backbuffer[i] = 0;
        depthbuffer[i] = 10000000;
    }
#elif defined(USE_SIMD_ARM)
    // NEON optimized clear for ARM
    int32x4_t zero = vdupq_n_s32(0);
    int32x4_t depth_val = vdupq_n_s32(10000000);
    
    int simd_end = (e / 4) * 4;  // Process 4 integers at a time
    
    for (i = 0; i < simd_end; i += 4) {
        vst1q_s32(&backbuffer[i], zero);
        vst1q_s32(&depthbuffer[i], depth_val);
    }
    
    // Handle remaining elements
    for (i = simd_end; i < e; i++) {
        backbuffer[i] = 0;
        depthbuffer[i] = 10000000;
    }
#else
    // Fallback to OpenMP parallelization
#pragma omp parallel for firstprivate(backbuffer,depthbuffer)
    for (i = 0; i < e; i++) {
        depthbuffer[i] = 10000000;
        backbuffer[i]=0;
    }
#endif
}
static void device_present(Device* dev){
    drawpixels(dev->backbuffer,0,0,dev->workingWidth,dev->workingHeight);
}
static void device_putPixel(Device* dev,int x,int y,int z,int color){
    // Bounds check to prevent out-of-bounds array access
    // Combined check for better branch prediction
    if(x < 0 || y < 0 || x >= dev->workingWidth || y >= dev->workingHeight) {
        return;
    }
    int idx=y*dev->workingWidth+x;
    if (dev->depthbuffer[idx] < z) {
        return;
    }
    dev->depthbuffer[idx]=z;
    dev->backbuffer[idx]=color;
}

static void device_drawPoint(Device* dev,const Vector3* point,int color){
    device_putPixel(dev,point->x,point->y,point->z,color);
}
static __inline float maxf(float a,float b){ return a>b?a:b;}
static __inline float minf(float a,float b){ return a<b?a:b;}
static __inline float device_clamp(float value,float minv,float maxv){
    return maxf(minv, minf(value, maxv));
}
static __inline float device_clamp0(float value){
    return device_clamp(value,0,1);
}
static __inline float device_interpolate(float minv, float maxv, float gradient) {
    return minv + (maxv - minv) * device_clamp0(gradient);
}

static Vertex device_project(const Device* dev,const Vertex* vertex,const Matrix* transMat,const Matrix* world){
    Vertex ret;
    Vector3 point2d = vector3_transform_coordinates(&vertex->Coordinates,transMat);
    float x = point2d.x * dev->workingWidth+dev->workingWidth/2;
    float y = -point2d.y * dev->workingHeight+dev->workingHeight/2;
    ret.Coordinates.x=x;
    ret.Coordinates.y=y;
    ret.Coordinates.z=point2d.z;
    ret.Normal = vector3_transform_coordinates(&vertex->Normal, world);
    //point3DWorld
    ret.WorldCoordinates=vector3_transform_coordinates(&vertex->Coordinates,world);
    ret.TextureCoordinates = vertex->TextureCoordinates;
    return ret;
}
static float device_computeNDotL(const Vector3* vertex,Vector3* normal,Vector3* lightPosition) {
    Vector3 lightDirection = vector3_subtract(lightPosition,vertex);
    vector3_normalize(normal);
    vector3_normalize(&lightDirection);
    return maxf(0, vector3_dot(normal, &lightDirection));
}
static int device_color4(int r,int g,int b,int a){
    return 0x010000*r+(0x000100)*g+0x000001*b+0x01000000*a;
}
static int device_color4ref(int refColr,float r,float g,float b,float a){
    return device_color4((int)(0xff0000&refColr)*r,(int)(0x00ff00&refColr)*g,(int)(0x0000ff&refColr)*b,(int)(0xff000000&refColr)*a);
}
typedef struct DrawData{
    float currentY;
    float ndotla,ndotlb,ndotlc,ndotld;
    float ua,ub,uc,ud,va,vb,vc,vd;
}DrawData;

static void device_processScanLine(Device* dev,const DrawData* data,const Vertex* va,const Vertex* vb,const Vertex* vc,const Vertex* vd,float color,const Texture* texture) {
    int x;
    float gradient1 = va->Coordinates.y != vb->Coordinates.y ? (data->currentY - va->Coordinates.y) / (vb->Coordinates.y - va->Coordinates.y) : 1;
    float gradient2 = vc->Coordinates.y != vd->Coordinates.y ? (data->currentY - vc->Coordinates.y) / (vd->Coordinates.y - vc->Coordinates.y) : 1;

    int sx = (int)device_interpolate(va->Coordinates.x, vb->Coordinates.x, gradient1);
    int ex = (int)device_interpolate(vc->Coordinates.x, vd->Coordinates.x, gradient2);

    float z1 = device_interpolate(va->Coordinates.z, vb->Coordinates.z, gradient1);
    float z2 = device_interpolate(vc->Coordinates.z, vd->Coordinates.z, gradient2);

    float snl = device_interpolate(data->ndotla, data->ndotlb, gradient1);
    float enl = device_interpolate(data->ndotlc, data->ndotld, gradient2);

    float su = device_interpolate(data->ua, data->ub, gradient1);
    float eu = device_interpolate(data->uc, data->ud, gradient2);
    float sv = device_interpolate(data->va, data->vb, gradient1);
    float ev = device_interpolate(data->vc, data->vd, gradient2);
    float currentY=data->currentY;
#pragma omp parallel for firstprivate(z1,z2,snl,enl,su,eu,sv,ev,texture)
    for (x = sx; x < ex; x++) {
        float gradient = (x - sx) / (ex - sx);

        float z = device_interpolate(z1, z2, gradient);
        float ndotl = device_interpolate(snl, enl, gradient) * color;
        float u = device_interpolate(su, eu, gradient);
        float v = device_interpolate(sv, ev, gradient);

        int textureColor;

        if (texture){
            textureColor = texture_map(texture,u, v); 
        }
        else {
            textureColor = 0xffffff;   
        }
        Vector3 pt=vector3(x,currentY,z);
        device_drawPoint(dev,&pt,device_color4ref(textureColor,ndotl ,ndotl,ndotl, 1));
    }
}

static void device_drawTriangle(Device* dev,Vertex* v1,Vertex* v2,Vertex* v3,float color,const Texture* texture) {
            if (v1->Coordinates.y > v2->Coordinates.y) {
                Vertex* temp = v2;
                v2 = v1;
                v1 = temp;
            }

            if (v2->Coordinates.y > v3->Coordinates.y) {
                Vertex* temp = v2;
                v2 = v3;
                v3 = temp;
            }

            if (v1->Coordinates.y > v2->Coordinates.y) {
                Vertex* temp = v2;
                v2 = v1;
                v1 = temp;
            }

            Vector3 lightPos = vector3(0, 10, 10);

            float nl1 = device_computeNDotL(&v1->WorldCoordinates, &v1->Normal, &lightPos);
            float nl2 = device_computeNDotL(&v2->WorldCoordinates, &v2->Normal, &lightPos);
            float nl3 = device_computeNDotL(&v3->WorldCoordinates, &v3->Normal, &lightPos);

            DrawData data;

            float dP1P2 = 0;
            float dP1P3 = 0;

            if (v2->Coordinates.y - v1->Coordinates.y > 0){
                dP1P2 = (v2->Coordinates.x - v1->Coordinates.x) / (v2->Coordinates.y - v1->Coordinates.y);
            }
            if ( v3->Coordinates.y - v1->Coordinates.y > 0)
                dP1P3 = ( v3->Coordinates.x - v1->Coordinates.x) / ( v3->Coordinates.y - v1->Coordinates.y);
            if (dP1P2 > dP1P3) {
                int y;
                for (y = v1->Coordinates.y;  y <= v3->Coordinates.y; y++) {
                    data.currentY = y;

                    if (y < v2->Coordinates.y) {
                        data.ndotla = nl1;
                        data.ndotlb = nl3;
                        data.ndotlc = nl1;
                        data.ndotld = nl2;

                        data.ua = v1->TextureCoordinates.x;
                        data.ub = v3->TextureCoordinates.x;
                        data.uc = v1->TextureCoordinates.x;
                        data.ud = v2->TextureCoordinates.x;

                        data.va = v1->TextureCoordinates.y;
                        data.vb = v3->TextureCoordinates.y;
                        data.vc = v1->TextureCoordinates.y;
                        data.vd = v2->TextureCoordinates.y;

                        device_processScanLine(dev,&data, v1, v3, v1, v2, color, texture);
                    } else {
                        data.ndotla = nl1;
                        data.ndotlb = nl3;
                        data.ndotlc = nl2;
                        data.ndotld = nl3;

                        data.ua = v1->TextureCoordinates.x;
                        data.ub = v3->TextureCoordinates.x;
                        data.uc = v2->TextureCoordinates.x;
                        data.ud = v3->TextureCoordinates.x;

                        data.va = v1->TextureCoordinates.y;
                        data.vb = v3->TextureCoordinates.y;
                        data.vc = v2->TextureCoordinates.y;
                        data.vd = v3->TextureCoordinates.y;

                        device_processScanLine(dev,&data, v1, v3, v2, v3, color, texture);
                    }
                }
            } else {
                int y;
                for (y = v1->Coordinates.y; y <= v3->Coordinates.y ; y++) {
                    data.currentY = y;

                    if (y < v2->Coordinates.y) {
                        data.ndotla = nl1;
                        data.ndotlb = nl2;
                        data.ndotlc = nl1;
                        data.ndotld = nl3;

                        data.ua = v1->TextureCoordinates.x;
                        data.ub = v2->TextureCoordinates.x;
                        data.uc = v1->TextureCoordinates.x;
                        data.ud = v3->TextureCoordinates.x;

                        data.va = v1->TextureCoordinates.y;
                        data.vb = v2->TextureCoordinates.y;
                        data.vc = v1->TextureCoordinates.y;
                        data.vd = v3->TextureCoordinates.y;

                        device_processScanLine(dev,&data, v1, v2, v1, v3, color, texture);
                    } else {
                        data.ndotla = nl2;
                        data.ndotlb = nl3;
                        data.ndotlc = nl1;
                        data.ndotld = nl3;

                        data.ua = v2->TextureCoordinates.x;
                        data.ub = v3->TextureCoordinates.x;
                        data.uc = v1->TextureCoordinates.x;
                        data.ud = v3->TextureCoordinates.x;

                        data.va = v2->TextureCoordinates.y;
                        data.vb = v3->TextureCoordinates.y;
                        data.vc = v1->TextureCoordinates.y;
                        data.vd = v3->TextureCoordinates.y;

                        device_processScanLine(dev,&data, v2, v3, v1, v3, color, texture);
                    }
                }
            }
}

#if 1

void device_render(Device* dev, const Camera* camera, const Mesh* meshes,int meshesLength){
    int index;
    Vector3 up = vector3_up();
    Matrix viewMatrix=matrix_LookAtLH(&camera->Position,&camera->Target,&up);
    Matrix projectionMatrix = matrix_PerspectiveFovLH(0.78,dev->workingWidth / dev->workingHeight, 0.01, 1.0);            
    for(index = 0; index < meshesLength; index++) {
        int indexVertices;
        const Mesh* cMesh = &meshes[index];
        Matrix rotationYPR= matrix_RotationYawPitchRoll(cMesh->Rotation.y, cMesh->Rotation.x, cMesh->Rotation.z);
        Matrix translation= matrix_Translation(cMesh->Position.x,cMesh->Position.y, cMesh->Position.z);
        Matrix worldMatrix = matrix_multiply(&rotationYPR,&translation);
        Matrix res1=matrix_multiply(&worldMatrix,&viewMatrix);
        Matrix transformMatrix = matrix_multiply(&res1,&projectionMatrix);
        for(indexVertices = 0; indexVertices < cMesh->faceCount; indexVertices++) {
            Face* currentFace = &cMesh->faces[indexVertices];
            Vertex* vertexA=&cMesh->Vertices[currentFace->A];
            Vertex* vertexB=&cMesh->Vertices[currentFace->B];
            Vertex* vertexC=&cMesh->Vertices[currentFace->C];
            Vertex pixelA = device_project(dev,vertexA,&transformMatrix, &worldMatrix);
            Vertex pixelB = device_project(dev,vertexB,&transformMatrix, &worldMatrix);
            Vertex pixelC = device_project(dev,vertexC,&transformMatrix, &worldMatrix);
            float color=1.0f;
            device_drawTriangle(dev,&pixelA, &pixelB, &pixelC, color, &cMesh->texture);
        }
    }
    device_present(dev);
}

// Memory cleanup functions
void mesh_free(Mesh* mesh) {
    if(!mesh) return;
    if(mesh->Vertices) {
        free(mesh->Vertices);
        mesh->Vertices = NULL;
    }
    if(mesh->faces) {
        free(mesh->faces);
        mesh->faces = NULL;
    }
    if(mesh->texture.internalBuffer) {
        free(mesh->texture.internalBuffer);
        mesh->texture.internalBuffer = NULL;
    }
    free(mesh);
}

void device_free(Device* dev) {
    if(!dev) return;
    if(dev->backbuffer) {
        free(dev->backbuffer);
        dev->backbuffer = NULL;
    }
    if(dev->depthbuffer) {
        free(dev->depthbuffer);
        dev->depthbuffer = NULL;
    }
    free(dev);
}

#endif


