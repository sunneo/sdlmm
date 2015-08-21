/**
 * farn fraktal(barnsley)
 */
#include "sdlmm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
int main(int argc, char *argv[]) {
    int bw=450,bh=700;
    float x=0,y=0;
    int n;
    screen(bw,bh);
    screentitle("barnsley");
    for(n=0; n<500000; ++n){
      float r=((float)rand())/RAND_MAX;
      float x1,y1;
      if (r<=0.01){
        x=0 ;
        y=0.16*y;
      }
      else if (r<=0.08){
        x=0.2*x-0.26*y;
        y=0.23*x+0.22*y+1.6;
      }
      else if( r<=0.15){
        x=-0.15*x+0.28*y;
        y=0.26*x+0.24*y+0.44;
      }
      else{
        x=0.85*x+0.04*y;
        y=-0.04*x+0.85*y+1.6;
      }
      x1=(x+3)*70;
      y1=bh-y*70;
      drawpixel((int)x1,(int)y1,0xffffff);
      if(n%10000==0){
         flushscreen();
      }
    }
    getchar();
    return 0;
}
