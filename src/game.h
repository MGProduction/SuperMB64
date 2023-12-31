//
// Copyright (c) 2023, Marco Giorgini [ @marcogiorgini ]
// Distributed under the MIT License
//
// some struct and base code for SuperMB64
//

// ************************************************************
// GAME GENERIC CODE
// ************************************************************

_img    canvas;

// ************************************************************

int     snd_coin,snd_jumpA,snd_jumpB,snd_stomp,snd_powerupappears,snd_powerup,snd_1up,snd_fireball,snd_kick,snd_breakblock,snd_pipe;

// ************************************************************

int     topscore;

// ************************************************************

// CURRENT status
#define status_normal      0
#define status_jump        1
#define status_dead        2
#define status_fall        4
#define status_automode  128 

#define autostatus_flagandleaving 1
#define autostatus_downthepipe    2
#define autostatus_upthepipe      3
#define autostatus_rightthepipe   4
#define autostatus_demoplay       5
#define autostatus_runright       6

// ************************************************************

int gui_drawdigits(int x,int y,int val,int digitcnt);
int gui_drawstring(int x,int y,const char*sz);

// ************************************************************

#define float_min(x,y) ((x<y)?x:y)
#define float_max(x,y) ((x>y)?x:y)
#define f2int(a) ((int)floor(a))

#define bitclear(what,mask) if(what&mask) what-=mask;

// ************************************************************

typedef struct{
 float x,y,w,h;
}_aabb;

float aabb_distance(_aabb*a,_aabb*b,float deltax,float deltay)
{
 return ((a->x+a->w/2+deltax)-(b->x+b->w/2))*((a->x+a->w/2+deltax)-(b->x+b->w/2))+((a->y+a->h/2+deltay)-(b->y+b->h/2))*((a->y+a->h/2+deltay)-(b->y+b->h/2));
}

int aabb_check(_aabb*a,_aabb*b,float deltax,float deltay)
{
 return ((a->x+deltax<b->x+b->w)&&(a->x+deltax+a->w>b->x)&&(a->y+deltay<b->y+b->h)&&(a->y+deltay+a->h>b->y));
}

int aabb_ispointin(_fpos*p,_aabb*b)
{
 return (p->x>=b->x)&&(p->x<=b->x+b->w)&&(p->y>=b->y)&&(p->y<=b->y+b->h);
}

int aabb_ispointinborder(_fpos*p,_aabb*b,float bw,float bh)
{
 return (p->x>=b->x-bw)&&(p->x<=b->x+b->w+bw)&&(p->y>=b->y-bh)&&(p->y<=b->y+b->h+bh);
}

// ************************************************************

typedef struct{
 float x1,y1,x2,y2;
 word  tx1,tx2,ty1,ty2;
 word  bkg;
}_fbox;

int fbox_ispointinborder(_fpos*p,_fbox*b,float bw,float bh)
{
 return (p->x>=b->x1-bw)&&(p->x<=b->x2+bw)&&(p->y>=b->y1-bh)&&(p->y<=b->y2+bh);
}

// ************************************************************

#pragma pack(1)
typedef struct{
 dword    id;
 word     from,to,loop;
}_animdesc;
typedef struct{
 word     speed;
 word     x,y,w,h;
}_framedesc;
typedef struct{
 _animdesc *anim; 
 word       animcount;
 _framedesc*frames;
 word    *  frameids;
 _img       atlas;
}_anim;
#pragma pack()
int anim_unload(_anim*a)
{
 img_delete(&a->atlas);
 return 1;
}
int anim_load(_anim*a,const char*name)
{
 if(img_load(&a->atlas,name))
  {
   char     nmanim[256];
   word    *heroanim;
   strcpy(nmanim,name);strcat(nmanim,".anim");
   heroanim=(word    *)res_get(nmanim,NULL);
   if(heroanim)
    {
     a->anim=(_animdesc*)&heroanim[3];
     a->animcount=heroanim[0];
     a->frameids=(word    *)&heroanim[heroanim[1]];
     a->frames=(_framedesc*)&heroanim[heroanim[2]];
     return 1;
    }
  }
 return 0;
}

// ************************************************************

