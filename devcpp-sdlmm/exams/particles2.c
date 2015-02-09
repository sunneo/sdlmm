#include "sdlmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int N=300;
int maxN=1000;
float* X;
float* Y;
float* vX;
float* vY;
int* collision;
int* Color;
int currentN=0;
float secPerSimulate=0.1;
float decreasePerFrame=0.001;
int radius=5;
int SCREEN_X=1024;
int SCREEN_Y=768;
int minV=-10;
int maxV=10;
int runFlag=1;
//#define MALLOC(X) malloc(X)
//#define ZALLOC(X) calloc(1,(X))
#define MALLOC(X) __debugMalloc((X),"",__LINE__,"",0)
#define ZALLOC(X) __debugMalloc((X),"",__LINE__,"",1)
static float randomf(float l,float u);
static void init(int size,int maxSize);
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
static void generate(int n);
static void init(int size,int allocSize){
   int i,hasCollision,idx;
   int retry=0;
   N=size;
   X = (float*)ZALLOC(sizeof(float)*allocSize);
   Y = (float*)ZALLOC(sizeof(float)*allocSize);
   vX = (float*)ZALLOC(sizeof(float)*allocSize);
   vY = (float*)ZALLOC(sizeof(float)*allocSize);
   Color=(int*)MALLOC(sizeof(int)*allocSize);
   collision=(int*)ZALLOC(sizeof(int)*allocSize);
   generate(size);

}

static int getSign(float f){
    if(f < 0) return -1;
    return 1;
}
static void swap(int i,int j);
static void simulate(){
   int i;
   float collisionDistanceSquare=(radius+radius);
   collisionDistanceSquare*=collisionDistanceSquare;
#pragma omp parallel for firstprivate(SCREEN_X,SCREEN_Y,collisionDistanceSquare,radius ) 
   for(i=currentN-1; i>=0; --i){
      float distanceToY;
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
      if((X[i] <= radius && vX[i]< 0)){
          vX[i]=-vX[i];
          
      }
      if(X[i] > SCREEN_X){
         swap(i,currentN-1);
         if(currentN-1 > 0){
             --currentN;  
         }
         continue;
      }
      if((Y[i] <= radius  && vY[i]< 0)||(Y[i]>=SCREEN_Y-radius && vY[i]>0)){
          vY[i]=-vY[i];
          
          if(Y[i]>=SCREEN_Y-radius && fabs(vY[i])<0.001){
               swap(i,currentN-1);
               if(currentN-1 > 0){
                 --currentN;  
               }
          }
      }
      if(getSign(vX[i])*vX[i] > maxV ) vX[i]=maxV*getSign(vX[i]);
      if(getSign(vY[i])*vY[i] > maxV ) vY[i]=maxV*getSign(vY[i]);
      // vX[i]-=getSign(vX[i])*decreasePerFrame;
      // vY[i]-=getSign(vY[i])*decreasePerFrame;
      distanceToY=(SCREEN_Y-Y[i])/2;
      vY[i] += (9.8*secPerSimulate)/(distanceToY*distanceToY);

   }
}
static void swapf(float* a,float* b){
    float t=*a;
    *a=*b;
    *b=t;
}
static void swapi(int* a,int* b){
    int t=*a;
    *a=*b;
    *b=t;
}
static void swap(int i,int j){
    swapf(&X[i],&X[j]);
    swapf(&vX[i],&vX[j]);
    swapf(&Y[i],&Y[j]);
    swapf(&vY[i],&vY[j]);
    swapi(&Color[i],&Color[j]);
}
static void draw(){
   int i;
   fillrect(0,0,SCREEN_X,SCREEN_Y,0xffffff);
   
   for(i=0; i<currentN; ++i){
      fillcircle((int)X[i],(int)Y[i],radius,Color[i]);
      if(collision[i]){
         drawcircle((int)X[i],(int)Y[i],radius+1,~Color[i]);
      }
      collision[i]=0;
      
   }
   flushscreen();
}

int retryCnt = 0;
int waitForEmpty = 0;
int waitCycle=0;
int runCycle=0;
static void generate(int cnt){
   int i;
   int end=0;
   int start=0;
   if(runCycle > 200){
       waitCycle=1;
       runCycle=0;
       return;
   }
   if(waitCycle){
       if(retryCnt < 1000){
          // printf("retryCnt=%d(waitCycle),currentN=%d,maxN=%d\n",retryCnt,currentN,maxN);
          ++retryCnt;
           return;
       }
       else{
           retryCnt = 0;
           waitCycle=0;
       }
   }
   if(!waitForEmpty){
      if(currentN >= maxN){
          waitForEmpty=1;
          return;
      }
   }
   if(waitForEmpty != 0){
      if(retryCnt < 1000){
          ++retryCnt;
          //printf("retryCnt=%d(waitForEmpty),currentN=%d,maxN=%d\n",retryCnt,currentN,maxN);
          return;
      }
      else{
          waitForEmpty = 0;
          retryCnt = 0;
      }
   }
   end =currentN+cnt;
   if(end >= maxN) end = N;
   start=currentN-1;
   if(start < 0) start=0;
   for(i=start; i<end; ++i){
      X[i]=randomf(0,10);
      Y[i]=randomf(0,10);
      vX[i] = randomf(minV,maxV);
      vY[i] = randomf(minV,maxV);
      Color[i]=rand();
      if(Color[i] <65536){
          Color[i] |= (Color[i]<<15);
      }
   }
   if(currentN+cnt < maxN){
      currentN+=cnt;
   }
   else{
       currentN=maxN;
   }
   ++runCycle;

}

int main(int argc, char** argv){
   int s=300;
   if(argc > 1) s = atoi(argv[1]);
   if(argc > 2) maxN = atoi(argv[2]);
   if(argc > 3) radius = atoi(argv[3]);
   if(argc > 4) maxV = atoi(argv[4]);
   if(argc > 5) decreasePerFrame = atof(argv[5]);
   printf("simulate particles :%d\n",s);
   init(s,maxN);
   screen(SCREEN_X,SCREEN_Y);
   while(runFlag){
     generate(1);
     simulate();
     draw();
     delay(1);
   }
}

