#ifndef SDLMM_H_
#define SDLMM_H_



void screen(int width,int height);
void screentitle(const char* title);
void setalwaysflush(int doflush);
void setusealpha(int usealpha);
void flushscreen();



/// draw /////////////////////////////////////////////
void drawcircle(int x,int y,double r,int color) ;
void drawpixel(int x,int y,int color);
void drawline(int x1,int y1,int x2,int y2,int color);
void drawrect(int x,int y,int w,int h,int color);
void fillxy(int x,int y,int color);
void fillrect(int x,int y,int w,int h,int color);
void fillcircle(int x,int y,double r,int color);
void drawpixels(int* pixels,int x,int y,int w,int h);
void drawpixels2(int* pixels,int x,int y,int w,int h,int transkey);
//load picture
void loadimage(const char* filename,int** ret,int* w,int *h); 
void copyscreen(int** ret,int x,int y,int w,int h);
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////
// directly playwave (44100Hz, 2 channels)
void playwave(short* wave,int len);
//load wave file
void loadwav(const char* filename, short** wav,unsigned int* len );
//////////////////////////////////////////////////////



/// text //////////////////////////////////////////////////
void settextfont(const char* font,int fontsize);
// draw text at x,y with color
void drawtext(const char* str,int x,int y,int color);
// utf8 support
#include <wchar.h>
void drawtextw(const wchar_t* str,int x,int y,int color);
///////////////////////////////////////////////////

// interactive /////////////////////////////////////////
void setontouch(void (*fnc)(int x,int y,int on));
void setonmotion(void (*fnc)(int x,int y,int on));
void setonmouse(void(*fnc)(int x,int y,int on,int btn));
void setonkey(void(*fnc)(int key,int ctrl,int on));
/////////////////////////////////////////////////////////

int run_async(void (*fnc)(void*),void* param);
void post_async(void (*fnc)(void*),void* param);
void delay(int mills);
#endif