#pragma pack(1)
typedef struct
{
 _strhash name;
 word     rmapw,rmaph;
 word     mapw,maph;
 word     tilew,tileh,tilecount;
 word     mappos;
}_tilemap;
typedef struct
{
 word      ntilemap;
 _tilemap *tilemap; 
 word     *maps;
}_tilemaps;
#pragma pack()
int tilemap_load(_tilemaps*a,const char*name)
{
 word    *base=(word    *)res_get(name,NULL);
 if(base)
  {
   a->ntilemap=base[0];
   a->tilemap=(_tilemap*)&base[1];
   a->maps=base;
   return 1;
  }
 return 0;
}

// ************************************************************

#define MAX_ACTORS  64

#define sprite_hflip     1
#define sprite_vflip     2
#define sprite_visible   4
#define sprite_activated 8
#define sprite_flashing  16
#define sprite_outlined  32
#define sprite_drawn     64
#define sprite_used      128

#define act_flashing     16

typedef struct __act _act;
typedef int (*_actplay)(_game*gm,_act*hero);

typedef struct __act{
 _fpos    pos;                   // world position
 _fpos    dpos;                  // movement delta
 float    zorder;                // drawn order (<0 before blocks >=0 after blocks)
 float    defspeed;
 
 _anim*   animset;               // active animset pointer
 dword    animid,prevanimid;     // active and last animid

 byte     flags,status,nextstatus,autostatus,kind;
 
 word     frame_cur,
          frame_from,frame_to,
          frame_speed,
          frame_time,
          frame_loop;

 word     timer; 
 _actplay play;
}_act;

_fpos   cam={0,0};
_act    actors[MAX_ACTORS];
_act*   pactors[MAX_ACTORS];
byte    actors_count;

void  actor_reset()
{
 memset(actors,0,sizeof(actors));actors_count=0;
}

_act  *actor_get()
{
 int i;
 for(i=0;i<MAX_ACTORS;i++)
  if((actors[i].flags&sprite_used)==0)
   {
     memset(&actors[i],0,sizeof(actors[i]));
     actors[i].flags|=sprite_used;
     pactors[actors_count++]=&actors[i];
     return &actors[i];
   }
 return NULL;
}

_framedesc*getframe(_act*a)
{
 return &a->animset->frames[a->animset->frameids[a->frame_cur]]; 
}

int act_setanim(_act*a,int animid)
{ 
 _anim*anim=a->animset;
 int   id;
 if(a->animid==animid)
  return 2;
 else
  for(id=0;id<anim->animcount;id++)
   if(anim->anim[id].id==animid)
    {
     _animdesc*ad=&anim->anim[id];
     a->animid=animid;
     a->frame_cur=a->frame_from=ad->from;
     a->frame_to=ad->to;
     a->frame_speed=anim->frames[anim->frameids[ad->from]].speed*GAME_FRAMERATE/1000;
     a->frame_time=0;
     a->frame_loop=ad->loop;      
     return 1;    
   }
 return 0;
}

#define col_yellow 0xFF00FFFF
void img_blit_outline(_img*idst,int px,int py,_img*i,int x,int y,int w,int h,int flip,dword outline)
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
     if(alpha==255)
      {
       int xxx=xx,next=1,prev=-1;
       canvas[(px+xx)+(py+yy)*ww]=val;
       if(flip&1) 
        {xxx=(w-xx-1);next=-1;prev=1;}
       if(img_get(i,xxx+x+prev,yy+y)==0)
        img_set(idst,px+xx-1,py+yy,outline);
       if(img_get(i,xxx+x+next,yy+y)==0)
        img_set(idst,px+xx+1,py+yy,outline);
       if(img_get(i,xxx+x,yy+y-1)==0)
        img_set(idst,px+xx,py+yy-1,outline);
       if(img_get(i,xxx+x,yy+y+1)==0)
        img_set(idst,px+xx,py+yy+1,outline);
      }
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

