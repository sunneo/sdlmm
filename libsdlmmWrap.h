#ifndef LIBSDLMMWRAP_H_
#define LIBSDLMMWRAP_H_

#include <wchar.h>

typedef struct SDLMM{
   void* file;
   void (*screen)(int width,int height);
   void (*screentitle)(const char* title);
   void (*setalwaysflush)(int doflush);
   void (*setusealpha)(int usealpha);
   void (*flushscreen)();
   void (*drawcircle)(int x,int y,double r,int color) ;
   void (*drawpixel)(int x,int y,int color);
   void (*drawline)(int x1,int y1,int x2,int y2,int color);
   void (*drawrect)(int x,int y,int w,int h,int color);
   void (*fillxy)(int x,int y,int color);
   void (*fillrect)(int x,int y,int w,int h,int color);
   void (*fillcircle)(int x,int y,double r,int color);
   void (*drawpixels)(int* pixels,int x,int y,int w,int h);
   void (*drawpixels2)(int* pixels,int x,int y,int w,int h,int transkey);
   void (*loadimage)(const char* filename,int** ret,int* w,int *h);
   void (*copyscreen)(int** ret,int x,int y,int w,int h);
   void (*stretchpixels)(const int* pixels,int w,int h,int* output,int w2,int h2);
   void (*stretchpixels2)(int** pixels,int w,int h,int w2,int h2);
   void (*mode7render)(float angle,int vx,int vy,int* bg,int bw,int bh,int tx,int ty,int w,int h);
   void* (*mode7render_create_conf)(float groundFactor,float xFactor,float yFactor,int scanlineJump);
   void (*mode7render2)(void* mode,float angle,int vx,int vy,int* bg,int bw,int bh,int tx,int ty,int w,int h);
   void (*playwave)(short* wave,int len);
   void (*loadwav)(const char* filename, short** wav,unsigned int* len );
   void (*settextfont)(const char* font,int fontsize);
   void (*drawtext)(const char* str,int x,int y,int color);
   void (*drawtextw)(const wchar_t* str,int x,int y,int color);
   void (*setontouch)(void (*fnc)(int x,int y,int on));
   void (*setonmotion)(void (*fnc)(int x,int y,int on));
   void (*setonmouse)(void(*fnc)(int x,int y,int on,int btn));
   void (*setonkey)(void(*fnc)(int key,int ctrl,int on));
   int (*run_async)(void (*fnc)(void*),void* param);
   void (*post_async)(void (*fnc)(void*),void* param);
   void (*delay)(int mills);
   void (*screen_msg_loop)(int width,int height,const char* title);
   void (*start_main_drawfnc)(void(*fnc)());
}SDLMM;
SDLMM* sdlmm_get_instance(const char* fname);
#endif
