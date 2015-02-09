#include "sdlmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int N=300;
float* X;
float* Y;
float* vX;
float* vY;
int* collision;
int* Color;
float secPerSimulate=0.1;
float decreasePerFrame=0.001;
int radius=5;
int SCREEN_X=800;
int SCREEN_Y=600;
int minV=-10;
int maxV=10;
int runFlag=1;
//#define MALLOC(X) malloc(X)
//#define ZALLOC(X) calloc(1,(X))
#define MALLOC(X) __debugMalloc((X),"",__LINE__,"",0)
#define ZALLOC(X) __debugMalloc((X),"",__LINE__,"",1)
static float randomf(float l,float u);
static void init(int size);
static void simulate();
static void draw();
static void clean();

static float randomf(float l,float u){
   float r,t;
   if(l>u){
      t=l;
      l=u;
      u=t;
   }
   r=u-l;
   return l+(r*((double)rand()/RAND_MAX));
}
static void* __debugMalloc(int size, const char* fn, int line, const char* file,int clean){
   void* ret;
   ret= malloc(size);
   if(!ret){
      fprintf(stderr,"at file '%s' line '%d' function '%s', malloc(%d) failed\n",
         file,line,fn,size);
      exit(-1);
   }
   if(clean)memset(ret,0,size);
   return ret;
}
static void clean(){
   if(X) free(X);
   if(Y) free(Y);
   if(vX) free(vX);
   if(vY) free(vY);
   if(Color) free(Color);
   X = Y = vX = vY = NULL;
   Color = NULL;
   
}
static void init(int size){
   int i,hasCollision,idx;
   int retry=0;
   N=size;
   X = (float*)MALLOC(sizeof(float)*size);
   Y = (float*)MALLOC(sizeof(float)*size);
   vX = (float*)MALLOC(sizeof(float)*size);
   vY = (float*)MALLOC(sizeof(float)*size);
   Color=(int*)MALLOC(sizeof(int)*size);
   collision=(int*)ZALLOC(sizeof(int)*size);

   for(i=0; i<N; ++i){
      X[i]=randomf(0,SCREEN_X-1);
      Y[i]=randomf(0,SCREEN_Y-1);
      idx=Y[i]*SCREEN_X+X[i];
      vX[i] = randomf(minV,maxV);
      vY[i] = randomf(minV,maxV);
      Color[i]=rand();
   }

}

static int getSign(float f){
    if(f < 0) return -1;
    return 1;
}

static void simulate(){
   int i;
   float collisionDistanceSquare=(radius+radius);
   collisionDistanceSquare*=collisionDistanceSquare;
#pragma omp parallel for firstprivate(SCREEN_X,SCREEN_Y,collisionDistanceSquare,radius ) 
   for(i=0; i<N; ++i){
      collision[i]=0;
      X[i]+=vX[i]*secPerSimulate;
      Y[i]+=vY[i]*secPerSimulate;
      int j;
      for(j=0; j<N; ++j){
         if(j==i) continue;
         float vXValue=X[j]-X[i];
         float vYValue=Y[j]-Y[i];
         vXValue*=vXValue;
         vYValue*=vYValue;
         if(vXValue+vYValue <= collisionDistanceSquare){
         // hit
              collision[i]=1;
              collision[j]=1;
              if( getSign(vX[j]) != getSign(vX[i]) ) {
      //           vX[i] = -vX[i];
      //           vX[j] = -vX[j];
                 vXValue=(vX[i]+vX[j])/2;
                 vX[j]-=getSign(vX[j])*vXValue;
                 vX[i]-=getSign(vX[i])*vXValue;
              }
              if( getSign(vY[j]) != getSign(vY[i]) ) {
//                 vY[i] = -vY[i];
//                 vY[j] = -vY[j];
                  vYValue=(vY[i]+vY[j])/2;
                  vY[j]-=getSign(vY[j])*vYValue;
                  vY[i]-=getSign(vY[i])*vYValue;
              }

         }
      }
      if((X[i] <= radius && vX[i]< 0)||(X[i]>=SCREEN_X-radius && vX[i]>0)){
          vX[i]=-vX[i];
      }
      if((Y[i] <= radius  && vY[i]< 0)||(Y[i]>=SCREEN_Y-radius && vY[i]>0)){
          vY[i]=-vY[i];
      }
      if(getSign(vX[i])*vX[i] > maxV ) vX[i]=maxV*getSign(vX[i]);
      if(getSign(vY[i])*vY[i] > maxV ) vY[i]=maxV*getSign(vY[i]);
      vX[i]-=getSign(vX[i])*decreasePerFrame;
      vY[i]-=getSign(vY[i])*decreasePerFrame;
   }
}

static void draw(){
   int i;
   fillrect(0,0,SCREEN_X,SCREEN_Y,0xffffff);
   for(i=0; i<N; ++i){
      fillcircle(X[i],Y[i],radius,Color[i]);
      if(collision[i]){
         drawcircle(X[i],Y[i],radius+1,~Color[i]);
      }
      collision[i]=0;
   }
   flushscreen();

}

int main(int argc, char** argv){
   int s=300;
   if(argc > 1) s = atoi(argv[1]);
   if(argc > 2) radius = atoi(argv[2]);
   if(argc > 3) minV = atoi(argv[3]);
   if(argc > 4) maxV = atoi(argv[4]);
   if(argc > 5) decreasePerFrame = atof(argv[5]);
   printf("simulate particles :%d\n",s);
   init(s);
   screen(SCREEN_X,SCREEN_Y);
   while(runFlag){
     simulate();
     draw();
     delay(1);
   }
}

