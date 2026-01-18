#ifndef BABYLON3D_H
#define BABYLON3D_H

#include <math.h>

// Forward declarations
typedef struct Vector2 Vector2;
typedef struct Vector3 Vector3;
typedef struct Matrix Matrix;
typedef struct Camera Camera;
typedef struct Face Face;
typedef struct Vertex Vertex;
typedef struct Texture Texture;
typedef struct Mesh Mesh;
typedef struct Device Device;

// Struct definitions
struct Vector2 {
    float x, y;
};

struct Vector3 {
    float x, y, z;
};

struct Matrix {
    float m[16];
};

struct Camera {
    Vector3 Position;
    Vector3 Target;
};

struct Face {
    int A, B, C;
};

struct Vertex {
    Vector3 Normal;
    Vector3 Coordinates;
    Vector3 WorldCoordinates;
    Vector3 TextureCoordinates;
};

struct Texture {
    int width, height;
    int* internalBuffer;
};

struct Mesh {
    char name[256];
    Vertex* Vertices;
    Face* faces;
    int faceCount;
    int verticesCount;
    Vector3 Rotation;
    Vector3 Position;
    Texture texture;
};

struct Device {
    int workingWidth, workingHeight;
    int* backbuffer;
    int* depthbuffer;
};

// Public API functions

// Mesh functions
Mesh* softengine_mesh(const char* name, int verticesCount, int facesCount);
void mesh_free(Mesh* mesh);

// Device functions
Device* device(int width, int height);
void device_free(Device* dev);
void device_clear(Device* dev);
void device_render(Device* dev, const Camera* camera, const Mesh* meshes, int meshesLength);

// Texture functions
Texture* texture_load(const char* filename);
void texture_unload(Texture* tex);

// Vector3 helper functions
Vector3 vector3(float x, float y, float z);
Vector3 vector3_zero(void);
Vector3 vector3_normalize_copy(const Vector3* vector);

#endif // BABYLON3D_H
