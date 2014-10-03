#include "sdlmm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static char keymsg_buf[1024];
static int hz=0;
static int valmap[3]={0,1,-1};
static const char* keystr[3]={"None","Up","Down"};
static int keystate;

static void onkeyPress(){
    hz += valmap[keystate];
    sprintf(keymsg_buf,"%-4s:%d",keystr[keystate],hz);
}

static void keyfnc(int key,int ctrl,int on){
    if(!on){
        keystate = 0;
        return;
    }
    switch(key){
        default: return;
        case 273: keystate = 1; break;
        case 274: keystate = 2; break;
        case 285: if(ctrl == 256) exit(0);
    }
}
int main(int argc, char *argv[]) {
    static const int width =250;
    static const int height = 100;
    screen(width,height);
    screentitle("Press Up/Down Key");
    settextfont("FreeMono.ttf",20);
    setonkey(keyfnc);
    while(1) {
        fillrect(0,0,width,height,0xffffff);
        drawtext( "Press Up/Down Key",0 ,0,0);
        onkeyPress();
        drawtext(keymsg_buf ,0,45,0);        
        flushscreen();
    }
    return 0;
}
