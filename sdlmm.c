#if 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

#include "sdlmm.h"
#include <math.h>
#include "SDL.h"
#include "SDL_video.h"
#include "SDL_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifndef __func__
#  define __func__ __FUNCTION__
#endif
#ifndef Uint8
typedef unsigned char Uint8;
#endif
#ifndef Uint32
typedef unsigned Uint32;
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
static int drawthread_created = 0;
static SDL_Thread * drawthread;
static SDL_Thread * sndthread;

static SDL_Surface *drawscreen;
static int sdlcolor;
static SDL_mutex * drawqueue_mutex;
static SDL_cond *init_cond;
static int sdlalwaysflush = 0;
const static char* newTitle;
static int changeTitleReq = 0;

static void sdlmousefnc_default(int x,int y,int on,int btn) {

}
static void (*sdlkeyfnc)(int,int,int);
static void (*sdltouchfnc)(int,int,int);
static void (*sdlmousefnc)(int,int,int,int)=sdlmousefnc_default;
static void (*sdlmotionfnc)(int,int,int);

static void surface_to_array(SDL_Surface* img,int** ret,int*w, int* h) {
    int len,ih,iw;
    int* pret;
    unsigned maxRange;
    ih = img->h;
    iw = img->w;
    *w = iw;
    *h = ih;
    len = sizeof(int)*iw*ih;
    pret = (int*)malloc(len);
    if(!pret) {
        fprintf(stderr,"%s %d %s:(malloc failed) %s\n", __FILE__,__LINE__,__func__,strerror(errno));
        return;
    }
    {
        int i,j;
        int defaultPixelSize = 4;

        if(img->format->BytesPerPixel > 0) {
            defaultPixelSize = img->format->BytesPerPixel;
        }
        //printf("defaultPixelSize=%d\n",defaultPixelSize);
        for(j=0; j<ih; ++j) {
            for(i=0; i<iw; ++i) {
                Uint8 *target_pixel = ( (Uint8 *)img->pixels) + j * img->pitch + i * defaultPixelSize;
                Uint32 newPixel = *(Uint32*)target_pixel;
                Uint32 newPixel2;
                Uint8 r,g,b,a;
                /*
                r = (((newPixel & img->format->Rmask)>>img->format->Rshift)<<img->format->Rloss)  ;
                g = (((newPixel & img->format->Gmask)>>img->format->Gshift)<<img->format->Gloss)  ;
                b = (((newPixel & img->format->Bmask)>>img->format->Bshift)<<img->format->Bloss)  ;
                a = (((newPixel & img->format->Amask)>>img->format->Ashift)<<img->format->Aloss)  ;
                */
                r = (((newPixel & img->format->Rmask)>>img->format->Rshift)<<img->format->Rloss);
                g = (((newPixel & img->format->Gmask)>>img->format->Gshift)<<img->format->Gloss);
                b = (((newPixel & img->format->Bmask)>>img->format->Bshift)<<img->format->Bloss);
                a = (((newPixel & img->format->Amask)>>img->format->Ashift)<<img->format->Aloss)  ;

                newPixel2 = (a << 24)|(r<<16)|(g<<8)|(b);



                pret[j*iw+i] = newPixel2;//*(Uint32*)target_pixel;
            }
        }
    }
    *ret = pret;
}


void loadimage(const char* filename,int** ret,int* w,int *h) {
    SDL_Surface* img;
    int isBMP = 0;
    {
        char* s = strrchr(filename,'.');
        if(strcmp(s,".bmp")==0) {
            isBMP = 1;
        }
    }
    if(isBMP) {

        img = SDL_LoadBMP(filename);
        surface_to_array(img,ret,w,h);
    }
    else {
        img = IMG_Load(filename);
        if(!img) {
            fprintf(stderr,"%s %d %s:(IMG_Load Failed) %s\n", __FILE__,__LINE__,__func__,strerror(errno));
            return;
        }
        surface_to_array(img,ret,w,h);
    }
}

void setonkey(void(*fnc)(int key,int ctrl,int on)) {
    if(fnc) sdlkeyfnc=  fnc;
}

void setontouch(void (*fnc)(int x,int y,int on)) {
    if(fnc) sdltouchfnc = fnc;
}

void setonmotion(void (*fnc)(int x,int y,int on)) {
    if(fnc) sdlmotionfnc = fnc;
}

