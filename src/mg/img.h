//
// Copyright (c) 2023, Marco Giorgini [ @marcogiorgini ]
// Distributed under the MIT License
//
// (very) basic img handling
//

#ifndef mg_img_h
#define mg_img_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
               int x;
               int y;
              } _ipos;
typedef struct{
               float x;
               float y;
              } _fpos;

typedef struct{
 char    attr;
 int     w,h;
 dword*  col;
}_img;

void  img_new(_img*i,int w,int h);
void  img_delete(_img*i);

int   img_load(_img*img,const char*name);

dword img_get(_img*i,int x,int y);
void  img_set(_img*i,int x,int y,dword col);

void  img_box(_img*idst,int x,int y,int w,int h,dword col);
void  img_blit(_img*idst,int px,int py,_img*i,int x,int y,int w,int h,int flip);

void efx_fade(_img*idst,int pos,int top,int efx);

#ifdef MGIMG_IMPLEMENTATION

#ifndef MGIMG_IMPLEMENTATION_ONCE
#define MGIMG_IMPLEMENTATION_ONCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void img_new(_img*i,int w,int h)
{
 memset(i,0,sizeof(_img));
 i->col=(dword*)calloc(w*h,sizeof(dword));
 i->w=w;i->h=h;
 i->attr=1;
}

int  img_load(_img*img,const char*name)
{ 
 dword  size;
 byte  *mem=res_get(name,&size);
 if(mem)
  {
   int       width, height;
   dword*    image;
   qoi_desc  desc;
   memset(img,0,sizeof(*img));         
   image = (dword*) qoi_decode(mem,size, &desc, 4 );
   width=desc.width;height=desc.height;
   if(image)
    {
     img->col=image;
     img->w=width;img->h=height;
     img->attr=0;
     return 1;
    }
  }
 return 0;
}
void img_delete(_img*i)
{
 if(i->attr&1)
  free(i->col); 
 i->col=NULL;i->w=i->h=0;
}

dword img_get(_img*i,int x,int y)
{
 if((x>=0)&&(x<i->w)&&(y>=0)&&(y<i->h))
  return i->col[x+y*i->w];
 else
  return 0;
}

void img_set(_img*i,int x,int y,dword col)
{
 if((x>=0)&&(x<i->w)&&(y>=0)&&(y<i->h))
  {
   byte alpha=(col&0xFF000000)>>24;
   if(alpha)
    if(alpha==255)
     i->col[x+y*i->w]=col;
    else
     {
      dword val2=i->col[x+y*i->w];
      byte  r=((col&0xFF)*alpha+(val2&0xFF)*(255-alpha))/255;
      byte  g=((((col&0xFF00)>>8)*alpha+((val2&0xFF00)>>8)*(255-alpha))/255);
      byte  b=((((col&0xFF0000)>>16)*alpha+((val2&0xFF0000)>>16)*(255-alpha))/255);
      i->col[x+y*i->w]=r|(g<<8)|(b<<16)|0xFF000000;        
     }
  }
}

void img_box(_img*idst,int x,int y,int w,int h,dword col)
{
 int   xx,yy,hh=idst->h,ww=idst->w;
 dword*canvas=idst->col;
 for(yy=0;yy<h;yy++)
  if(isbetween(y+yy,0,hh-1))
   for(xx=0;xx<w;xx++)
    if(isbetween(x+xx,0,ww-1))
    {
     byte alpha=(col&0xFF000000)>>24;
     if(alpha)
      if(alpha==255)
       canvas[(x+xx)+(y+yy)*ww]=col;
      else
       {
        dword val2=canvas[(x+xx)+(y+yy)*ww];
        byte  r=((col&0xFF)*alpha+(val2&0xFF)*(255-alpha))/255;
        byte  g=((((col&0xFF00)>>8)*alpha+((val2&0xFF00)>>8)*(255-alpha))/255);
        byte  b=((((col&0xFF0000)>>16)*alpha+((val2&0xFF0000)>>16)*(255-alpha))/255);
        canvas[(x+xx)+(y+yy)*ww]=r|(g<<8)|(b<<16)|0xFF000000;        
       }
    }
}

void efx_fade(_img*idst,int pos,int top,int efx)
{
 byte alpha=0;
 switch(efx)
 {
 case 1:
  alpha=(top-pos)*255/top;
  img_box(idst,0,0,idst->w,idst->h,0x000000|(alpha<<24));
  break;
 case -1:
  alpha=(pos)*255/top;
  img_box(idst,0,0,idst->w,idst->h,0x000000|(alpha<<24));
  break;
 }
}

void img_blit(_img*idst,int px,int py,_img*i,int x,int y,int w,int h,int flip)
{
 int   xx,yy,ww=idst->w,hh=idst->h;
 dword*canvas=idst->col;
 for(yy=0;yy<h;yy++)
  if(isbetween((py+yy),0,hh-1)&&isbetween((y+yy),0,i->h-1))
  for(xx=0;xx<w;xx++)
   if(isbetween((px+xx),0,ww-1)&&isbetween((x+xx),0,i->w-1))
    {     
     byte  alpha;
     dword val;
     if(flip&1)
      val=i->col[x+(w-xx-1)+(y+yy)*i->w];
     else
      val=i->col[x+xx+(y+yy)*i->w];
     alpha=(val&0xFF000000)>>24;
     if(alpha)
      if(alpha==255)
       canvas[(px+xx)+(py+yy)*ww]=val;
      else
       {
        dword val2=canvas[(px+xx)+(py+yy)*ww];
        byte  r=((val&0xFF)*alpha+(val2&0xFF)*(255-alpha))/255;
        byte  g=((((val&0xFF00)>>8)*alpha+((val2&0xFF00)>>8)*(255-alpha))/255);
        byte  b=((((val&0xFF0000)>>16)*alpha+((val2&0xFF0000)>>16)*(255-alpha))/255);
        canvas[(px+xx)+(py+yy)*ww]=r|(g<<8)|(b<<16)|0xFF000000;        
       }
    }
}

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif