#include "sdlmm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
static const int width =1024;
static const int height = 768;
static char msg[1024];
static int colors[3]={0x80ff0000,0x8000ff00,0x800000ff};
static int color;
static int canvas[1024*768];
static int canCover =1;
int pickColor = 0;
static void keyfnc(int key,int ctrl,int on){
    if(on){
       switch(key){
           case '1':
           case '2':
           case '3':    
              color=colors[key-'0'-1];
              break;
           case ' ':
                {
                   int i,j;
                   for(j=0;j<height; ++j){
                      for(i=0; i<width; ++i){
                        canvas[ j*width+i] = 0;//0x00ffffff;
                      }
                   }
                }
               break;
       }
    }
}
static int mouseClicked=0;
static int startX = 0;
static int startY = 0;
static int currentX = -1;
static int currentY = -1;

static void getMinMaxDistance(int a,int b,int* pmin,int* pmax,int* pdistance){
    if(a>b){
        *pmin=b;
        *pmax=a;
        *pdistance = a-b;
    }
    else{
        *pmin=a;
        *pmax=b;
        *pdistance = b-a;
    }
}
static min(int a,int b){
    return a>b?b:a;
}
static max(int a,int b){
    return a>b?a:b;
}
static void mousefnc(int x,int y,int on,int btn){
    if(on){
        if(!mouseClicked){
           startX = x;
           startY = y;
           mouseClicked = 1;
        }
        else{
           currentX = x;
           currentY = y;
        }
    }
    else{
    
        int* pixels;
        int minx,miny,maxx,maxy;
        int w,h;
        int* origCover;
        int* overrideOne;
        if(!mouseClicked) return;
        
        getMinMaxDistance(x,startX,&minx,&maxx,&w);
        getMinMaxDistance(y,startY,&miny,&maxy,&h);
        canCover = 0;
        copyscreen(&origCover,minx,miny,w,h);
        fillrect(minx,miny,w,h,color);
        copyscreen(&pixels,0,0,width,height);
        copyscreen(&overrideOne,minx,miny,w,h);
        canCover = 1;
        memcpy(canvas,pixels,1024*768*sizeof(int));
        free(pixels);
        mouseClicked = 0;
        startX = 0;
        startY = 0;
        currentX = -1;
        currentY = -1;
        {
            int i,j;
            for(j=0; j<h; ++j){
                for(i=0; i<w; ++i){
                     int v = origCover[ j*w+i ];
                     int v2 = overrideOne[ j*w+i ];
                     int alpha1 = (v&0xff000000) >> 24;
                     int alphaMask = alpha1*0x010101010;
                     int alpha2 = (v2&0xff000000) >> 24;
                     int alphaMask2 = alpha2*0x010101010;
                     int newv = ((v&~alphaMask)|v2&~alphaMask2)|(((alpha1+alpha2)/2)<<24);
                     //overrideOne[ j*w+i ] = newv;
                     canvas[ (miny+j)*width+(minx+i) ] = newv;
                }
            }
            free(origCover);
            free(overrideOne);    
        }
    }
}
static void motion(int x,int y,int on){
    mousefnc(x,y,on,0);

      pickColor = canvas[ y*width+x ];
  
}

int main(int argc, char *argv[]) {
    color = colors[0];
    screen(width,height);
    {
        int i,j;
        for(j=0;j<height; ++j){
           for(i=0; i<width; ++i){
              canvas[ j*width+i] = 0;//0x00ffffff;
           }
        }
    }
    screentitle("Draw Rectangles,Click");   
    setonkey(keyfnc);
    settextfont("FreeMono.ttf",24);
    setonmouse(mousefnc);
    setonmotion(motion);
    while(1) {
        if(!canCover) {delay(1); continue;}
        drawpixels(canvas,0,0,width,height);       
        drawtext(msg,0,0,0);
        if(mouseClicked){
           if(currentX != -1 && currentY != -1){
               int minx,miny,maxx,maxy,w,h;
               getMinMaxDistance(startX,currentX,&minx,&maxx,&w);
               getMinMaxDistance(startY,currentY,&miny,&maxy,&h);
               drawrect(minx,miny,w,h,color|0x80000000);
               
           }
        }
        {
           char buf[256];
           sprintf(buf,"pickcolor:%08x",pickColor);
           fillrect(0,0,18*24,30,0x000000);
           drawtext(buf,0,0,0xffffffff);
        }   
        flushscreen();
    }
    return 0;
}
