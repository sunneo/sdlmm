#include "sdlmm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define SCREEN_X 800
#define SCREEN_Y 600
static const int FPS=60;
typedef struct Bitmap{
    int* pixels;
    int width;
    int height;
}Bitmap;

typedef struct Animation{
   Bitmap** bitmapSeq;
   int bitmapSeqCnt;
   int* x_translation;
   int* y_translation;
   int* translation_duration_frame; 
   int* sequenceCode;
   int seqCnt;
}Animation;

typedef struct AnimationPlayContext{
    Animation* animation;
    int x,y;
    int dx,dy;
    int currentFrame;
    int sequenceIdx;
}AnimationPlayContext;

static Bitmap* loadBMP(const char* filename){
    Bitmap* ret = (Bitmap*)malloc(sizeof(Bitmap));
    loadimage(filename,&ret->pixels,&ret->width,&ret->height);
    return ret;
}
static Animation* loadAnimationDescription(const char* fname){
    FILE* fp = fopen(fname,"r");
    Animation* ret;
    if(fp){
        int i,cnt;
        char buf[1024];
        if(fgets(buf,1024,fp)){
            sscanf(buf,"%d",&cnt);
        }
        ret = (Animation*)malloc(sizeof(Animation));
        if(cnt > 0){
            ret->bitmapSeq = (Bitmap**)malloc(sizeof(Bitmap*)*cnt);
            ret->bitmapSeqCnt = cnt;
        }
        for(i=0; i<cnt; ++i){
            if(fgets(buf,1024,fp)){
                char* newline;
                newline=strrchr(buf,'\n');
                if(newline!=NULL) *newline='\0';
                ret->bitmapSeq[i] = loadBMP(buf);
            }
        }
        if(fgets(buf,1024,fp)){
            sscanf(buf,"%d",&cnt);
            if(cnt>0){
                ret->seqCnt = cnt;
                ret->x_translation = (int*)malloc(sizeof(int)*cnt);
                ret->y_translation = (int*)malloc(sizeof(int)*cnt);
                ret->translation_duration_frame = (int*)malloc(sizeof(int)*cnt);
                ret->sequenceCode = (int*)malloc(sizeof(int)*cnt);
            }
            for(i=0; i<cnt; ++i){
               if(fgets(buf,1024,fp)){
                   int duration=0;
    
                   sscanf(buf,"%d %d %d %d",
                      &ret->x_translation[i],&ret->y_translation[i],&duration,&ret->sequenceCode[i]);
                   // the unit of duration is in "ms"
                   // 1 sec = 1000 ms
                   // 1 sec : 30 frames
                   // 1 frame = 33 ms
                   // duration -> frames = duration / (1000/FPS)
                   ret->translation_duration_frame[i] = duration / (1000/(FPS));
               }
            }
        }
    }
    return ret;
}
static void animationPlayContextNextStatus(AnimationPlayContext* a,int seq){
    if(a->sequenceIdx+1 < a->animation->seqCnt){
       a->dx = a->animation->x_translation[a->sequenceIdx+1] - a->animation->x_translation[a->sequenceIdx];
       a->dy = a->animation->y_translation[a->sequenceIdx+1] - a->animation->y_translation[a->sequenceIdx];
       if(a->animation->translation_duration_frame[a->sequenceIdx] > 0){
           a->dx/= a->animation->translation_duration_frame[a->sequenceIdx];
           a->dy/= a->animation->translation_duration_frame[a->sequenceIdx];
       }
       a->x= a->animation->x_translation[a->sequenceIdx];
       a->y= a->animation->y_translation[a->sequenceIdx];
    }
    else{
        a->dx=0;
        a->dy=0;
        a->x =a->animation->x_translation[0];
        a->y =a->animation->x_translation[0];
    }
    
}

static AnimationPlayContext* createAnimationPlayContext(Animation* animation){
    AnimationPlayContext* ret = (AnimationPlayContext*)malloc(sizeof(AnimationPlayContext));
    ret->animation = animation;
    ret->currentFrame = 0;
    ret->sequenceIdx = 0;
    animationPlayContextNextStatus(ret,0);
    return ret;
}

static void advanceAnimationPlayContext(AnimationPlayContext* ctx){
    Bitmap* bmp;
    int seqIdx,seqCode;
    if(ctx->sequenceIdx >= ctx->animation->seqCnt){
        ctx->sequenceIdx = 0;
    }
    seqIdx=ctx->sequenceIdx;
    seqCode=ctx->animation->sequenceCode[seqIdx];
    bmp = ctx->animation->bitmapSeq[seqCode ];
    drawpixels(bmp->pixels,ctx->x,ctx->y,bmp->width,bmp->height);
    ctx->x+=ctx->dx;
    ctx->y+=ctx->dy;
    ++ctx->currentFrame;
    if(ctx->currentFrame >= ctx->animation->translation_duration_frame[ctx->sequenceIdx]){
       ctx->currentFrame=0;
       ++ctx->sequenceIdx;
       animationPlayContextNextStatus(ctx,ctx->sequenceIdx);
       if(ctx->sequenceIdx>=ctx->animation->seqCnt){
           ctx->sequenceIdx = 0;
       }    
    }
}

/**
 * example of input file
5
C:\Users\HPDS\devcpp-sdlmm\bin\anim\pics\0.bmp
C:\Users\HPDS\devcpp-sdlmm\bin\anim\pics\1.bmp
C:\Users\HPDS\devcpp-sdlmm\bin\anim\pics\2.bmp
C:\Users\HPDS\devcpp-sdlmm\bin\anim\pics\3.bmp
C:\Users\HPDS\devcpp-sdlmm\bin\anim\pics\4.bmp
33
100 100 200 0
400 100 200 1
400 400 200 2
400 100 100 3
100 100 100 4
400 100 100 3
100 400 100 2
200 100 100 1
100 100 100 0
100 100 10  1
100 100 10  2
100 100 10  3
100 100 10  4
100 100 10  3
100 100 10  2
100 100 10  1
100 100 10  0
100 100 10  1
100 100 10  2
100 100 10  3
100 100 10  4
100 100 10  3
100 100 10  2
100 100 10  1
100 100 10  0
100 100 10  1
100 100 10  2
100 100 10  3
100 100 10  4
100 100 10  3
100 100 10  2
100 100 10  1
100 100 10  0
*/
  
int main (int argc, char** argv)
{
   Animation* animation;
   AnimationPlayContext* playctx;
   screen (SCREEN_X, SCREEN_Y);
   screentitle("Sort");
   animation = loadAnimationDescription("anime.txt");
   if(!animation){
       fprintf(stderr,"Not found file:anime.txt");
       return 0;
   }
   playctx=createAnimationPlayContext(animation);
   while(1){
       fillrect(0,0,SCREEN_X,SCREEN_Y,0xffffff);
       advanceAnimationPlayContext(playctx);
       flushscreen();
       delay(1000/FPS);
   }
}