void setonmouse(void(*fnc)(int x,int y,int on,int btn)) {
    if(fnc) sdlmousefnc = fnc;
}

static void sdlset_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    Uint8* target_pixel;
    if(x >= surface->w || x < 0 ||y >=surface->h||y<0) return;
    target_pixel = ( (Uint8 *)surface->pixels) + y * surface->pitch + x * 4;
    *(Uint32 *)target_pixel = pixel;
}

static void sdlset_pixel_nocheck(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
    *(Uint32 *)target_pixel = pixel;
}
static void sdlset_pixel_nocheck_with_alpha(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
    Uint32 v = *(Uint32 *)target_pixel;
    Uint32 a2 = ((pixel >> 24) & 0xff);
    a2 = (a2) | (a2 << 8) | (a2 << 16) | (a2 << 24);
    Uint32 ua2 = ~a2;
    *(Uint32 *)target_pixel = (v&ua2)+(pixel&a2);
}

static void sdldraw_circle(SDL_Surface *surface, int n_cx, int n_cy, int radius, Uint32 pixel)
{
    // if the first pixel in the drawscreen is represented by (0,0) (which is in sdl)
    // remember that the beginning of the circle is not in the middle of the pixel
    // but to the left-top from it:

    double error = (double)-radius;
    double x = (double)radius -0.5;
    double y = (double)0.5;
    double cx = n_cx - 0.5;
    double cy = n_cy - 0.5;
    SDL_LockSurface(surface);

    while (x >= y)
    {
        sdlset_pixel(surface, (int)(cx + x), (int)(cy + y), pixel);
        sdlset_pixel(surface, (int)(cx + y), (int)(cy + x), pixel);

        if (x != 0)
        {
            sdlset_pixel(surface, (int)(cx - x), (int)(cy + y), pixel);
            sdlset_pixel(surface, (int)(cx + y), (int)(cy - x), pixel);
        }

        if (y != 0)
        {
            sdlset_pixel(surface, (int)(cx + x), (int)(cy - y), pixel);
            sdlset_pixel(surface, (int)(cx - y), (int)(cy + x), pixel);
        }

        if (x != 0 && y != 0)
        {
            sdlset_pixel(surface, (int)(cx - x), (int)(cy - y), pixel);
            sdlset_pixel(surface, (int)(cx - y), (int)(cy - x), pixel);
        }

        error += y;
        ++y;
        error += y;

        if (error >= 0)
        {
            --x;
            error -= x;
            error -= x;
        }
    }
    SDL_UnlockSurface(surface);

}
static void sdlfill_circle(SDL_Surface *surface, int cx, int cy, int radius, Uint32 pixel)
{
    static const int BPP = 4;

    double r = (double)radius;
    double dy;
    SDL_LockSurface(surface);

    for (dy = 1; dy <= r; dy += 1.0)
    {
        double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
        int x = cx - dx;
        Uint8 *target_pixel_a = (Uint8 *)surface->pixels + ((int)(cy + r - dy)) * surface->pitch + x * BPP;
        Uint8 *target_pixel_b = (Uint8 *)surface->pixels + ((int)(cy - r + dy)) * surface->pitch + x * BPP;
        for (; x <= cx + dx; x++)
        {
            sdlset_pixel(surface,x,cy+r-dy,pixel);
            sdlset_pixel(surface,x,cy-r+dy,pixel);
        }
    }
    SDL_UnlockSurface(surface);

}

static void sdldrawLine(SDL_Surface *Screen, int x0, int y0, int x1, int y1, Uint32 pixel,int lock) {

    int i;
    double x = x1 - x0;
    double y = y1 - y0;
    double length = sqrt( x*x + y*y );
    double addx = x / length;
    double addy = y / length;
    x = x0;
    y = y0;
    if(lock) SDL_LockSurface(Screen);
    for ( i = 0; i < length; i += 1) {
        sdlset_pixel(Screen, x, y, pixel );
        x += addx;
        y += addy;
    }
    if(lock)SDL_UnlockSurface(Screen);

}
static void sdldrawpixels(SDL_Surface *Screen,Uint32* pixels, int x, int y, int w, int h) {
    int i,j;
    int miny = y+h;
    int minx = x+w;
    if(miny > Screen->h) miny=Screen->h;
    if(minx > Screen->w) minx=Screen->w;
    if(x<0) x = 0;
    if(y<0) y = 0;
    SDL_LockSurface(Screen);
    for(j=y; j<miny; ++j) {
        for(i=x; i<minx; ++i) {
            sdlset_pixel_nocheck(Screen,i,j,*pixels);
            ++pixels;
        }
    }
    SDL_UnlockSurface(Screen);

}
static void sdlfillrect(SDL_Surface *Screen,int x, int y, int w, int h,Uint32 color) {

    int i,j;
    int miny = y+h;
    int minx = x+w;
    if(miny > Screen->h) miny=Screen->h;
    if(minx > Screen->w) minx=Screen->w;
    if(x<0) x = 0;
    if(y<0) y = 0;
    SDL_LockSurface(Screen);
    for(j=y; j<miny ; ++j) {
        for(i=x; i<minx; ++i) {
            sdlset_pixel_nocheck(Screen,i,j,color);
        }
    }
    SDL_UnlockSurface(Screen);

}

static void surface_to_array_text(SDL_Surface* img,int** ret,int*w, int* h) {
    int len,ih,iw;
    int* pret;
    ih = img->h;
    iw = img->w;
    *w = iw;
    *h = ih;
    len = sizeof(int)*iw*ih;
    pret = (int*)malloc(len);
    if(!pret) {
        fprintf(stderr,"%s %d %s:(malloc failed) %s\n", __FILE__,__LINE__,__func__,strerror(errno));
        return;
    }
    {
        int i,j;
        for(j=0; j<ih; ++j) {
            for(i=0; i<iw; ++i) {
                Uint8 *target_pixel = ( (Uint8 *)img->pixels) + j * img->pitch + i;
                Uint8 val = *target_pixel;
                pret[j*iw+i] = (val<<24)|(val<<16)|(val<<8)|val;
            }
        }
    }
    *ret = pret;
}


typedef struct SetTextFontReq {
    char* font;
    int fontsize;
} SetTextFontReq;
static SetTextFontReq setTextFontReq;
static int hasSetTextFontReq = 0;

static void sdldrawpixels_text_masked(SDL_Surface *Screen,Uint32* pixels, int x, int y, int w, int h,int foreground,int transkey) {
    int i,j;
    int miny = y+h;
    int minx = x+w;
    if(miny > Screen->h) miny=Screen->h;
    if(minx > Screen->w) minx=Screen->w;
    if(x<0) x = 0;
    if(y<0) y = 0;
    SDL_LockSurface(Screen);
    for(j=y; j<miny; ++j) {
        for(i=x; i<minx; ++i) {
            Uint32 pixel = *pixels;
            Uint32 alpha = pixel&0xff000000;
            if(pixel != transkey&&((alpha)!=0 )) {
                pixel = alpha|(foreground&0xffffff);
                sdlset_pixel_nocheck_with_alpha(Screen,i,j,pixel);
            }
            ++pixels;
        }
    }
    SDL_UnlockSurface(Screen);
}


void settextfont(const char* font,int fontsize) {
    SDL_mutexP(drawqueue_mutex);
    hasSetTextFontReq = 1;
    setTextFontReq.font = (char*)font;
    setTextFontReq.fontsize = fontsize;
    SDL_mutexV(drawqueue_mutex);
}
static FT_Library ft_library;
static FT_Face ft_face;

static void do_settextfont(const char* font,int fontsize) {

    FT_Face ft_newface;
    if(0 != FT_New_Face(ft_library, font, 0, &ft_newface)) {
        fprintf(stderr,"settextfont failed:FT_NewFace can not open font %s(fontsize=%d)\n",
                font,fontsize);
        return;
    }
    FT_Set_Pixel_Sizes(ft_newface, 0, fontsize);
    if(ft_face) {
        FT_Done_Face(ft_face);
    }
    ft_face = ft_newface;
}


static void sdl_setcolor(int color) {
    sdlcolor = color;
}

