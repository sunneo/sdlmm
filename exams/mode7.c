#include "sdlmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
static const int width=1024;
static const int height=768;
static int* bg;
static int* bg_small;
static volatile int mx,my,vx=1440,vy=1440,bh,bw,ox,oy;
static int UP,LEFT,DOWN,RIGHT,TLEFT,TRIGHT;
static char msg[1024];
static float angle = 0;

static void drawfnc(){
    fillrect(0,0,width,height,0xcccccc);
    mode7render(angle,vx,vy,bg,bw,bh,0,height/2,width,500);
    
    drawpixels(bg_small,width-400,0,400,400);
    fillcircle(width-400+400*(((double)vx)/bw),400*(((double)vy)/bh),5,0xffffff);
    fillcircle(width-400+400*(((double)vx)/bw),400*(((double)vy)/bh),3,0x0000ff);
    
    
    fillrect(0,0,100,22,0xffffff);
    sprintf(msg,"%-4d,%-4d,vx=%-3d,vy=%-3d,angle=%-5.2f", mx,my,vx,vy,angle);
    drawtext(msg,0,0,0x0);
    flushscreen();
    delay(15);
}

static void handlekb(){
    static float delta=5;
	float s = ((float)((height/2)-my)/(height/2))*2*delta;
	if(ox < mx) TLEFT=1;
	else if(ox > mx) TRIGHT=1;
	else if(ox==mx) TLEFT=TRIGHT=0;
	ox=mx;
	
    if(LEFT) vx -= delta;//sin(-angle+3.1415926)*s;
	if(RIGHT) vx += delta;//sin(-angle+3.1415926)*s;
	if(UP)vy += delta;//cos(-angle+3.1415926)*s;
	if(DOWN) vy -= delta;//cos(-angle+3.1415926)*s;
	if(vy<100) vy=100;
	if(vy>bh-100) vy=bh-100;
	if(vx > bw-100) vx=bw-100;
	if(vx < 100) vx=100;
    if(TLEFT)angle -= /*(mx - width/2)*/0.01*delta;
    if(TRIGHT)angle += /*(mx - width/2)*/0.01*delta;
}
static void kb(int k,int c,int o){
    switch(k){
        case 'W':case 'w': UP=o; break;
        case 'A':case 'a': LEFT=o; break;
        case 'S':case 's': DOWN=o; break;
        case 'D':case 'd': RIGHT=o; break;
        case 'O':case 'o': TLEFT=o; break;
        case 'P':case 'p': TRIGHT=o; break;
    }
}
static void mouse(int x,int y,int on){
    ox=mx;
    oy=my;
    mx=x;
    my=y;
}
int main(int argc, char** argv){
   screen(width,height);
   screentitle("Mode7 Demo");
   setonkey(kb);
   setonmotion(mouse);
   settextfont("FreeMono.ttf",20);
   loadimage("g.bmp",&bg,&bw,&bh);
   loadimage("g.bmp",&bg_small,&bw,&bh);
   stretchpixels2(&bg_small,bw,bh,400,400);
   
   while(1){
       handlekb();
       drawfnc();
   }
}

