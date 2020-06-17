#include "sdlmm.h"
#include <stdlib.h>
#define SIZE 100
#define SCREEN_X  1024
#define SCREEN_Y  600
#define MAXDATA 16384
static void init(int* data,int size)
{
    int i;
    for(i=0; i<size; ++i)
    {
       data[i] = rand()%MAXDATA;
    }
}
    
static void swap(int* a, int i,int j)
{
   int t = a[i];
   a[ i ] = a[ j ];
   a[ j ] = t;
}


static void showData1D(int* data,int size,int p1,int p2)
{
   int widthOfX = SCREEN_X / size;
   int i;
   fillrect(0,0,SCREEN_X,SCREEN_Y,0);
   for(i=0; i<size; ++i)
   {
       int x = widthOfX * i ; 
       float dataHeightRatio = ((float)data[ i ])/MAXDATA;
       int dataHeight = (int)(SCREEN_Y*dataHeightRatio);
       int y = SCREEN_Y-dataHeight;
       if(y < 0) y = 0;
       int color = 0x00ffff;
       if(i==p1)
       {
           color = 0xff0000;
       }
       else if(i == p2)
       {
           color = 0xffff00;
       }
       fillrect(x,y,widthOfX,dataHeight,color);
   }
   flushscreen();
   delay (1);
}

int main (int argc, char** argv)
{
   int* data = (int*)malloc(sizeof(int)*SIZE);
   int dimShow=1;
   int needCompress=0;
   int i,j;
   if(SIZE > SCREEN_X)
   {
       dimShow=2;
       if(SIZE > SCREEN_X*SCREEN_Y)
       {
           needCompress=1;
       }
   }
   init(data,SIZE);
   screen (SCREEN_X, SCREEN_Y);
   screentitle("Sort");
   
   for(i=0; i<SIZE-1; ++i)
   {
      for(j=0; j<SIZE-i-1; ++j)
      {
         if(data[j] > data[j+1]){
             showData1D(data,SIZE,j,j+1);
             swap(data,j,j+1);
         }
      }
   }
   while(1){
       delay(10);
   }
   free(data);
}


