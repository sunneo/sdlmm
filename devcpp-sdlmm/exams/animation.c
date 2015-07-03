#include "sdlmm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "anime_engine.h"
#define SCREEN_X 800
#define SCREEN_Y 600
static const int FPS=60;


int main (int argc, char** argv)
{
   void* animation;
   screen (SCREEN_X, SCREEN_Y);
   screentitle("Animation");
   animation = anime_load("anime.txt",FPS);
   if(!animation){
       fprintf(stderr,"Not found file:anime.txt");
       return 0;
   }
   while(1){
       fillrect(0,0,SCREEN_X,SCREEN_Y,0xffffff);
       anime_show(animation);
       anime_advance(animation);
       flushscreen();
       delay(1000/FPS);
   }
}


