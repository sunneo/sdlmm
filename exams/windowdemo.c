#include "sdlmm.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define SCREENX 1000
#define SCREENY 700


static int iconsize=32;
static int scrollbarWidth=12;
#define WIN_DRAGGING 1
#define WIN_RESIZINGX 2
#define WIN_RESIZINGY 4
#define WIN_RESIZING 6
#define WIN_CLOSING 8
typedef struct Rect{
   int x,y,w,h,movable,resizable,zindex;
   int PrevX,PrevY,event;
   int (*onClick)(struct Rect* obj,int x,int y,int on,int btn);
   int (*onMove)(struct Rect* obj,int x,int y,int on);
   void (*onDraw)(struct Rect* obj);
   void (*Moved)(struct Rect* w,int origX,int origY);
   void (*Resized)(struct Rect* w,int origW, int origH);
}Rect;

typedef struct Drawable{
   Rect super;
   void (*onPaint)(struct Drawable*);
   int bgcolor;
   int frcolor;
}Drawable;

typedef struct Window{
   Drawable super;
   char title[256];
   int iconsize;
   int titleWidth;
   int controlSize;
   int titlebgclr;
   int titlefrclr;
   int borderclr,border;
   void (*Closing)(struct Window* w);
}Window;

typedef struct Button{
   Drawable super;
   char text[256];
}Button;


Rect* objs[128];
int rectCnt = 0;

static void rectobjs_swap(Rect** arr,int i,int j){
   Rect* a=arr[i];
   Rect* b=arr[j];
   int z=a->zindex;
   a->zindex=b->zindex;
   b->zindex=z;
   arr[i]=arr[j];
   arr[j]=a;
}

static void setOnClick(Rect* r,void* fnc){
   r->onClick = (int(*)(Rect*,int,int,int,int))fnc;
}

static void window_draw(Window* w){
   Rect* r=(Rect*)w;
   Drawable* d=(Drawable*)w;
   fillrect(r->x,r->y,r->w,r->h,d->bgcolor);
   //border
L_showborder:
   {
      int i;
      for(i=0;i<w->border; ++i){
        drawrect(r->x+i,r->y+i,r->w-i*2,r->h-i*2,w->borderclr);//border
      }
      if(r->resizable && (r->event & WIN_RESIZING) ){
         drawrect(r->x,r->y,r->PrevX,r->PrevY,~w->borderclr);
         drawrect(r->x+1,r->y+1,r->PrevX-1,r->PrevY-1,~w->borderclr);
      }
   }
L_showtitle:
   {
      fillrect(r->x,r->y,r->w,w->titleWidth,w->titlebgclr); 
      drawtext(w->title,r->x,r->y,w->titlefrclr);
   }
L_showcontrol:
   {
      fillrect(r->x+r->w-w->controlSize-1,r->y,w->controlSize,w->controlSize,0xaa0000);
      drawrect(r->x+r->w-w->controlSize-1,r->y,w->controlSize,w->controlSize,0xffffff);
      drawtext("X",r->x+r->w-w->controlSize/2-1,r->y,0xffffff);
   }

}

