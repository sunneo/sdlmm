#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
//#include<mpi.h>
#include<omp.h>
#define SCREENX 800
#define SCREENY 600
#define HAS_SDLMM 1
#define SIZE 256

#ifdef HAS_SDLMM
#include <sdlmm.h>
#endif
#define LOOP 5000
////////////////////////////////////////////////////
float *A,*B;
int SZ=SIZE;
int invert=0;
int ispause=0;
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
static int isInBound(int y,int x,int sz){
    return y >= 0 && y < sz && x >= 0 && x < sz;
}
static float fetch(const float* a,int y,int x,int sz,int* cnt){
    int inbound=isInBound(y,x,sz);
    *cnt += inbound;
    return a[(y*sz+x)*inbound]*inbound;
}

static void SOR(float* a,const float* b,int sz) {
    int i,e=sz*sz;
#pragma omp parallel for firstprivate(a,b,sz)
    for(i=0; i<e; i++) {
        int x = i % sz;
        int y = i / sz;
        float s=0;
        int cnt=0;
#if 0
        if(x-1>=0){ s += B[y*sz+(x-1)]; ++cnt; } // left
        if(y-1>=0){ s += B[(y-1)*sz+x]; ++cnt; }// up
        if(x+1<sz){ s += B[y*sz+(x+1)]; ++cnt; }// right
        if(y+1<sz){ s += B[(y+1)*sz+x]; ++cnt; }// down
#else
        s+=fetch(b,y,x-1,sz,&cnt);
        s+=fetch(b,y-1,x,sz,&cnt);
        s+=fetch(b,y,x+1,sz,&cnt);
        s+=fetch(b,y+1,x,sz,&cnt);
#endif
        a[i] = s/cnt;
    }
}

void stretchFloatPixels(const float* pixels,int w,int h,int* output,int w2,int h2){
    float dw=((float)w2)/w;
    float dh=((float)h2)/h;
    int ii,e;
    e=h2*w2;
    int currentInvert=invert;
    for(ii=0; ii<e; ii+=1){
        int origi,origj;
        float i=ii%w2;
        float j=ii/w2;
        origi=(int)(i/dw);
        origj=(int)(j/dh);
        if(currentInvert){
           output[(int)(j*w2+i)]=~(int)(pixels[(int)(origj*w+origi)]);
        }else{
           output[(int)(j*w2+i)]=(int)(pixels[(int)(origj*w+origi)]);
        }
    }
}

int* canvas;
static void drawMatrix(float* a,int loop,double tm){
#ifdef HAS_SDLMM
   double t1,t2;
   char buf[1024];
   if(!canvas){
       canvas = (int*)malloc(sizeof(int)*SCREENX*SCREENY);
   }
   t1 = getDoubleTime();
   stretchFloatPixels(a,SZ,SZ,canvas,SCREENX,SCREENY);
   drawpixels(canvas,0,0,SCREENX,SCREENY);
   sprintf(buf,"loop: %-3d/%-3d, time:%-3.4f",loop,LOOP,tm);
   if(invert){
      drawtext(buf,0,0,0);
   }
   else{   
      drawtext(buf,0,0,0xffffff);
   }
  
   flushscreen();
   t2 = getDoubleTime();
   if(tm+t2 - t1 < (1.0/60)){
       delay((1.0/60)-tm-(t2-t1));
   }
   delay(5);
#endif    
}

static void main_run(){
    int i;
    double t1,t2;
    for(i=0; i<SZ*SZ; i++) {
        A[i]=B[i]=0;
    }
    for(i=0; i<LOOP; ++i){
        t1=getDoubleTime();
        if(ispause){
            --i;
            drawMatrix(A,i,t2-t1);
            t2=getDoubleTime();
            memcpy(B,A,sizeof(float)*SZ*SZ);
            continue;
        }
        SOR(A,B,SZ);
        t2=getDoubleTime();
#ifdef HAS_SDLMM
        drawMatrix(A,i,t2-t1);
#endif
        memcpy(B,A,sizeof(float)*SZ*SZ);
    }
}
static void kbfnc(int k,int ctrl,int on){
    if(on){
        switch(k){
            case ' ': case 'i':case 'I': 
               invert=!invert; 
               break;
            case 'r': case 'R': 
               memset(B,0,sizeof(float)*SZ*SZ); 
               memset(A,0,sizeof(float)*SZ*SZ); 
               break;
            case 'p': case 'P':
               ispause = !ispause; 
               break;
        }
    }
}
// handle mouse event
static void mousefnc(int x,int y,int on,int btn){
    float xratio=((float)SZ)/SCREENX;
    float yratio=((float)SZ)/SCREENY;
    if(on){
        x = x * xratio;
        y = y * yratio;
        A[y*SZ+x] = 0xffffff;
        if(y-1 >= 0) A[(y-1)*SZ+x] = 0x0000ff;
        if(y+1 < SZ) A[(y+1)*SZ+x] = 0x00000f;
        if(x-1 >= 0) A[y*SZ+x-1] =0x00000f;
        if(x+1 < SZ) A[(y+1)*SZ+x+1] = 0x00000f;
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
    A = (float*)malloc(sizeof(float)*SZ*SZ);
    B = (float*)malloc(sizeof(float)*SZ*SZ);
#ifdef HAS_SDLMM
    //sdlmm = sdlmm_get_instance("libsdlmm.so");
    screen(SCREENX,SCREENY);
    screentitle("SOR");
    settextfont("FreeMono.ttf",16);
    setonmouse(mousefnc);
    setonmotion(mousemotion);
    setonkey(kbfnc);
#endif
    for(i=0; i<20; ++i){
       main_run(argc,argv);
    }
    free(A);free(B);
    return 0;
}


