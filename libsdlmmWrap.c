#include "libsdlmmWrap.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static void* dlsym_z(void* f,const char* name){
   void* ret = 0;
   ret = dlsym(f,name);
   if(!ret){
      fprintf(stderr,"%s(%p,%s) failed\n",__func__,f,name);
   }
   return ret;
}

SDLMM* sdlmm_get_instance(const char* fname){  
   void* dl = 0;
   if(fname == NULL) fname="./libsdlmm.so";
   dl = dlopen(fname,RTLD_LAZY);
   SDLMM* ret;
   if(!dl){
      fprintf(stderr,"%s(%s) failed: %s\n",__func__,fname,dlerror());
      return 0;
   }
   ret = (SDLMM*)malloc(sizeof(SDLMM));

   ret->file = dl;
   ret->screen = (void(*)(int,int))dlsym_z(dl,"screen"); 
   ret->screentitle=(void (*)(const char*))dlsym_z(dl,"screentitle");
   ret->setalwaysflush=(void (*)(int))dlsym_z(dl,"setalwaysflush");
   ret->setusealpha = (void (*)(int))dlsym_z(dl,"setusealpha");
   ret->flushscreen = (void (*)())dlsym_z(dl,"flushscreen");
   ret->drawcircle=(void (*)(int,int,double ,int))dlsym_z(dl,"drawcircle") ;
   ret->drawpixel = (void(*)(int,int,int))dlsym_z(dl,"drawpixel");
   ret->drawline = (void(*)(int,int,int,int,int))dlsym_z(dl,"drawline");
   ret->drawrect = (void(*)(int,int,int,int,int))dlsym_z(dl,"drawrect");
   ret->fillxy = (void (*)(int,int,int))dlsym_z(dl,"fillxy");
   ret->fillrect = (void(*)(int,int,int,int,int))dlsym_z(dl,"fillrect");
   ret->fillcircle = (void(*)(int,int,double,int))dlsym_z(dl,"fillcircle");
   ret->drawpixels = (void(*)(int*,int,int,int,int))dlsym_z(dl,"drawpixels");
   ret->drawpixels2 = (void(*)(int*,int,int,int,int,int))dlsym_z(dl,"drawpixels2");
   ret->loadimage = (void (*)(const char*,int**,int*,int *))dlsym_z(dl,"loadimage");
   ret->copyscreen = (void(*)(int** ,int,int,int,int))dlsym_z(dl,"copyscreen");
   ret->stretchpixels = (void (*)(const int*,int,int,int*,int,int))dlsym_z(dl,"stretchpixels");
   ret->stretchpixels2 = (void(*)(int**,int,int,int,int))dlsym_z(dl,"stretchpixels2");
   ret->mode7render = (void(*)(float,int,int,int*,int,int,int,int,int,int))dlsym_z(dl,"mode7render");
   ret->mode7render_create_conf = (void* (*)(float,float,float,int))dlsym_z(dl,"mode7render_create_conf");
   ret->mode7render2 = (void (*)(void*,float,int,int,int*,int,int,int,int,int,int))dlsym_z(dl,"mode7render2");
   ret->playwave = (void (*)(short*,int)) dlsym_z(dl,"playwave");
   ret->loadwav = (void (*)(const char*, short**,unsigned int*))dlsym_z(dl,"loadwav");
   ret->settextfont = (void (*)(const char*,int))dlsym_z(dl,"settextfont");
   ret->drawtext = (void (*)(const char* ,int,int,int))dlsym_z(dl,"drawtext");
   ret->drawtextw = (void (*)(const wchar_t* ,int,int,int))dlsym_z(dl,"drawtextw");
   ret->setontouch = (void (*)(void (*)(int,int,int)))dlsym_z(dl,"setontouch");
   ret->setonmotion = (void (*)(void (*)(int,int,int)))dlsym_z(dl,"setonmotion");
   ret->setonmouse = (void (*)(void (*)(int,int,int,int)))dlsym_z(dl,"setonmouse");
   ret->setonkey = (void (*)(void(*)(int,int,int)))dlsym_z(dl,"setonkey");
   ret->run_async = (int (*)(void (*)(void*),void*))dlsym_z(dl,"run_async");
   ret->post_async = (void (*)(void (*)(void*),void*))dlsym_z(dl,"post_async");
   ret->delay = (void(*)(int))dlsym_z(dl,"delay");
   ret->screen_msg_loop = (void(*)(int,int,const char*))dlsym_z(dl,"screen_msg_loop");
   ret->start_main_drawfnc = (void(*)(void(*)()))dlsym_z(dl,"start_main_drawfnc");
   return ret;

}
static SDLMM* defaultSDLMM;
__inline static SDLMM* getInstance(){
   if(!defaultSDLMM) defaultSDLMM=sdlmm_get_instance("./libsdlmm.so");
   return defaultSDLMM;
}
void screen(int width,int height){   getInstance()->screen(width,height); }
void screentitle(const char* title){ getInstance()->screentitle(title); }
void setalwaysflush(int doflush){ getInstance()->setalwaysflush(doflush); }
void setusealpha(int usealpha){ getInstance()->setusealpha(usealpha); }
void flushscreen(){ getInstance()->flushscreen(); }
void drawcircle(int x,int y,double r,int color){ getInstance()->drawcircle(x,y,r,color); }
void drawpixel(int x,int y,int color){ getInstance()->drawpixel(x,y,color); } 
void drawline(int x1,int y1,int x2,int y2,int color){ getInstance()->drawline(x1,y1,x2,y2,color); }
void drawrect(int x,int y,int w,int h,int color){ getInstance()->drawrect(x,y,w,h,color); }
void fillxy(int x,int y,int color){ getInstance()->fillxy(x,y,color); } 
void fillrect(int x,int y,int w,int h,int color){ getInstance()->fillrect(x,y,w,h,color); } 
void fillcircle(int x,int y,double r,int color){ getInstance()->fillcircle(x,y,r,color); }
void drawpixels(int* pixels,int x,int y,int w,int h){ getInstance()->drawpixels(pixels,x,y,w,h); }
void drawpixels2(int* pixels,int x,int y,int w,int h,int transkey){ getInstance()->drawpixels2(pixels,x,y,w,h,transkey);}
void loadimage(const char* filename,int** ret,int* w,int *h){ getInstance()->loadimage(filename,ret,w,h); }
void copyscreen(int** ret,int x,int y,int w,int h){ getInstance()->copyscreen(ret,x,y,w,h); }
void stretchpixels(const int* pixels,int w,int h,int* output,int w2,int h2){ getInstance()->stretchpixels(pixels,w,h,output,w2,h2); }
void stretchpixels2(int** pixels,int w,int h,int w2,int h2){ getInstance()->stretchpixels2(pixels,w,h,w2,h2); }
void mode7render(float angle,int vx,int vy,int* bg,int bw,int bh,int tx,int ty,int w,int h){ getInstance()->mode7render(angle,vx,vy,bg,bw,bh,tx,ty,w,h); }
void* mode7render_create_conf(float groundFactor,float xFactor,float yFactor,int scanlineJump){ return getInstance()->mode7render_create_conf(groundFactor,xFactor,yFactor,scanlineJump); }
void mode7render2(void* mode,float angle,int vx,int vy,int* bg,int bw,int bh,int tx,int ty,int w,int h){ getInstance()->mode7render2(mode,angle,vx,vy,bg,bw,bh,tx,ty,w,h); }
void playwave(short* wave,int len){ getInstance()->playwave(wave,len); }
void loadwav(const char* filename, short** wav,unsigned int* len ){ getInstance()->loadwav(filename,wav,len); }
void settextfont(const char* font,int fontsize){ getInstance()->settextfont(font,fontsize); }
void drawtext(const char* str,int x,int y,int color){ getInstance()->drawtext(str,x,y,color); }
#include <wchar.h>
void drawtextw(const wchar_t* str,int x,int y,int color){ getInstance()->drawtextw(str,x,y,color); }
void setontouch(void (*fnc)(int x,int y,int on)){ getInstance()->setontouch(fnc); }
void setonmotion(void (*fnc)(int x,int y,int on)){ getInstance()->setonmotion(fnc); }
void setonmouse(void(*fnc)(int x,int y,int on,int btn)){getInstance()->setonmouse(fnc); }
void setonkey(void(*fnc)(int key,int ctrl,int on)){getInstance()->setonkey(fnc); }
int run_async(void (*fnc)(void*),void* param){getInstance()->run_async(fnc,param); }
void post_async(void (*fnc)(void*),void* param){getInstance()->post_async(fnc,param); }
void delay(int mills){getInstance()->delay(mills); }
void screen_msg_loop(int width,int height,const char* title){getInstance()->screen_msg_loop(width,height,title); }
void start_main_drawfnc(void(*fnc)()){getInstance()->start_main_drawfnc(fnc); }
