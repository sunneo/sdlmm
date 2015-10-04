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
    float viewY=cos(-angle);
    float viewX=sin(-angle);
    int mapCenterX=width-400+400*(((double)vx)/bw);
    int mapCenterY=400*(((double)vy)/bh);
    fillrect(0,0,width,height,0xcccccc);
    mode7render(angle,vx,vy,bg,bw,bh,0,height/2,width,500);
    
    drawpixels(bg_small,width-400,0,400,400);
    fillcircle(mapCenterX,mapCenterY,5,0xffffff);
    fillcircle(mapCenterX,mapCenterY,3,0x0000ff);
    drawline(mapCenterX,mapCenterY,mapCenterX+viewX*15,mapCenterY+viewY*15,0xffff00);
    drawline(mapCenterX+2,mapCenterY,mapCenterX+viewX*15,mapCenterY+viewY*15,0xff0000);
    drawline(mapCenterX-2,mapCenterY,mapCenterX+viewX*15,mapCenterY+viewY*15,0xffff00);
    
    fillrect(0,0,100,22,0xffffff);
    sprintf(msg,"%-4d,%-4d,vx=%-3d,vy=%-3d,angle=%-5.2f", mx,my,vx,vy,angle);
    drawtext(msg,0,0,0x0);
    flushscreen();
    delay(5);
}

static void handlekb(){
    static float delta=5;
    
    int deltaX=abs(ox-mx);
	float s = ((float)((height/2)-my)/(height/2))*2*delta;
	if(ox < mx) TLEFT=1;
	else if(ox > mx) TRIGHT=1;
	
	// First Shooting View
	if(UP || DOWN){
	    float sY=cos(angle)*delta;
        float sX=sin(fabs(angle))*delta;
	    vx+=sX*(DOWN?1:-1);
	    vy+=sY*(DOWN?-1:1); 
	}
	if(LEFT || RIGHT){
	    float sY=cos(fabs(angle))*delta;
        float sX=sin(fabs(angle))*delta;
	    vx-=sY*(LEFT?-1:1);
	    vy-=sX*(LEFT?-1:1); 
	}
	
	if(vy<100) vy=100;
	if(vy>bh-100) vy=bh-100;
	if(vx > bw-100) vx=bw-100;
	if(vx < 100) vx=100;
    if(TLEFT)angle += /*(mx - width/2)*/deltaX*0.01*delta;
    if(TRIGHT)angle -= /*(mx - width/2)*/deltaX*0.01*delta;
    if(ox==mx) TLEFT=TRIGHT=0;
	ox=mx;
}
static void kb(int k,int c,int o){
    switch(k){
        case 273:case 'W':case 'w': UP=o; break;
        case 276: case 'A':case 'a': LEFT=o; break;
        case 275: case 'S':case 's': DOWN=o; break;
        case 274: case 'D':case 'd': RIGHT=o; break;
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

