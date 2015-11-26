#include <wchar.h>
#include "sdlmm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#define SAMPLES 88200
#define MAX_VOLUME 32767
#define HZ_INC 10
static const int width =1024;
static const int height = 768;
static short snd[SAMPLES];
static char keymsg_buf[1024];
static float hz=995;
int keystate;

static void generateWave(){
    int i;
    memset(snd,0,sizeof(short)*SAMPLES);
    for(i=0; i<SAMPLES; i++){
        snd[ i ] = (int)(MAX_VOLUME * sin(2.0*3.1415926*(i)*hz/SAMPLES));       
    }
}
static void inchz(){
    int i;
    if(hz + HZ_INC < SAMPLES){
        hz+=HZ_INC;
    }   
    sprintf(keymsg_buf,"Up  :%f",hz);
}
static void dechz(){
    int i;
    if(hz - HZ_INC > 0){
        hz-=HZ_INC;
    }
    sprintf(keymsg_buf,"Down:%f",hz);
}

static void drawwav(){
   int i;
   int prevx=0,prevyL=768/2,prevyR=768/4;
   float ratio=MAX_VOLUME/(768/4);
   for(i=0; i<1024; ++i){
       int j;
       int piece=SAMPLES/1024;
       float sumL=0,sumR=0;
       float prevsumL=0,prevsumR=0;
       for(j=piece*i; j<piece*(i+1); j+=2){
           sumL+=snd[j];
           sumR+=snd[j+1];
       }
       sumL/=piece; sumR/=piece;
       prevsumL=768/2+(sumL/ratio);
       prevsumR=768/4+(sumR/ratio);
       drawline(prevx,prevyL,i,prevsumL,0x00ffff);
       drawline(prevx,prevyR,i,prevsumR,0xffff00);
       prevx=i;
       prevyL=prevsumL;
       prevyR=prevsumR;
   } 
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
        playwave(snd+sndidx,SAMPLES);
        sndidx += SAMPLES;
        if(sndidx >= SAMPLES){
            sndidx = 0;
        }
    }
}

int main(int argc, char *argv[]) {
    screen(width,height);
    screentitle("PlayWave");
    settextfont("FreeMono.ttf",45);
    setonkey(keyfnc);
    run_async(sndPlayer,NULL);
    while(1) {
        fillrect(0,0,width,height,0);
        drawtext( "Press Something",0 ,0,0xf0ffff00);
        if(keystate==1){
            inchz();
        }
        else if(keystate == -1){
            dechz();
        }
        generateWave();
        drawwav();
        drawtext(keymsg_buf ,0,45,0xf0ffff00);        
        flushscreen();
    }
    return 0;
}

