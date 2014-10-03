#include "sdlmm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
static void musicPlayer(void* p){
    short* wav;
    unsigned int len;
    loadwav("music.wav",&wav,&len);
    playwave(wav,len);
    free(wav);
}
int main(int argc, char *argv[]) {
    static const int width =250;
    static const int height = 75;
    int i;
    int* pic,pic_w,pic_h;
    screen(width,height);
    screentitle("Play Wave File");
    run_async(musicPlayer,NULL);
    while(1) {
        delay(1);
    }
    return 0;
}