void act_draw(_game*gm,_act*a)
{
 _fpos p;
 p.x=a->pos.x-floor(cam.x);p.y=a->pos.y-floor(cam.y);
 if((p.x-8>GAME_WIDTH)||(p.x+8<0))
  {
   if(a->flags&sprite_drawn)
    a->flags-=sprite_drawn;
  }
 else
  if(a->flags&sprite_visible)
   {
    _framedesc*fr=getframe(a);
    float      fw=fr->w,fh=fr->h;
    a->flags|=sprite_drawn;
    if((a->flags&sprite_flashing)&&((gm->tick%7)<2))
     ;
    else
     {
      img_blit(&canvas,f2int(p.x-fw/2),f2int(p.y-fh),&a->animset->atlas,fr->x,fr->y,fr->w,fr->h,a->flags);
      if(a->flags&sprite_outlined)
       if((gm->tick%7)<2)
        img_blit_outline(&canvas,f2int(p.x-fw/2),f2int(p.y-fh),&a->animset->atlas,fr->x,fr->y,fr->w,fr->h,a->flags,col_yellow);
     }
   }

 if(a->animid)
  {
   a->frame_time++;
   if(a->frame_time>a->frame_speed)
    {
     a->frame_time=0;
     if(a->frame_cur+1<=a->frame_to)
      a->frame_cur++;
     else
      if(a->frame_loop)
       a->frame_cur=a->frame_from;
      else
       {
        a->prevanimid=a->animid;
        a->animid=0;
       }
    }
  }
}

int actors_ysort(const void*a,const void*b)
{
 _act**A=(_act**)a;
 _act**B=(_act**)b;
 float  dif=(*A)->pos.y-(*B)->pos.y; 
 if(dif)
  if(dif>0)
   return 1;
  else
   return -1;
 else
  return ((*A)-actors)-((*B)-actors);
}

int actors_zxsort(const void*a,const void*b)
{
 _act**A=(_act**)a;
 _act**B=(_act**)b;
 float  dif=(*A)->zorder-(*B)->zorder; 
 if(dif)
  if(dif>0)
   return 1;
  else
   return -1;
 else
  {
   dif=(*A)->pos.x-(*B)->pos.x; 
   if(dif)
    if(dif>0)
     return 1;
    else
     return -1;
   else
    return ((*A)-actors)-((*B)-actors);
  }
}

extern int anim_walk,anim_idle;
void act_getaabb(_act*c,_aabb*box)
{
 _framedesc*fr=getframe(c);
 float      w=fr->w,h=fr->h; 
 if((c->kind==1)&&((h==10)||(h==9))&&((c->animid==anim_walk)||(c->animid==anim_idle)))
  h=8;
 box->x=c->pos.x-w/2;box->w=w;
 box->y=c->pos.y-h;box->h=h;
}

int act_intersect(_act*a,_act*b)
{
 _aabb b1,b2;
 act_getaabb(a,&b1);
 act_getaabb(b,&b2);
 return aabb_check(&b1,&b2,0,0);
}


// ************************************************************
_tilemaps level;
_img      offscreen;

_act     *hero;

word      worldareascnt=0;
_fbox*    worldarea;
_fbox     worldareas[8]; 

_anim     charanim[24];
int       anim_idle,anim_walk,anim_shoot,anim_jump,anim_dress,anim_die,anim_backdie,anim_attack,anim_grow,anim_shrink,anim_fire,anim_climb,anim_shell;
int       charanim_cnt;
int       mario_anim,mariohi_anim,mariofire_anim,score_anim,fireflower_anim,brickpieces_anim,fireball_anim,fireballexplosion_anim;

// ************************************************************
int level_load(_game*gm,int world,int lv,int flags);

void camera_update(_game*gm);

void tilebackground_blit(_img*canvas,int bx,int by,int bw,int bh,_tilemaps*tm,_img*i,int cx,int cy);
void tilemap_blit(_img*canvas,int bx,int by,int bw,int bh,_tilemaps*tm,_img*i,int cx,int cy);

void input_clear(_game*gm)
{
 gm->input.key_left=gm->input.key_right=gm->input.key_up=gm->input.key_down=gm->input.key_control=false;
}

// ************************************************************

#if defined(_GIF_SUPPORT)
MsfGifState gifState;

void gif_start()
{
 memset(&gifState,0,sizeof(gifState));
 msf_gif_begin(&gifState, GAME_WIDTH,GAME_HEIGHT);
}

void gif_addframe(byte*pixelbuffer)
{
 if(gifState.width)
  msf_gif_frame(&gifState,pixelbuffer, 3, 16, GAME_WIDTH * 4); //frame 1
}

void gif_stop(const char*name)
{
 MsfGifResult result = msf_gif_end(&gifState);
 if (result.data)
  {
   FILE * fp = fopen(name, "wb");
   fwrite(result.data, result.dataSize, 1, fp);
   fclose(fp);
  }
 msf_gif_free(result);
}
#endif

// ************************************************************