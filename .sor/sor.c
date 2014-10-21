#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
//#include <DSM.h>
#define SCREENX 800
#define SCREENY 600
#ifdef SDLMM
#include "sdlmm.h"
#endif
#define dsmMalloc malloc
#define dsmFree free
#define SIZE DEFSIZE
#define LOOP 105
float* A;
float* B;
#define PI 3.1415926
static double tmloopend,tmloopstart;
static int currentLoop,totalLoop=LOOP;
int DSM_Main(){
}

static double getDoubleTime() {
    struct timeval tmv;
    gettimeofday(&tmv,0);
    return ((double)tmv.tv_sec)+1e-6*tmv.tv_usec;
}
static void displayMatrix(float* matrix) {
    char buf[1024];
#ifdef SDLMM
    static double prevtime;
    float pixelPerWidth=((float)SIZE)/SCREENX;
    float pixelPerHeight=((float)SIZE)/SCREENY;
    float prod = pixelPerHeight*pixelPerWidth;
    int x,y,average;
    int hasPrev = 0;
    double tm = getDoubleTime();
    if(prod == 0) prod = 1;
    if(!prevtime) {
        hasPrev = 0;
        prevtime = tm;
    }
    else {
        hasPrev = 1;
        if(tm - prevtime < 0.0166)  return;
    }
    for(y=0; y<SCREENY; ++y) {
        int starty = (int)(y*pixelPerHeight);
        int endy = (int)(starty+pixelPerHeight);
        if(endy <= starty) endy = starty+1;
        for(x=0; x<SCREENX; ++x) {
            int sum = 0;
            int innerx,innery;
            int startx = (int)(x*pixelPerWidth);
            int endx = (int)(startx+pixelPerWidth);
            if(endx <= startx) endx = startx+1;
            for(innery=starty; innery<endy; ++innery) {
                for(innerx=startx; innerx<endx; ++innerx) {
                    sum += matrix[ innery*SIZE+innerx ];
                }
            }
            average = (sum/(prod));
            if(average < 10000 && average != 0) average = 0xffffff/average;
            else average=~average;
     //       printf("drawpixel(%d,%d,%d) from(%d,%d)-(%d,%d)\n",x,y,average,startx,starty,endx,endy);
            drawpixel(x,y,average|0xff000000);
        }
    }
    if(!hasPrev){
        drawtext("Init...",0,0,0xffffff);
        flushscreen();
        sleep(1);
    }
    if(!hasPrev || tm - prevtime >= 0.0166) {
        prevtime = tm;
        tmloopend = getDoubleTime();
        sprintf(buf,"time elapsed: %-16.3lf [%d/%d]",tmloopend-tmloopstart,currentLoop,totalLoop);
        drawtext(buf,0,0,0xffffff);
        flushscreen();
    }
#endif
}


static void init()
{
#ifdef SDLMM
    screen(SCREENX,SCREENY);
    setalwaysflush(0);
    settextfont("/usr/share/fonts/truetype/freefont/FreeSerif.ttf",16);
    settextattr(1);

#endif
    printf("do init\n");
    int i=0;
    for(i=0; i<SIZE; i++) {
        B[i] = 0;
        A[i] = (float)cos((float)i);
    }
//    #pragma omp parallel for
    for(i=0; i<SIZE*SIZE; i++) {
        B[i] = 0xff0000+(i%16)*SIZE*SIZE*(float)sin((float)i*PI);
        A[i] = 0xddff00+(i%16)*SIZE*(float)sin((float)i*PI);
    }

    for(i=(SIZE-1)*SIZE; i<SIZE*SIZE; i++) {
        B[i] = 0;
        A[i] = (float)cos((float)i);
    }
    displayMatrix(A);
    printf("do init done\n");

}

static int inBound ( int x,int y )
{
    return !! ( x < SIZE && y < SIZE && x >= 0 && y >= 0 );
}

static int boundIdx ( int x,int y )
{
    return ( y*SIZE+x ) * inBound ( x,y );
}

static float fetchDataXY ( const float const* fetcha,int x,int y )
{
    int in_bound = inBound ( x,y );
    return fetcha[ ( y*SIZE+x ) *in_bound ] * in_bound;
}

static void writeDataXY ( float* writeC,int x,int y,float data )
{
    writeC[ boundIdx ( x,y ) ] = (writeC[ boundIdx ( x,y ) ]+data)/2;
}