void sdl_main_run() {
    SDL_Event event;
    if(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
            SDL_mutexP( drawqueue_mutex);
            drawscreen = 0;
            SDL_mutexV(drawqueue_mutex);
            exit(0);
        }
        else if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            if(sdlkeyfnc) {
                sdlkeyfnc(event.key.keysym.sym,event.key.keysym.mod,event.type==SDL_KEYDOWN);
            }
        }
        else if(event.type == SDL_MOUSEBUTTONDOWN ||
                event.type == SDL_MOUSEBUTTONUP) {
            if(sdlmousefnc) {
                sdlmousefnc(event.button.x,event.button.y,event.button.state==SDL_PRESSED,event.button.button);
            }
            if(sdltouchfnc) {
                sdltouchfnc(event.button.x,event.button.y,event.button.state==SDL_PRESSED);
            }
        }
        else if(event.type == SDL_MOUSEMOTION) {
            if(sdlmotionfnc) {
                sdlmotionfnc(event.motion.x,event.motion.y,event.motion.state==SDL_PRESSED);
            }
        }
        SDL_mutexP(drawqueue_mutex);
        if(changeTitleReq != 0) {
            SDL_WM_SetCaption(newTitle,NULL);
            changeTitleReq = 0;
        }
        if(hasSetTextFontReq) {
            do_settextfont(setTextFontReq.font,setTextFontReq.fontsize);
            hasSetTextFontReq=  0;
        }
        SDL_mutexV(drawqueue_mutex);
        //SDL_CondSignal(init_cond);
    }
    SDL_Delay(1);
}

typedef struct ThreadParam {
    int width,height;
} ThreadParam;
static void ft_atexit() {
    FT_Done_FreeType(ft_library);
}

typedef struct AudioContext{
   short buf[65536];
   int audio_len;
   Uint8* audio_pos;
   Uint32 audio_offset;
   int isFull;
   int hasAudio;
}AudioContext;
static AudioContext audioContext;
static void fill_audio(void *udata, Uint8 *stream, int len){
          /* Only play if we have data left */
        if ( audioContext.audio_len == 0 )
            return;
        /* Mix as much data as possible */
        //SDL_mutexP(audioContext.mutex);
        len = ( len > audioContext.audio_len ? audioContext.audio_len : len );
        printf("fill_audio len=%d\n",len);

        SDL_MixAudio(stream, audioContext.audio_pos, len, SDL_MIX_MAXVOLUME);
        audioContext.audio_pos += len;
        audioContext.audio_len -= len;
        if(audioContext.audio_len == 0){
           audioContext.audio_offset = 0;
           audioContext.audio_pos = (Uint8*)&audioContext.buf[0];
        }
        if(audioContext.isFull) {
           audioContext.isFull = 0;
           //SDL_CondSignal(audioContext.cond);
        }
        //SDL_mutexV(audioContext.mutex);

}
typedef struct DOPlayWaveReq{
   short* wave;
   int len;
}DOPlayWaveReq;

static DOPlayWaveReq playwaveReq;
static void do_playwave();
void playwave(short* wave,int len){
   playwaveReq.wave = wave;
   playwaveReq.len = len;
   do_playwave();
}

static void do_playwave(){
   if(playwaveReq.len == 0) return;
   int byteLen = playwaveReq.len * sizeof(short);
   char* pwave = (char*)playwaveReq.wave;
   while(byteLen > 0){
       if(byteLen + audioContext.audio_offset >= 65535){
          int procLen = 65536-audioContext.audio_offset;
          memcpy(&audioContext.buf[audioContext.audio_offset],pwave,procLen);
          byteLen -= procLen;
          pwave += procLen;
          audioContext.audio_len = procLen;
          audioContext.isFull = 1;
          audioContext.audio_offset +=procLen;
          SDL_PauseAudio(0);
          while ( audioContext.audio_len > 0 ) {
             SDL_Delay(10);
          } 
       }
       else{
          int procLen = byteLen;
          memcpy(&audioContext.buf[audioContext.audio_offset],pwave,procLen);
          byteLen -= procLen;
          audioContext.audio_offset +=procLen;
          audioContext.audio_len = procLen;
          SDL_PauseAudio(0);
          break;
       }
   }
   while ( audioContext.audio_len > 0 ) {
      SDL_Delay(10);
   } 
   playwaveReq.len = 0;
}

