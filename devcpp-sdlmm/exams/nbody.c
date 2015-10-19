#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
//#include<mpi.h>
#include<omp.h>
#define SCREENX 800
#define SCREENY 600
#define VSYNC 1
#define HAS_SDLMM 1
#define NUM_BODY 2000

#ifdef HAS_SDLMM
#include <sdlmm.h>
#include "libsdlmmWrap.h"
//SDLMM* sdlmm;
#endif
int showhelp=1;
int showmode=3;
float simulatetime_factor=0.01;
int random_simulatefactor=1;
int centralize=0;
int SZ=NUM_BODY;
#define LOOP 500
#define MAX_X_axis 1000
#define MIN_X_axis 000
#define MAX_Y_axis 1000
#define MIN_Y_axis 000
#define MAX_Velocity 200
#define MIN_velocity -200
#define MAX_Mass 150
#define MIN_Mass 3

////////////////////////////////////////////////////
float *X_axis,*Y_axis,*Z_axis,*X_Velocity,*Y_Velocity,*Z_Velocity,*Mass;
float *newX_velocity,*newY_velocity,*newZ_velocity;
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
        Z_axis[i]=rand()%(MAX_Y_axis-MIN_Y_axis)+MIN_Y_axis;
        X_Velocity[i]=newX_velocity[i]=0;
        Y_Velocity[i]=newY_velocity[i]=0;
        Z_Velocity[i]=newZ_velocity[i]=0;
        Mass[i]=rand()%(MAX_Mass-MIN_Mass)+MIN_Mass;
    }
}
static float clamp(float v,float minv,float maxv){
    if(v > maxv) v = (v+maxv)/2;
    if(v < minv) v = (v+minv)/2;
    return v;
}
static void Nbody(int i,int sz,float* X_axis,float* Y_axis,float* Z_axis,float* newX_velocity,float* newY_velocity,float* newZ_velocity,const float* Mass,float simulatetime) {
    int j;
    float sumX=0,sumY=0,sumZ=0;
#define  Gravity_Coef 3.3
    for(j=0; j<sz; j++) {
        float X_position,Y_position,Z_position;
        float Distance;
        float Force=0;
        if(j==i) continue;
        X_position=X_axis[j]-X_axis[i];
        Y_position=Y_axis[j]-Y_axis[i];
        Z_position=Z_axis[j]-Z_axis[i];
        Distance=sqrt(X_position*X_position+Y_position*Y_position+Z_position*Z_position);
        if(Distance==0) continue;
        Force= Gravity_Coef*Mass[i]/(Distance*Distance);
        sumX += Force*X_position;
        sumY += Force*Y_position;
        sumZ += Force*Z_position;
    }
    newX_velocity[i]+=sumX*simulatetime;
    newY_velocity[i]+=sumY*simulatetime; 
    newZ_velocity[i]+=sumZ*simulatetime; 
    X_axis[i]+=clamp(newX_velocity[i],MIN_velocity,MAX_Velocity)*simulatetime;
    Y_axis[i]+=clamp(newY_velocity[i],MIN_velocity,MAX_Velocity)*simulatetime;
    Z_axis[i]+=clamp(newZ_velocity[i],MIN_velocity,MAX_Velocity)*simulatetime;
    X_Velocity[i]=newX_velocity[i];
    Y_Velocity[i]=newY_velocity[i];
    Z_Velocity[i]=newZ_velocity[i];
   
}

static float* allocateBody(){
   float* ret;
   ret =(float*)malloc(sizeof(float)*SZ);
   memset(ret,0,sizeof(float)*SZ);
   return ret;
}
static void freeBody(void* p){   free(p);  }
/// draw body and hints
static void drawBodys(int loop,int totalLoop,double tm,float avgX,float avgY,float avgZ){
#ifdef HAS_SDLMM
   int i;
   double rendert1,rendert2;
   char buf[1024];
   rendert1 = getDoubleTime();
   fillrect(0,0,SCREENX,SCREENY,0x2f2f2f);
   for(i=0; i<SZ;++i){
      int x,y,r,c;
      if(centralize){
         x = SCREENX/2*(X_axis[i]/avgX);
         y = SCREENY/2*(Y_axis[i]/avgY);
      }
     else  {
          x = avgX-X_axis[i]+SCREENX/2;
          y = avgY-Y_axis[i]+SCREENY/2;
      }
      r = 5*(Z_axis[i]/avgZ); 
      if(r < 0 || r > 255 ) continue;
      if(showmode==0){
         drawcircle(x,y,r,0x00D0D0|(r*0xa0a000));
      }
      else if(showmode==1){
         drawpixel(x,y,0x00D0D0|(r*0xa0a000));
      }
      else if(showmode==2){
         drawpixel(x,y,0xffffff);
         drawcircle(x,y,r,0x00D0D0|(r*0xa0a000));
      }
      else if(showmode==3){
         fillcircle(x,y,r,0x00D0D0|(r*0xa0a000));
      }
   }
   if(showhelp){
      sprintf(buf,"[%-3d/%-3d] tm:%-3.3f",loop,totalLoop,tm);
      //fillrect(0,0,200,20,0xffffff);
      drawtext(buf,0,0,0xffffff);
      sprintf(buf,"show mode: %-2d[0-3]",showmode);
      drawtext(buf,0,20,0xffffff);
      sprintf(buf,"simulate factor: %-3.5f",simulatetime_factor);
      drawtext(buf,0,40,0xffffff);
      sprintf(buf,"randomize simulate factor: %s[r]",random_simulatefactor?"on":"off");
      drawtext(buf,0,60,0xffffff);
      fillrect(320,60,400*(simulatetime_factor/2),20,0xfdfd00);
      drawrect(320,60,400,20,0xffffff);
      sprintf(buf,"centralize: %s[c]",centralize?"on":"off");
      drawtext(buf,0,80,0xffffff);
   }
   rendert2=getDoubleTime();
   flushscreen();
   tm += (rendert2-rendert1);
   #if VSYNC == 1
   if(tm <  1.0 / 60){
       delay((int)((1.0/60)*1000-tm ));
   }
   #endif
#endif
}