static void sor_point ( float*  srWriteC,const float const* sr,int x,int y )
{
    float sum = 0.0f;
    int cnt = 0;

#if 0
    sum += fetchDataXY ( sr,x,y-1 );
    cnt += ( inBound ( x,y-1 ) );

    sum += fetchDataXY ( sr,x,y+1 );
    cnt += ( inBound ( x,y+1 ) );

    sum += fetchDataXY ( sr,x-1,y );
    cnt += ( inBound ( x-1,y ) );

    sum += fetchDataXY ( sr,x+1,y );
    cnt += ( inBound ( x+1,y ) );
#endif
    sum += fetchDataXY ( sr,x-1,y-1 );
    cnt += ( inBound ( x-1,y-1 ) );
    sum += fetchDataXY ( sr,x,y-1 );
    cnt += ( inBound ( x,y-1 ) );
    sum += fetchDataXY ( sr,x+1,y-1 );
    cnt += ( inBound ( x+1,y-1 ) );

    sum += fetchDataXY ( sr,x-1,y );
    cnt += ( inBound ( x-1,y ) );
    sum += fetchDataXY ( sr,x,y );
    cnt += ( inBound ( x,y ) );
    sum += fetchDataXY ( sr,x+1,y );
    cnt += ( inBound ( x+1,y ) );

    sum += fetchDataXY ( sr,x-1,y+1 );
    cnt += ( inBound ( x-1,y+1 ) );
    sum += fetchDataXY ( sr,x,y+1 );
    cnt += ( inBound ( x,y+1 ) );
    sum += fetchDataXY ( sr,x+1,y+1 );
    cnt += ( inBound ( x+1,y+1 ) );


    writeDataXY ( srWriteC,x,y,sum/cnt );
}

static void sor_once ()
{
    int globalLoop,globalLoopEnd;
    globalLoopEnd = 0;
    #pragma omp parallel
    {
        #pragma omp for
        for ( globalLoop=SIZE; globalLoop<globalLoopEnd-SIZE; ++globalLoop )
        {
            int y = globalLoop / SIZE;
            int x = globalLoop % SIZE;
            sor_point ( B,A,x,y );
        }
    }
#ifdef SDLMM
    displayMatrix(B);
#endif
}

static void sor_once2 ()
{
    int globalLoop,globalLoopEnd;
    globalLoopEnd = SIZE*SIZE;
    #pragma omp parallel
    {
        #pragma omp for
        for ( globalLoop=0; globalLoop<globalLoopEnd; ++globalLoop )
        {
            int y = globalLoop / SIZE;
            int x = globalLoop % SIZE;
            sor_point (A,B,x,y );
        }
    }
#ifdef SDLMM
    displayMatrix(A);
#endif
}

static float reduction()
{
    int globalLoop;
    int globalLoopEnd;
    float sum = 0.0f;
    float v = 0;

    globalLoopEnd = SIZE*SIZE;
    printf("A is at %x\n",A);
    fflush(stdout);
    //printf ( "try reduction\n" );
    for ( globalLoop=0; globalLoop<globalLoopEnd; ++globalLoop )
    {
        sum += A[ globalLoop ];
    }
    printf ( "cpu reduction done...sum=%f\n",sum );
    sum = 0.0f;
    /*#pragma omp parallel for reduction(+:sum) tagname("reduction") \
      problemsize(globalLoopEnd)  */
    for ( globalLoop=0; globalLoop<globalLoopEnd; ++globalLoop )
    {
        sum += A[ globalLoop ];
    }
    printf ( "reduction done...sum=%f\n",sum );

    return sum;
}

static int __ompc_sigma_node_id;

int main(int argc,char** argv)
{
    double tmstart=0,tmend=0,tmrecord=0;

    int i;
    A = (float*)dsmMalloc(SIZE*SIZE*sizeof(float));
    B = (float*)dsmMalloc(SIZE*SIZE*sizeof(float));
    printf("A is at %p~%p, B is at %p~%p\n",A,A+SIZE*SIZE,B,B+SIZE*SIZE);

    if(1 || __ompc_sigma_node_id == 0) init();

    for ( i = 0; i<LOOP; i++ )
    {
        printf("%d\n",i);
        tmstart = getDoubleTime();
        tmloopstart=tmstart;
        currentLoop = i;
        if(i%2==0) sor_once ();
        else sor_once2();

        tmend = getDoubleTime() - tmstart;
        printf("%d:Per_time:%lf\n",__ompc_sigma_node_id,tmend);
        if( i >= 5 ) tmrecord += tmend;
    }

    //printf ( "result = %f\n",reduction() );
    dsmFree(A);
    dsmFree(B);
    printf("%d,%lf\n",SIZE,tmrecord);

    return 0;
}
