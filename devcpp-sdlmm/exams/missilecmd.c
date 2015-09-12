/**
 *' Farn Fraktal (barnsley)
 ' 
 ' use sdlmm
 */
#include "sdlmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int buildWidth=64;
static const int buildHeight=64;
static const int width=400;
static const int height=300;
static const int maxRadius=32;
static const int maxMissile=16;
static int score=0;
static int remainMissile=45;
static int remainEnermy=40;
static int maxEnemyMissile=5;
static int* buildTex[2];
static int* missileTex;
static int* launcherTex[5];
static int* bg;
volatile static int mx=0;
volatile static int my=0;
#define ENERMY_SPEED 1

typedef struct Missile{
    int fx,fy,tx,ty;
    float x,y,dx,dy;
    int alive,expl,r,targetBuild;
}Missile;

typedef struct OurLaunchedMissile{
    int tx,ty,r,active,expl;
    float x,y,dx,dy;
}OurLaunchedMissile;

typedef struct Build{
    int left, top, right, bottom, alive,isbuild;
}Build;

static Build build[5];
static Missile enermy[20];
static OurLaunchedMissile launchedMissile[16];

static float frand(){
   return ((float)rand())/RAND_MAX;
}
static void draw_enermy(){
   int i;
   for(i=0; i<20; ++i){
       if(!enermy[i].alive) continue;
       if(enermy[i].expl){
           fillcircle(enermy[i].x,enermy[i].y,enermy[i].r,rand()<<10);
       }
       else{
           drawline(enermy[i].fx,enermy[i].fy,enermy[i].x,enermy[i].y,0xff0000);
           //drawcircle(enermy[i].fx,enermy[i].fy,enermy[i].r,0xff0000);
           fillcircle(enermy[i].x,enermy[i].y,enermy[i].r+1,0xffff00);
           drawcircle(enermy[i].x,enermy[i].y,enermy[i].r,0xff0000);
       }
   }
}

static void update_enermy(){
   int i;
   for(i=0; i<20; ++i){
       if(!enermy[i].alive) continue;
       if(enermy[i].expl){
           if(enermy[i].r >= maxRadius){
               enermy[i].alive=0;
               if(remainEnermy-1>=0)
                  --remainEnermy;
           }
           ++enermy[i].r;
       }
       else{
           {
               int j;
               for(j=0; j<16; ++j){
                   if(!launchedMissile[j].active) continue;
                   if(launchedMissile[j].expl){
                       float distX = (enermy[i].x - launchedMissile[j].x);
                       float distY = (enermy[i].y - launchedMissile[j].y);
                       if(distX*distX+distY*distY < launchedMissile[j].r*launchedMissile[j].r){
                           enermy[i].expl = 1;
                           score+=100;
                           return;
                       }
                   }
               }
               for(j=0; j<20; ++j){
                   if(j == i) continue;
                   if(!enermy[j].alive) continue;
                   if(!enermy[j].expl) continue;
                   float distX = (enermy[i].x - enermy[j].x);
                   float distY = (enermy[i].y - enermy[j].y);
                   if(distX*distX+distY*distY < enermy[j].r*enermy[j].r){
                       enermy[i].expl = 1;
                       score+=100;
                       return;
                   }
               }
           }
           enermy[i].x+=enermy[i].dx;
           enermy[i].y+=enermy[i].dy;
           if( (int)(enermy[i].tx  - enermy[i].x) <= 0 && 0 >= (int)(enermy[i].ty  - enermy[i].y)){
               enermy[i].expl = 1;
               build[enermy[i].targetBuild].alive = 0;
           }
       }
   } 
}

static void generate_enermy(){
    if(remainEnermy > 0){
        int currentAlive=0;
        int i;
        for(i=0; i<20; ++i){
            if(currentAlive >= maxEnemyMissile) break;
            if(enermy[i].alive){
               ++currentAlive;
               continue;
            }
            else{
               int targetIdx,sx,sy,tx,ty;
               targetIdx=(rand()%5);
               sx = (rand() % width); sy = 0;
               tx = (build[targetIdx].left+build[targetIdx].right)/2;
               ty = build[targetIdx].top;
               float dx = ((float)(tx-sx)) / (1024/ENERMY_SPEED);
               float dy = ((float)(ty-sy)) / (1024/ENERMY_SPEED);
               enermy[i].x = enermy[i].fx = sx;
               enermy[i].y = enermy[i].fy = sy;
               enermy[i].dx = dx;
               enermy[i].dy = dy;
               enermy[i].tx = tx;
               enermy[i].ty = ty;
               enermy[i].expl = 0;
               enermy[i].alive = 1;
               enermy[i].r = 2;
               enermy[i].targetBuild = targetIdx;
               ++currentAlive;
            }
        }
   }
}

