#if 1
#include <wchar.h>
#include "sdlmm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define MAXSIZE 10000
static int prevClickX = 0;
static int prevClickY = 0;
static int circnt;
struct Circle {
int x,y,r,c,active;
};
struct Circle circles[MAXSIZE];
void touchfnc(int x,int y,int on) {
    prevClickX = x;
    prevClickY = y;
    if(on) {
        int select = circnt%MAXSIZE;
        circles[select].x = x;
        circles[select].y = y;
        circles[select].r = 1;
        circles[select].active = 1;
        circles[select].c = rand();//(circnt|circnt<<8|circnt<<16);
        ++circnt;
        if(circnt >= MAXSIZE) circnt = 0;
    }
}
void mousefnc(int x,int y,int on,int btn) {
    touchfnc(x,y,on);
}
static void* sndPlayer(void* p){
    int i;
    int sndidx=0;

    short snd[40960];
    for(i=0; i<40960; i+=2){
       snd[ i ] = (short)(32767 * sin(3.14f*i+100)+ 32767*sin(3.14*550+i) );
       snd[ i+1 ] = (short)(32767 * sin(3.14f*i+340)+ 32767*sin(3.14*550+i) );
       printf("%d\n",snd[i]);
    }
    while(1){
        playwave(snd+sndidx,10240);
        sndidx += 10240;
        sndidx %= 40960;
    }
    
}
int main(int argc, char *argv[]) {
    static const int width =1024;
    static const int height = 768;
    static const int max_radius = 64;
    int i;
    int* pic,pic_w,pic_h;
    screen(width,height);

    screentitle("DrawRects");
    settextfont("FreeMono.ttf",45);
    setonmotion(touchfnc);
    setonmouse(mousefnc);
    loadimage("Lucid-burn-white_1024_768.png",&pic,&pic_w,&pic_h);
    {
       pthread_t ppsnd;
       pthread_create(&ppsnd,NULL,sndPlayer,NULL);
    }
    while(1) {
        //fillrect(0,0,width,height,0x000000);
        drawpixels(pic,0,0,pic_w,pic_h);
        #pragma omp parallel for
        for(i=0; i<circnt; ++i) {
            if(circles[i].active) {
                drawcircle(circles[i].x,circles[i].y,circles[i].r,circles[i].c);
                circles[i].r+=1;
                if(circles[i].r>=500)
                    circles[i].active = 0;
            }
        }
        //fillrect(100,100,300,50,0xffffff);
        for(i=0; i<20; ++i) {
            //0xff000000|rand()|rand()<<16
            drawtext( "Hello World",0,0,0xf0ffff00);
        }
        flushscreen();
    }
    return 0;
}

#endif
