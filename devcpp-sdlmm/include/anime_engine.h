#ifndef SDLMM_ANIME_ENGINE_INTERFACE_H
#define SDLMM_ANIME_ENGINE_INTERFACE_H
typedef struct AnimeEngine{
    void* (*anime_load)(const char* name,int fps);
    void  (*anime_show)(void* handle);
    void  (*anime_showxy)(void*,int x,int y);
    void  (*anime_advance)(void*);
    void  (*anime_unload)(void*);
}AnimeEngine;

void use_anime_engine(AnimeEngine* new_engine);
void* anime_load(const char* name,int fps);
void  anime_show(void* handle);
void  anime_showxy(void*,int x,int y);
void  anime_advance(void* handle);
void  anime_unload(void* handle);

#endif
