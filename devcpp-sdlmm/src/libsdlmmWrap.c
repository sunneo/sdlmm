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
   void* dl = dlopen(fname,RTLD_LAZY);
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

