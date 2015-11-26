#include <wchar.h>
#include "sdlmm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#define SAMPLES 16000
#define SAMPLES_EMIT (4000)
#define MAX_VOLUME 32767
#define HZ_INC 10
static const int width =1024;
static const int height = 768;
static short snd[SAMPLES];
static char keymsg_buf[1024];
static float hz=995;
int keystate;
static float musicHZs[] = {
  220.00,// Hz A (La)
  246.00,// Hz B (Si)
  261.00,// Hz C (Do)
  293.00,// Hz D (Re)
  329.00,// Hz E (Mi)
  349.00,// Hz F (Fa)
  392.00,// Hz G (Sol)    
};

static int changed=0;
static void generateWave(){
    int i;
    if(!changed) return;
    short* psnd=snd;
#pragma omp parallel for firstprivate(psnd,hz)
    for(i=0; i<SAMPLES; i++){
          psnd[ i ] = (int)(MAX_VOLUME * sin(2.0*3.1415926*(i)*(hz)/44100));       
    }
    changed=0;
}
static void inchz(){
    int i;
    if(hz + HZ_INC < SAMPLES){
        hz+=HZ_INC;
    }   
    changed=1;
    sprintf(keymsg_buf,"Up  :%f",hz);
}
static void dechz(){
    int i;
    if(hz - HZ_INC > 0){
        hz-=HZ_INC;
    }
    changed=1;
    sprintf(keymsg_buf,"Down:%f",hz);
}

static int startWavePosDraw=0;
static int rangeWavePosDraw=1024;

static void drawWavInRange(int start,int end){
   int i;
   int prevx=0,prevyL=768/2,prevyR=768/4;
   float ratio=MAX_VOLUME/(768/4);
   for(i=0; i<1024; ++i){
       int j;
       int piece=(end-start+1)/1024;
       float sumL=0,sumR=0;
       float prevsumL=0,prevsumR=0;
       for(j=piece*i; j<piece*(i+1)&&j+2<end; j+=2){
           sumL+=snd[start+j];
           sumR+=snd[start+j+1];
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
static void drawwav(){
   drawWavInRange(startWavePosDraw,startWavePosDraw+rangeWavePosDraw);
   startWavePosDraw++;;
   startWavePosDraw%=SAMPLES;
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
    switch(key){
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': 
        sprintf(keymsg_buf,"Music:%f",hz);
        hz = musicHZs[key-'0'-1];
        changed=1;

        break;
    }
    if(key == 285 && ctrl == 256){
        exit(0);
    }
}
static void sndPlayer(void* p){
    int i;
    int sndidx=0;
    while(1){
        playwave(snd+sndidx,SAMPLES_EMIT);
        sndidx += SAMPLES_EMIT;
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