static int main_run(int argc,char **argv) {
    int loop;
    double tmstart,tmend;
    double totalTime = 0.0;
    double fps_time_1,fps_time_2;
    float avgX=0,avgY=0,avgZ=0;
    tmstart = getDoubleTime();
    Init_AllBody();
    for(loop=0; loop<LOOP; loop++) 
    {
        int i;
        //printf("loop %d (%f,%f)\n",loop,avgX,avgY);
        avgX=0;avgY=0;avgZ=0; 
        fps_time_1=getDoubleTime();
#pragma omp parallel for firstprivate(X_Velocity,Y_Velocity,Z_Velocity,newX_velocity,newY_velocity,newZ_velocity,X_axis,Y_axis,Z_axis,Mass,SZ,simulatetime_factor) // reduction(+:avgX,avgY,avgZ)
        for(i=0; i<SZ; i++) 
        {
             Nbody(i,SZ,X_axis,Y_axis,Z_axis,newX_velocity,newY_velocity,newZ_velocity,Mass,simulatetime_factor);
             //avgX+=X_axis[i];
             //avgY+=Y_axis[i];
             //avgZ+=Z_axis[i];
        }
        for(i=0; i<SZ; i++) 
        {
             avgX+=X_axis[i];
             avgY+=Y_axis[i];
             avgZ+=Z_axis[i];
         
        }
        avgX/=SZ;
        avgY/=SZ;
        avgZ/=SZ;
        fps_time_2 = getDoubleTime();
        drawBodys(loop,LOOP,fps_time_2-fps_time_1,avgX,avgY,avgZ);
    }
    tmend = getDoubleTime();
    printf("%d %lf\n",NUM_BODY,tmend-tmstart);
    if(random_simulatefactor){
       simulatetime_factor = (((float)rand())/RAND_MAX);
    }
    return 0;
}
// handle keyboard event
static void kbfnc(int k,int ctrl,int on){
    if(on){
        switch(k){
            case '0': case '1': case '2': case '3':  showmode=k-'0'; break;
            case 'c':case 'C': centralize=!centralize; break;
            case 'r':case 'R': random_simulatefactor=!random_simulatefactor; break;
            case 'h':case 'H': showhelp = !showhelp; break;
        }
    }
}
// handle mouse event
static void mousefnc(int x,int y,int on,int btn){
    if(on){
        if(y > 60 && y < 80 && x >= 320 && x <= 320+400){
            float value=2.0*((float)(x - 320))/400;
            simulatetime_factor=value;
        }
    }
}
static void mousemotion(int x,int y,int on){
    mousefnc(x,y,on,0);
}
int main(int argc,char** argv){
    int i;
    if(argc > 1){
       SZ = atoi(argv[1]);
    }
    X_axis = allocateBody();  
    Y_axis = allocateBody();  
    Z_axis = allocateBody();  
    X_Velocity = allocateBody();  
    Y_Velocity = allocateBody();
    Z_Velocity = allocateBody();
    Mass = allocateBody();  
    newX_velocity = allocateBody();
    newY_velocity = allocateBody();
    newZ_velocity = allocateBody();
#ifdef HAS_SDLMM
    screen(SCREENX,SCREENY);
    screentitle("[GPU] NBody-Simulation");
    settextfont("FreeMono.ttf",16);
    setonkey(kbfnc);
    setonmouse(mousefnc);
    setonmotion(mousemotion);
#endif
    for(i=0; i<20; ++i){
       main_run(argc,argv);
    }
    freeBody(X_axis);
    freeBody(Y_axis);
    freeBody(X_Velocity);
    freeBody(Y_Velocity );
    freeBody(Mass);
    freeBody(newX_velocity );
    freeBody(newY_velocity );
    return 0;
}


