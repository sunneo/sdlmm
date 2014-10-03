#if 1
#include <wchar.h>
#include "sdlmm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#define MAXSIZE 10000
static int prevClickX = 0;
static int prevClickY = 0;
static int circnt;
struct Circle {
int x,y,r,c,active;
};
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
        circles[select].c = circnt|circnt<<8|circnt<<16;
        ++circnt;
        if(circnt >= MAXSIZE) circnt = 0;
    }
}
static void mousefnc(int x,int y,int on,int btn) {
    touchfnc(x,y,on);
}

static short snd[88200];
static char keymsg_buf[1024];
#define HZ_INC 1
static int hz=995;
int keystate;
static void inchz(){
    int i;
    if(hz + HZ_INC < 88200){
        hz+=HZ_INC;
    }
    memset(snd,0,sizeof(short)*88200);
    for(i=0; i<88200; i+= (int)(88200.0/hz)){
        snd[ i ] = 16384;
        if(i+1 < 88200){
            snd[ i+1 ] = -16384;
        }
    }
    sprintf(keymsg_buf,"Up  :%d",hz);
}
static void dechz(){
    int i;
    if(hz - HZ_INC > 0){
        hz-=HZ_INC;
    }
    
    memset(snd,0,sizeof(short)*88200);
    for(i=0; i<88200; i+=(int)(88200.0 /hz)){
        snd[ i ] = 16384;
        if(i+1 < 88200){
            snd[ i+1 ] = -16384;
        }
    }
    sprintf(keymsg_buf,"Down:%d",hz);
}

static void keyfnc(int key,int ctrl,int on){
    if(key == 273){
        if(on)keystate = 1;
        else keystate = 0;
    }
    else if(key == 274){
        if(on)keystate = -1;
        else keystate = 0;
    }
    if(key == 285 && ctrl == 256){
        exit(0);
    }
}
static void sndPlayer(void* p){
    int i;
    int sndidx=0;
    while(1){
        playwave(snd+sndidx,1024);
        sndidx += 1024;
        if(sndidx >= 88200){
            sndidx = 0;
        }
    }
}
static void musicPlayer(void* p){
    short* wav;
    unsigned int len;
    loadwav("music.wav",&wav,&len);
    playwave(wav,len);
    free(wav);
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
    setonkey(keyfnc);
    loadimage("Lucid-burn-white_1024_768.png",&pic,&pic_w,&pic_h);
    run_async(musicPlayer,NULL);
    while(1) {
        drawpixels(pic,0,0,pic_w,pic_h);
        for(i=0; i<circnt; ++i) {
           if(circles[i].active) {
               drawcircle(circles[i].x,circles[i].y,circles[i].r,circles[i].c);
               circles[i].r+=1;
               if(circles[i].r>=max_radius)
                   circles[i].active = 0;
           }
        }
        drawtext( "Press Something",0 ,0,0xf0ffff00);
        if(keystate==1){
            inchz();
        }
        else if(keystate == -1){
            dechz();
        }
        drawtext(keymsg_buf ,0,45,0xf0ffff00);        
        flushscreen();
    }
    return 0;
}

#endif
