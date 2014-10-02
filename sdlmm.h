#ifndef SDLMM_H_
#define SDLMM_H_
void screen(int width,int height);
void setalwaysflush(int doflush);
void flushscreen();
void screentitle(const char* title);
void drawcircle(int x,int y,double r,int color) ;
void drawpixel(int x,int y,int color);
void drawline(int x1,int y1,int x2,int y2,int color);
void drawrect(int x,int y,int w,int h,int color);
void fillrect(int x,int y,int w,int h,int color);
void fillcircle(int x,int y,double r,int color);
void drawpixels(int* pixels,int x,int y,int w,int h);
void loadimage(const char* filename,int** ret,int* w,int *h);
void playwave(short* wave,int len);
//default
void drawtext(const char* str,int x,int y,int color);
void settextfont(const char* font,int fontsize);
void settextattr(int drawtype); // 0: solid, 1:shaded, 2:blended
// interactive
void setontouch(void (*fnc)(int x,int y,int on));
void setonmotion(void (*fnc)(int x,int y,int on));
void setonmouse(void(*fnc)(int x,int y,int on,int btn));
void setonkey(void(*fnc)(int key,int ctrl,int on));

#endif