static void draw_launcher(int x,int y){
   int split=width/5;
   int idx=mx/split;
   int padding=16;
   if(idx < 0) idx = 0;
   if(idx > 4) idx = 4;
   drawpixels2(launcherTex[idx], padding+x,y,32,32,0xff0000);
}
static void load_tex(){
   int dummy1,dummy2;
   loadimage("missile-command/img/Lucid-burn400x300.bmp",&bg,&dummy1,&dummy2);
   loadimage("missile-command/img/build0.bmp",&buildTex[0],&dummy1,&dummy2);
   loadimage("missile-command/img/build.bmp",&buildTex[1],&dummy1,&dummy2);   
   loadimage("missile-command/img/launcher-L2.bmp",&launcherTex[0],&dummy1,&dummy2);
   loadimage("missile-command/img/launcher-L1.bmp",&launcherTex[1],&dummy1,&dummy2);
   loadimage("missile-command/img/launcher.bmp",&launcherTex[2],&dummy1,&dummy2);
   loadimage("missile-command/img/launcher-R1.bmp",&launcherTex[3],&dummy1,&dummy2);
   loadimage("missile-command/img/launcher-R2.bmp",&launcherTex[4],&dummy1,&dummy2);
   loadimage("missile-command/img/missile.bmp",&missileTex,&dummy1,&dummy2);
}

static void init_build(int cnt){
   int i;
   int padding=10;
   int buildtop = height-buildHeight;
   for(i=0; i<cnt; ++i){
      build[i].left = (padding+buildWidth)*i;
      build[i].top = buildtop;
      build[i].right = build[i].left+buildWidth;
      build[i].bottom = height;
      build[i].alive = 1;
      build[i].isbuild=1;
   }
   build[cnt/2].isbuild=0;
   build[cnt/2].top = buildtop+32;
}

static void draw_build(int cnt){
   int i;
   for(i=0; i<cnt; ++i){
      if(build[i].isbuild){
          drawpixels2(buildTex[build[i].alive],build[i].left,build[i].top,buildWidth,buildHeight,0xff0000);
      }
      else{
          draw_launcher(build[i].left,build[i].top);
      }
   }
}

static void update_missile(){
    int i;
    for(i=0; i<maxMissile; ++i){
        if(!launchedMissile[i].active) continue;
        if(!launchedMissile[i].expl){
           if((int)(launchedMissile[i].tx - launchedMissile[i].x) == 0 && 0 == (int)(launchedMissile[i].ty-launchedMissile[i].y)){
               launchedMissile[i].expl=1;
           }
           launchedMissile[i].x+=launchedMissile[i].dx;
           launchedMissile[i].y+=launchedMissile[i].dy;
        }
        else{
           if(launchedMissile[i].r < maxRadius){
              ++launchedMissile[i].r;
           }
           else{
              launchedMissile[i].active=0;
              launchedMissile[i].expl=0;
           }
        }
    }
}
static void drawMessage(){
    char cscore[256];
    char cmissile[256];
    char cenermy[256];
    sprintf(cscore,"Score:%-04d",score);
    sprintf(cmissile,":%04d",remainMissile);
    sprintf(cenermy,"Enermy:%03d",remainEnermy);
    drawtext(cscore,0,0,0xffffff);
    drawpixels2(missileTex,width-80-16,24,16,64,0xff0000);
    drawtext(cmissile,width-80,24,0xffffff);
    drawtext(cenermy,width-160,0,0xffffff);
}
static void draw_missile(){
    int i;
    for(i=0; i<maxMissile; ++i){
        if(!launchedMissile[i].active) continue;
        if(!launchedMissile[i].expl){
           float dx=launchedMissile[i].dx;
           float dy=launchedMissile[i].dy;
           int j;
           fillcircle(launchedMissile[i].x,launchedMissile[i].y,launchedMissile[i].r,0xffff00);
           for(j=0; j<8; ++j){
              fillcircle(launchedMissile[i].x-j*dx,launchedMissile[i].y-j*dy,launchedMissile[i].r+j,0xffffff-0x101010*(j+1));
           }
           
        }
        else{
           fillcircle(launchedMissile[i].x,launchedMissile[i].y,launchedMissile[i].r,rand()<<9); 
        }
    }
}
static void drawfnc(){
    drawpixels(bg,0,0,400,300);
    draw_build(5);
    draw_missile();
    draw_enermy();
    update_missile();
    update_enermy();
    drawMessage();
    if(rand() % 100 < 20) generate_enermy();
    flushscreen();
    delay(16);
}
void onmotion(int x,int y,int on){
    mx = x;
    my = y;
}
static void generate_missile(int mx,int my){
    int i;
    if(remainMissile <= 0) return;
    for(i=0; i<maxMissile; ++i){
        if(!launchedMissile[i].active){
            int sx = build[2].left+32;
            int sy = build[2].top;
            float dx = ((float)(mx-sx))/50;
            float dy = ((float)(my-sy))/50;
            launchedMissile[i].active = 1;
            launchedMissile[i].x = sx;
            launchedMissile[i].y = sy;
            launchedMissile[i].r = 3;
            launchedMissile[i].tx = mx;
            launchedMissile[i].ty = my;               
            launchedMissile[i].dx = dx;
            launchedMissile[i].dy = dy;
            launchedMissile[i].expl=0;
            --remainMissile;
            break;
        }
    }
}
void onmouse(int x,int y,int on,int btn){
    mx = x;
    my = y;
    if(on){
       generate_missile(mx,my);   
    }
}

int main(int argc, char** argv){
   //screen_msg_loop(width,height,"Missile Command [demo]");
   screen(width,height);
   screentitle("Missile Command [demo]");
   setonmotion(onmotion);
   setonmouse(onmouse);
   settextfont("missile-command/fnt/FreeMono.ttf",20);
   load_tex();
   init_build(5);
   while(1){
       drawfnc();
   }
   //start_main_drawfnc(drawfnc);
}