static void window_destroy(Window* w){
   Rect* r=(Rect*)w;
   if(r->zindex != rectCnt-1){
      rectobjs_swap(objs,rectCnt-1,r->zindex);
   }
   free(w);
   --rectCnt;
}
static int window_click(Window* w,int x,int y,int on,int btn){
   Rect* r=(Rect*)w;
   if(r->zindex != rectCnt-1){
      rectobjs_swap(objs,rectCnt-1,r->zindex);
   }
   if(r->movable && !on) {
       if(r->event & WIN_DRAGGING){
          int origx=r->x;
          int origy=r->y;
          r->x+=x-r->PrevX;
          r->y+=y-r->PrevY;
          r->PrevX=x;
          r->PrevY=y;
          if(r->Moved) r->Moved(r,origx,origy);
       }
       if(r->event & WIN_RESIZING){
          int origw=r->w;
          int origh=r->h;
          r->w=r->PrevX;
          r->h=r->PrevY;
          if(r->Resized) r->Resized(r,origw,origh);
       }
       r->event=0;
       return 1;
   }
L_ControlClickHandler:
   if(x >= 0 && x >= r->w-w->controlSize && x <= r->w && y>=0 && y <= w->controlSize){
       r->event = WIN_CLOSING;
       if(w->Closing) w->Closing(w);
       window_destroy(w);
       return 1;
   } 

   if(r->movable){
      if(x>=0 && x<=r->w && y>=0 && y<=w->titleWidth){
         r->event=WIN_DRAGGING;
         r->PrevX=x;
         r->PrevY=y;
         return 1;
      }
   }
   if(r->resizable){
      if(x>= r->w-w->border && x<=r->w){
         r->event=WIN_RESIZINGX;
      }
      if( (y>=r->h-w->border && y<=r->h)){
         r->event|=WIN_RESIZINGY;
      }
      if(r->event & WIN_RESIZING){
         r->PrevX=r->w;
         r->PrevY=r->h;
      }
      return 1;
   }

   return 0;
}
static int window_move(Window* w,int x,int y,int on){
   Rect* r=(Rect*)w;
   if(!on) r->event=0;
   if(r->movable && (r->event & WIN_DRAGGING) ){
         r->x+=x-r->PrevX;
         r->y+=y-r->PrevY;
         return 1;
   }
   if(r->resizable && (r->event & WIN_RESIZING) ){
      if(r->event & WIN_RESIZINGX) r->PrevX=x;
      if(r->event & WIN_RESIZINGY) r->PrevY=y;
      return 1;
   }
   return 0;
}
static Window* window_create(const char* title,int posx,int posy,int w,int h){
   Window* win = (Window*)malloc(sizeof(Window));
   Rect* r = (Rect*)win;
   Drawable* d=(Drawable*)win;
   memset(win,0,sizeof(Window));
   r->onDraw = (void(*)(Rect*))(window_draw);
   r->onClick = (int(*)(Rect*,int,int,int,int))(window_click);
   r->onMove = (int(*)(Rect*,int,int,int))(window_move);
   if(w < iconsize) w = iconsize;
   if(h < iconsize) h = iconsize;
   if(posx < 0) posx = rand() % SCREENX;
   if(posy < 0) posy = rand() % SCREENY;
   r->w = w;
   r->h = h;
   r->x = posx;
   r->y = posy;
   r->movable=1;
   r->resizable=1;
   strncpy(win->title,title,256);
   win->iconsize=iconsize;
   win->controlSize=iconsize;
   win->border=5;
   d->bgcolor=0xffffff;
   d->frcolor=0x000000;
   win->titleWidth=iconsize;
   win->titlebgclr=0x0000dd;
   win->titlefrclr=0xffffff;
   win->borderclr=0x1a1a1a;
   r->zindex=rectCnt;
   objs[rectCnt] = (Rect*)win;
   ++rectCnt;
   return win;
}
static Window* window_create0(const char* title,int w,int h){
   return window_create(title,-1,-1,w,h);
}
static void button_ondraw(Button* btn){
   Rect* r = (Rect*)btn;
   Drawable* d = (Drawable*)btn;
   fillrect(r->x,r->y,r->w,r->h,d->bgcolor);
   drawtext(btn->text,r->x +3, r->y+3,d->frcolor);
}
static Button* button_create(const char* txt,int x,int y,int w,int h){
   Button* btn = (Button*)malloc(sizeof(Button));
   Drawable* d=(Drawable*)btn;
   Rect* r = (Rect*)btn;
   memset(btn,0,sizeof(btn));
   strncpy(btn->text,txt,256);
   r->x = x;
   r->y = y;
   r->w = w;
   r->h = h;
   d->frcolor=0x000000;
   d->bgcolor=0xcccccc;
   r->onDraw=(void(*)(Rect*))button_ondraw;
   r->zindex=rectCnt;
   objs[rectCnt] = (Rect*)btn;
   ++rectCnt;
   return btn; 
}

static void global_mouse_fnc(int x,int y,int on,int btn){
   int i;
   for(i=rectCnt-1; i>=0; --i){
      Rect* obj = objs[i];
      if(obj == NULL || obj->onClick==NULL) continue;
      if(obj->event&WIN_CLOSING) continue;
      if(obj->resizable && obj->event & WIN_RESIZING){
         if(obj->onClick(obj,x-obj->x,y-obj->y,on,btn)) break;
      }
      else{
         if(x>=obj->x && x<=obj->x+obj->w && y>=obj->y && y <=obj->y+obj->h)
            if(obj->onClick(obj,x-obj->x,y-obj->y,on,btn)) break;
      }
   }
}

static void global_mouse_mv_fnc(int x,int y,int on){
   int i;
   for(i=rectCnt-1; i>=0; --i){
      Rect* obj = objs[i];
      if(obj == NULL || obj->onMove==NULL) continue;
      if(obj->event&WIN_CLOSING) continue;
      if(obj->movable && (obj->event & (WIN_DRAGGING|WIN_RESIZING))) {
         if(obj->onMove(obj,x-obj->x,y-obj->y,on)) break;
      }
   }
}

static void global_ondraw(){
   int i;
   fillrect(0,0,SCREENX,SCREENY,0x0000ff);
   for(i=0; i<rectCnt; ++i){
      if(objs[i] != NULL && objs[i]->onDraw != NULL){
          if(objs[i]->event & WIN_CLOSING) continue;
          objs[i]->onDraw(objs[i]);
      }
   }
L_drawMessage:
   flushscreen();
}

int onBtnClick(Button* obj,int x,int y,int on,int btn){
   if(on){
      Window* w = window_create("Demo",100,100,400,200);
      snprintf(w->title,256,"Demo-%d",w->super.super.zindex);
      w->super.bgcolor=rand();
   }
}

static void prepareWindow(){
   Button* b = button_create("Click Me",0,0,160,50);
   setOnClick((Rect*)b,&onBtnClick);
}

int main(){
   screen(SCREENX,SCREENY);
   screentitle("Window demo");
   setonmouse(global_mouse_fnc);
   setonmotion(global_mouse_mv_fnc);
   settextfont("./FreeMono.ttf",20);
   prepareWindow();
   while(1){
      global_ondraw();
      delay(1);
   }
}
