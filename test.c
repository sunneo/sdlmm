#include "sdlmm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sched.h>
#define MAXSIZE 10000
static int prevClickX = 0;
static int prevClickY = 0;
static int circnt;
struct Circle {int x,y,r,c,active;};
struct Circle circles[MAXSIZE];
static void touchfnc(int x,int y,int on) {
    prevClickX = x;
    prevClickY = y;
    if(on) {
        int select = circnt%MAXSIZE;
        circles[select].x = x;
        circles[select].y = y;
        circles[select].r = 1;
        circles[select].active = 1;
        circles[select].c = rand();
        ++circnt;
        if(circnt >= MAXSIZE) circnt = 0;
    }
}
static void mousefnc(int x,int y,int on,int btn) {
    touchfnc(x,y,on);
}
int main(int argc, char *argv[]) {
    static const int width =1024;
    static const int height = 768;
    static const int max_radius = 300;
    int i;
    screen(width,height);
    screentitle("Draw Rectangles,Use Circle Param On Click");
    setonmotion(touchfnc);
    setonmouse(mousefnc);
    while(1) {
        fillrect(0,0,width,height,0xffffff);
        for(i=0; i<circnt; ++i) {
           if(circles[i].active) {
               drawline(circles[i].x-circles[i].r,circles[i].y+circles[i].r,circles[i].x,circles[i].y-circles[i].r,circles[i].c);
               drawline(circles[i].x,circles[i].y-circles[i].r,circles[i].x+circles[i].r,circles[i].y+circles[i].r,circles[i].c);
               drawline(circles[i].x-circles[i].r,circles[i].y+circles[i].r,circles[i].x+circles[i].r,circles[i].y+circles[i].r,circles[i].c);
               fillxy(circles[i].x,circles[i].y,circles[i].c);

               //drawrect(circles[i].x-circles[i].r,circles[i].y-circles[i].r,circles[i].r*2,circles[i].r*2,circles[i].c);
               circles[i].r+=1;
               if(circles[i].r>=max_radius)
                   circles[i].active = 0;
           }
        }
        flushscreen();
        delay(1);
    }
    return 0;
}