static int init_audio(){
    SDL_AudioSpec wanted;
    /* Set the audio format */
    wanted.freq = 11025;
    wanted.format = AUDIO_S16;
    wanted.channels = 1;    /* 1 = mono, 2 = stereo */
    wanted.samples = 1024;  /* Good low-latency value for callback */
    wanted.callback = fill_audio;
    wanted.userdata = NULL;
    audioContext.audio_offset = 0;
    audioContext.audio_pos = (Uint8*)&audioContext.buf[0];
    /* Open the audio device, forcing the desired format */
    if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return(-1);
    }
   audioContext.hasAudio = 1;
    return(0);
}

static void audio_atexit(){
   audioContext.hasAudio = 0;
   audioContext.audio_pos = (Uint8*)&audioContext.buf[0];
   audioContext.audio_len = 0;
   SDL_CloseAudio();
}

static void init_manual( ThreadParam* param) {
    if(!drawscreen) {
        int initflag = IMG_INIT_JPG|IMG_INIT_PNG;
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            fprintf(stderr,"SDL_Init Failed @screen(%d,%d)\n",param->width,param->height);
            exit( 1 );
        }
        if ((IMG_Init(initflag) & initflag) != initflag) {
            fprintf(stderr,"IMG_INIT Init Failed @screen(%d,%d)\n",param->width,param->height);
            exit( 1 );
        }
        drawscreen = SDL_SetVideoMode(param->width,param->height,32,   SDL_DOUBLEBUF|SDL_ANYFORMAT);
        if (drawscreen == NULL) {
            exit(2);
        }
        FT_Init_FreeType(&ft_library);
        atexit(ft_atexit);
        atexit(SDL_Quit);

        SDL_mutexP(drawqueue_mutex);
        SDL_CondSignal(init_cond);
        SDL_mutexV(drawqueue_mutex);

    }
}
static int sdl_draw_thfnc(void* dummy) {
    ThreadParam* param = (ThreadParam*)dummy;
    init_manual(param);
    while(1) {
        sdl_main_run();
    }
    return 0;
}

void setalwaysflush(int doflush) {
    sdlalwaysflush = doflush;
}

void screen(int width,int height) {
    ThreadParam* p;
    p = (ThreadParam*)malloc(sizeof(ThreadParam));
    p->width = width;
    p->height = height;
    if(!drawthread_created) {
        drawqueue_mutex= SDL_CreateMutex();
        if(!drawqueue_mutex) {
            perror("create lock failed");
        }
        init_cond = SDL_CreateCond();
        if(!init_cond) {
            perror("create cond failed");
        }
        drawthread_created = 1;
        //init_manual(p);
        if((drawthread=SDL_CreateThread(sdl_draw_thfnc,p)) == NULL) {
            perror("create thread failed");
        }
        if(init_audio()==0){
           atexit(audio_atexit);
        }
 
        SDL_mutexP(drawqueue_mutex);
        SDL_CondWait(init_cond,drawqueue_mutex);
        SDL_mutexV(drawqueue_mutex);



    }
}

void drawpixel(int x,int y,int color) {
    SDL_mutexP(drawqueue_mutex);
    if(drawscreen) {
        sdlset_pixel(drawscreen,x,y,color);
    }
    SDL_mutexV(drawqueue_mutex);
}

void drawline(int x1,int y1,int x2,int y2,int color) {
    SDL_mutexP(drawqueue_mutex);
    if(drawscreen) {

        sdldrawLine(drawscreen, x1, y1, x2, y2, color,1);
        if(sdlalwaysflush) {
            SDL_Flip(drawscreen);
        }
    }
    SDL_mutexV(drawqueue_mutex);
}

void drawrect(int x,int y,int w,int h,int color) {
    SDL_mutexP(drawqueue_mutex);
    if(drawscreen) {
        SDL_LockSurface(drawscreen);
        sdldrawLine(drawscreen, x, y, x+w, y, color,0);
        sdldrawLine(drawscreen, x, y+h, x+w, y+h, color,0);
        sdldrawLine(drawscreen, x, y, x, y+h, color,0);
        sdldrawLine(drawscreen, x+w, y, x+w, y+h, color,0);
        SDL_UnlockSurface(drawscreen);
        if(sdlalwaysflush) {
            SDL_Flip(drawscreen);
        }
    }
    SDL_mutexV(drawqueue_mutex);
}

void fillrect(int x,int y,int w,int h,int color) {
    SDL_Rect rect;
    SDL_mutexP(drawqueue_mutex);
    if(drawscreen) {
        sdlfillrect(drawscreen, x, y, w, h, color);
        if(sdlalwaysflush) {
            SDL_Flip(drawscreen);
        }
    }
    SDL_mutexV(drawqueue_mutex);

}

void drawpixels(int* pixels,int x,int y,int w,int h) {
    SDL_mutexP(drawqueue_mutex);
    if(drawscreen) {
        sdldrawpixels(drawscreen,(Uint32*) pixels,x, y, w, h);
        if(sdlalwaysflush) {
            SDL_Flip(drawscreen);
        }
    }
    SDL_mutexV(drawqueue_mutex);
}


void fillcircle(int x,int y,double r,int color) {
    SDL_mutexP(drawqueue_mutex);
    if(drawscreen) {
        sdlfill_circle(drawscreen, x, y, r, color);
        if(sdlalwaysflush) {
            SDL_Flip(drawscreen);
        }
    }
    SDL_mutexV(drawqueue_mutex);


}

void drawcircle(int x,int y,double r,int color) {
    SDL_mutexP(drawqueue_mutex);
    if(drawscreen) {
        sdldraw_circle(drawscreen, x, y, r, color);
        if(sdlalwaysflush) {
            SDL_Flip(drawscreen);
        }
    }
    SDL_mutexV(drawqueue_mutex);

}


void flushscreen() {
    SDL_mutexP(drawqueue_mutex);
    if(drawscreen) {
        SDL_Flip(drawscreen);
    }
    SDL_mutexV(drawqueue_mutex);

}


void screentitle(const char* title) {
    SDL_mutexP(drawqueue_mutex);
    changeTitleReq = 1;
    newTitle = title;
    SDL_mutexV(drawqueue_mutex);
}

static SDL_Surface* CreateTextureFromFT_Bitmap(const FT_Bitmap* bitmap,
        int r,int g,int b)
{
    Uint32 rmask, gmask, bmask, amask;
    void *buffer;
    int pitch;
    int x,y;
    unsigned char *src_pixels;
    unsigned int *target_pixels;
    SDL_Surface* output;
    SDL_PixelFormat* pixel_format;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    output = SDL_CreateRGBSurface(SDL_SWSURFACE,bitmap->width,bitmap->rows,32,rmask,gmask,bmask,amask);
    SDL_LockSurface(output);
    src_pixels = bitmap->buffer;
    target_pixels = (unsigned int*)(output->pixels);
    pixel_format = output->format;
    for (y = 0; y < bitmap->rows; y++)
    {
        for (x = 0; x < bitmap->width; x++)
        {
            int index = (y * bitmap->width) + x;
            unsigned int alpha = src_pixels[index];
            unsigned int pixel_value = SDL_MapRGBA(pixel_format, r, g, b, alpha);
            target_pixels[index] = pixel_value;
        }
    }
    SDL_UnlockSurface(output);

    return output;
}

typedef struct FreeTypeResult {
    int* pixels;
    char ch;
    wchar_t wch;
    int fontsize;

    int w;
    int h;
    int offsetY;
    int offsetX;

} FreeTypeResult;

typedef struct FreeTypeResultMap {
    FreeTypeResult* map[65536];
    int cnt;
} FreeTypeResultMap;

static FreeTypeResultMap freeTypeResultMap;

static void freetype_result_swap(FreeTypeResultMap* map,int i,int j) {
    FreeTypeResult* tmp;
    if(i == j) return;
    tmp = map->map[ i ];
    map->map[ i ] = map->map[ j ];
    map->map[ j ] = tmp;
}

static FreeTypeResult* freetype_result_map_new(int c,int fontsize,int* arr,int w,int h,int offsetX,int offsetY,int wide) {
    FreeTypeResult* ret;
    ret = (FreeTypeResult*)malloc(sizeof(FreeTypeResult));
    if(!ret) {
        fprintf(stderr,"create freetyperesult failed at %s %d\n",__FILE__,__LINE__);
        return NULL;
    }
    ret->pixels = arr;
    if(wide) {
        ret->wch = c;
    }
    else {
        ret->ch = c;
    }
    ret->fontsize = fontsize;
    ret->w= w;
    ret->h= h;
    ret->offsetY = offsetY;
    ret->offsetX = offsetX;
    return ret;
}
static void freetype_result_map_replace(FreeTypeResult* res,char c,int fontsize,int* arr,int w,int h,int offsetX,int offsetY,int wide) {
    if(res->pixels) {
        free(res->pixels);
    }
    res->pixels = arr;
    if(wide) {
        res->wch = c;
    }
    else {
        res->ch = c;
    }
    res->fontsize = fontsize;
    res->w= w;
    res->h= h;
    res->offsetY = offsetY;
    res->offsetX = offsetX;
}

