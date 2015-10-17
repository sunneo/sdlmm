#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
//#include<mpi.h>
#include<omp.h>
#define SCREENX 800
#define SCREENY 600
#include <sdlmm.h>

#define NUM_BODY 1024
int SZ=NUM_BODY;
#define LOOP 500
#define MAX_X_axis 800
#define MIN_X_axis 100
#define MAX_Y_axis 600
#define MIN_Y_axis 100
#define MAX_Velocity 5
#define MIN_velocity 1
#define MAX_Mass 15
#define MIN_Mass 10
////////////////////////////////////////////////////
float *X_axis,*Y_axis,*X_Velocity,*Y_Velocity,*Mass;
float *newX_velocity,*newY_velocity;
int* bodyColor,* bodyRadius;
#ifdef __linux__
#include<sys/time.h>
#else
#include <time.h>
#endif
static double getDoubleTime() {
#ifdef __linux__
    struct timeval tm_tv;
    gettimeofday(&tm_tv,0);
    return (double)tm_tv.tv_sec + (1e-6)*tm_tv.tv_usec;
#else
    return ((double)clock())/CLOCKS_PER_SEC;
#endif
}

static void Init_AllBody() {
    int i=0;
    for(i=0; i<SZ; i++) {
        X_axis[i]=rand()%(MAX_X_axis-MIN_X_axis)+MIN_X_axis;
        Y_axis[i]=rand()%(MAX_Y_axis-MIN_Y_axis)+MIN_Y_axis;
        X_Velocity[i]=newX_velocity[i]=0;
        Y_Velocity[i]=newY_velocity[i]=0;
        Mass[i]=rand()%(MAX_Mass-MIN_Mass)+MIN_Mass;
        bodyColor[i]=0xffff00|(int)(Mass[i]*60);
        bodyRadius[i]=Mass[i]/10+1;
    }
}

static void Nbody(int i,int sz,float* X_axis,float* Y_axis,float* newX_velocity,float* newY_velocity,const float* Mass) {
    int j;
    float sumX=0,sumY=0;
#define  Gravity_Coef 3.3 
    for(j=0; j<sz; j++) {
        float X_position,Y_position;
        float Distance;
        float Force=0;
        if(j==i) continue;
        X_position=X_axis[j]-X_axis[i];
        Y_position=Y_axis[j]-Y_axis[i];
        Distance=sqrt(X_position*X_position+Y_position*Y_position);
        if(Distance==0) continue;
        Force= Gravity_Coef*Mass[i]/(Distance);
        sumX += Force*X_position/Distance;
        sumY += Force*Y_position/Distance;
    }
    newX_velocity[i]+=sumX;
    newY_velocity[i]+=sumY; 
    X_axis[i]+=newX_velocity[i]*0.01;
    Y_axis[i]+=newY_velocity[i]*0.01;
    X_Velocity[i]=newX_velocity[i];
    Y_Velocity[i]=newY_velocity[i];
    if(X_Velocity[i] > MAX_Velocity) X_Velocity[i] = (MIN_velocity+newX_velocity[i])/2;
    if(Y_Velocity[i] > MAX_Velocity) Y_Velocity[i] = (MIN_velocity+newY_velocity[i])/2;
    if(X_Velocity[i] < MIN_velocity) X_Velocity[i] = (MAX_Velocity+newX_velocity[i])/2;
    if(Y_Velocity[i] < MIN_velocity) Y_Velocity[i] = (MAX_Velocity+newY_velocity[i])/2;
    newX_velocity[i]=newX_velocity[i];
    newY_velocity[i]=newY_velocity[i];

}

static float* allocateBody(){
   float* ret;
   ret =(float*)malloc(sizeof(float)*SZ);
   memset(ret,0,sizeof(float)*SZ);
   return ret;
}
static void freeBody(void* p){   
   free(p);
}

static void drawBodys(int loop,int totalLoop,double fps){
#ifdef HAS_SDLMM
   int i;
   char buf[1024];
   sprintf(buf,"[%-3d/%-3d] fps:%-3.3f",loop,totalLoop,fps);
   fillrect(0,0,SCREENX,SCREENY,0);
   for(i=0; i<SZ;++i){
      int x,y,r,c;
      if(X_axis[i] < 0 || X_axis[i] > SCREENX || Y_axis[i] < 0 || Y_axis[i] > SCREENY) continue;
      x = X_axis[i]-bodyRadius[i];
      y = Y_axis[i]-bodyRadius[i];
      r = (((int)bodyRadius[i])&0xf);
      c = bodyColor[i];
      drawcircle(x,y,r+3,c|0x80808080);
      fillcircle(x,y,r,c|0xD0D0D0D0);
   }
   fillrect(0,0,200,20,0xffffff);
   drawtext(buf,0,0,0);
   flushscreen();
#endif
}

static int main_run(int argc,char **argv) {
    int loop;
    double tmstart,tmend;
    double totalTime = 0.0;
    double fps_time_1,fps_time_2;
    tmstart = getDoubleTime();
    Init_AllBody();
    for(loop=0; loop<LOOP; loop++) 
    {
        int i;
        printf("loop %d\n",loop);
        fps_time_1=getDoubleTime();
#pragma omp parallel for firstprivate(X_Velocity,Y_Velocity,newX_velocity,newY_velocity,X_axis,Y_axis,Mass,SZ)
        for(i=0; i<SZ; i++) 
        {
             Nbody(i,SZ,X_axis,Y_axis,newX_velocity,newY_velocity,Mass);
        }
        fps_time_2 = getDoubleTime();
        drawBodys(loop,LOOP,1/(fps_time_2-fps_time_1));
    }
    tmend = getDoubleTime();
    printf("%d %lf\n",NUM_BODY,tmend-tmstart);
    return 0;
}

int main(int argc,char** argv){
    int i;
    if(argc > 1){
       SZ = atoi(argv[1]);
    }
    X_axis = allocateBody();  
    Y_axis = allocateBody();  
    X_Velocity = allocateBody();  
    Y_Velocity = allocateBody();
    Mass = allocateBody();  
    newX_velocity = allocateBody();
    newY_velocity = allocateBody();
    bodyColor = (int*)malloc(sizeof(int)*NUM_BODY);
    bodyRadius = (int*)malloc(sizeof(int)*NUM_BODY);
#ifdef HAS_SDLMM
    //sdlmm = sdlmm_get_instance("libsdlmm.so");
    screen(SCREENX,SCREENY);
    screentitle("[GPU] NBody-Simulation");
    settextfont("/usr/share/fonts/truetype/freefont/FreeSerif.ttf",16);
#endif
    for(i=0; i<20; ++i){
       main_run(argc,argv);
    }
    freeBody(X_axis);
    freeBody(Y_axis);
    freeBody(X_Velocity);
    freeBody(Y_Velocity );
    freeBody(Mass );
    freeBody(newX_velocity );
    freeBody(newY_velocity );
    free(bodyColor);
    free(bodyRadius);
    return 0;
}