static int freetyperesult_match_char(const FreeTypeResult* result,int c,int fontsize,int wide) {
    if(wide) {
        return result != NULL && result->pixels != NULL && result->wch==c && result->fontsize == fontsize;
    }
    else {
        return result != NULL && result->pixels != NULL && result->ch==c && result->fontsize == fontsize;
    }
}

static FreeTypeResult* freetype_result_map_find_or_create(int c,int fontsize,const FT_Face* face,int wide) {
    int i;
    int* arr;
    int w,h;

    for(i=0; i<freeTypeResultMap.cnt; ++i) {
        FreeTypeResult* result = freeTypeResultMap.map[i];
        if(freetyperesult_match_char(result,c,fontsize,wide)) {
            if(i > 1) {
                freetype_result_swap(&freeTypeResultMap,1,i);
                freetype_result_swap(&freeTypeResultMap,0,1);
            }
            return result;
        }
    }
    // not found
    if(wide) {
        FT_UInt gindex = FT_Get_Char_Index(*face, c);
        FT_Load_Glyph(*face, gindex, FT_LOAD_RENDER);
    }
    else {
        FT_Load_Char(*face, c,FT_LOAD_RENDER);
    }
    SDL_Surface* glyph_texture = CreateTextureFromFT_Bitmap(&(*face)->glyph->bitmap, 0xff,0xff,0xff);
    surface_to_array(glyph_texture,&arr,&w,&h);
    SDL_FreeSurface(glyph_texture);
    if(freeTypeResultMap.cnt < 65535) {
        freeTypeResultMap.map[freeTypeResultMap.cnt] = freetype_result_map_new(c,fontsize,arr,w,h,((*face)->glyph->metrics.horiAdvance >> 6),((*face)->glyph->metrics.horiBearingY >> 6),wide);
        if(freeTypeResultMap.cnt > 1) {
            freetype_result_swap(&freeTypeResultMap,1,freeTypeResultMap.cnt);
            freetype_result_swap(&freeTypeResultMap,0,1);
        }
        else {
            freetype_result_swap(&freeTypeResultMap,0,1);
        }
        ++freeTypeResultMap.cnt;
    }
    else {
        freetype_result_map_replace(freeTypeResultMap.map[freeTypeResultMap.cnt-1],c,fontsize,arr,w,h,((*face)->glyph->metrics.horiAdvance >> 6),((*face)->glyph->metrics.horiBearingY >> 6),wide);
        freetype_result_swap(&freeTypeResultMap,1,freeTypeResultMap.cnt-1);
        freetype_result_swap(&freeTypeResultMap,0,1);
    }
    return freeTypeResultMap.map[0];
}
void drawtextw(const wchar_t* text,int x_start,int baseline,int rcolor)
{
    if(!ft_face) {
        return;
    }
    int x = x_start;
    unsigned int i;
    for (; text[i]!=L'\0'; i++)
    {
        int ftsize = (int)setTextFontReq.fontsize;
        FreeTypeResult* result = freetype_result_map_find_or_create(text[i],(int)ftsize,&ft_face,1);
        if(result == NULL) continue;
        sdldrawpixels_text_masked(drawscreen,(Uint32*)result->pixels,x,baseline+(ftsize - result->offsetY),result->w,result->h,rcolor,0);
        x += result->offsetX;
    }
}

void drawtext(const char* text,int x_start,int baseline,int rcolor)
{
    if(!ft_face) {
        return;
    }
    int x = x_start;
    unsigned int i;
    for (; text[i]!='\0'; i++)
    {
        int ftsize = (int)setTextFontReq.fontsize;
        FreeTypeResult* result = freetype_result_map_find_or_create(text[i],ftsize,&ft_face,0);
        if(result == NULL) continue;
        sdldrawpixels_text_masked(drawscreen,(Uint32*)result->pixels,x,baseline + (ftsize- result->offsetY),result->w,result->h,rcolor,0);
        x += result->offsetX;
    }
}
#endif