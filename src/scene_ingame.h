//
// Copyright (c) 2023, Marco Giorgini [ @marcogiorgini ]
// Distributed under the MIT License
//
// ingame code for SuperMB64
//

// ************************************************************
// INGAME CODE
// ************************************************************

#define tile_background_groundA 1
#define tile_background_groundB 2

#define tile_background_wallA   3
#define tile_background_wallB   4

#define tile_background_blockA  5
#define tile_background_blockB  6

#define tile_background_blockQ1 7
#define tile_background_blockQ2 8
#define tile_background_blockQ3 9

#define tile_background_pipeupTL 12
#define tile_background_pipeupTR 13
#define tile_background_pipeupML 16
#define tile_background_pipeupMR 17

#define tile_background_blockC1 24
#define tile_background_blockC2 25
#define tile_background_blockC3 26

#define tile_background_blockQH 10

#define tile_background_blockQX 11

#define tile_background_pipeA   12
#define tile_background_pipeZ   23

#define tile_background_coin    24

#define tile_background_pole    27

#define tile_solid_start        tile_background_groundA 
#define tile_solid_end          tile_background_pipeZ

// ************************************************************

#define player_mario        1

#define enemy_goomba        2
#define enemy_greentroopa   3
#define enemy_redtroopa     4

#define bonus_coin          5
#define bonus_magicmushroom 6
#define bonus_mushroom1up   7
#define bonus_star          8

#define castle_flag         9

#define worldarea_start     10
#define worldarea_end       11

#define worldlevel_exit     12

#define worldarea_enter     13
#define worldarea_exit      17

#define bonus_fireflower    9
#define bonus_fragments     10
#define bonus_fireball      12

#define forcedforward 1

#define status_magic       8
#define status_invincible 16
#define status_fire       32

// NEXT status
#define status_killed      1
#define status_autojump    2      
#define status_star        4
#define status_grow        8

#define status_mainmask 7

#define ground_friction   0.2f
#define air_friction      0.1f
#define hero_speed        0.25f
#define hero_maxspeed     2
#define hero_jumpspeed    1
#define hero_topjumpspeed 5
#define hero_maxfallspeed 6
#define gravityup         0.8f
#define gravityuppow      0.4f
#define gravitydown       1.2f
#define goomba_speed      0.5f

#define layer_map         0
#define layer_elements    1

// ************************************************************

#define MAXPATCH 1024

byte patchmapid[MAXPATCH];
word patchx[MAXPATCH],patchy[MAXPATCH];
word patchtile[MAXPATCH];
word ipatch;

void patch_reset()
{
 ipatch=0;
}

void patch_add(int mapid,int x,int y,word tile)
{
 if(ipatch==MAXPATCH)
  ;
 else
  {
   patchmapid[ipatch]=mapid;
   patchx[ipatch]=x;patchy[ipatch]=y;
   patchtile[ipatch]=tile;
   ipatch++;
  }
}

int  patch_get(int mapid,int x,int y,word*tile)
{
 int i=ipatch;
 while(i--)
  if((patchmapid[i]==mapid)&&(patchx[i]==x)&&(patchy[i]==y))
   {
    *tile=patchtile[i];
    return 1;
   }
 return 0;
}

int  tile_get(int mapid,int x,int y);
int  tile_set(int mapid,int x,int y);

void camera_update(_game*gm);

void hero_killed(_act*hero);
void hero_grow(_act*hero);
int  hero_play(_game*gm,_act*hero);
void hero_fire(_act*hero);
int  hero_onpipeup(_act*c);
void hero_setautopipemode(_act*hero,int automode);

void coin_take(int addscore);
int  coin_play(_game*gm,_act*hero);
int  score_play(_game*gm,_act*goomba);
void bonus_add(_game*gm,int x,int y,int tile);
void fragments_add(int x,int y);
void explosion_add(_act*goomba);

int  fireflower_play(_game*gm,_act*hero);

int  movingelements_play(_game*gm,_act*goomba);
void goomba_backdie(_act*goomba);



char      guimsg[32];
_anim     gui;
_actplay  charplay[]={hero_play,movingelements_play,movingelements_play,coin_play,movingelements_play,movingelements_play};
short     lives,coins,score,secs,rsecs;
short     wrld,lv;
short     changearea,changelevel,currentarea;
_act*     flag;

byte      blockQ[]={tile_background_blockQ1,tile_background_blockQ2,tile_background_blockQ3,tile_background_blockQ2};
byte      blockC[]={tile_background_blockC1,tile_background_blockC2,tile_background_blockC3,tile_background_blockC2};
short     blockhitTD,blockhitX=-1,blockhitY=-1,blockhitT=0,colX,colY,colKIND;
byte      blockdisplacement[]={1,2,3,3,2,1,0};

// ************************************************************

int level_loadarea(_game*gm,short newarea,int flags)
{
 _act bhero; 
 
 if(hero)
  memcpy(&bhero,hero,sizeof(bhero));
 else
  memset(&bhero,0,sizeof(bhero));

 flag=NULL;
 *guimsg=0;
 actor_reset(); 
 patch_reset();

 if(isbetween(newarea,0,worldareascnt-1))
  {
   word herokey=player_mario;
   currentarea=newarea;changelevel=changearea=-1;
   worldarea=&worldareas[currentarea];
   if(flags&4)
    herokey=worldarea_exit+currentarea;
   if(flags&(1|2))
    {
     int x,y,tw=level.tilemap[layer_elements].tilew,th=level.tilemap[layer_elements].tileh;
     for(x=worldarea->tx1;x<worldarea->tx2;x++)
      for(y=worldarea->ty1;y<worldarea->ty2;y++)   
       {
        word     tile=tile_get(layer_elements,x,y);
        if(tile)
         {
          int px=x*tw;
          int py=y*th;
          if(tile==herokey)
           {
            if(flags&1)
             {
              hero=actor_get();                  
              if(bhero.play&&((bhero.status&status_dead)==0))
               memcpy(hero,&bhero,sizeof(bhero));
              else
               {
                hero->kind=(byte)tile;             
                hero->animset=&charanim[tile-1];
                hero->play=hero_play;
               }
              hero->zorder=1;
              bitclear(hero->flags,sprite_hflip)
              hero->flags|=(sprite_visible|sprite_used);
              act_setanim(hero,anim_idle);
              hero->dpos.x=hero->dpos.y=0;
              hero->pos.x=(float)(px+tw/2);
              hero->pos.y=(float)(py+th);
             }
           }
          else
           switch(tile)
            {
             case enemy_goomba:
             case enemy_greentroopa:
             case enemy_redtroopa:
              if(flags&2)
               {
                _act*tmp=actor_get();                    
                tmp->kind=(byte)tile;
                tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);
                tmp->animset=&charanim[tile-1];tmp->flags|=sprite_hflip;
                tmp->defspeed=goomba_speed;
                tmp->play=movingelements_play;tmp->flags|=sprite_visible;
                act_setanim(tmp,anim_walk);
               }
              break;
              case castle_flag:
               {
                flag=actor_get();                    
                flag->kind=(byte)tile;
                flag->pos.x=(float)(px+tw/2);flag->pos.y=(float)(py+th);
                flag->animset=&charanim[tile-1];
                flag->flags|=sprite_visible;
                act_setanim(flag,anim_idle);
               }
              break;
            }
         }
       }   
    }
   if(hero)
    camera_update(gm);

   if(hero_onpipeup(hero))
    {
     hero->pos.x+=4;
     hero_setautopipemode(hero,autostatus_upthepipe);
    }
   else
    if((wrld==1)&&(lv==2)&&(currentarea==0))
     {hero->status|=status_automode;hero->autostatus=autostatus_runright;}
   
   gm->scene->status=scene_entering;
   gm->timer=0;gm->maxtimer=GAME_FRAMERATE;
   return 1;
  }
 return 0;
}

int level_load(_game*gm,int world,int lv,int flags)
{
 int  x,y,start,end; 
 word mapw,maph,tw,th;
 char tilemap[32];
 
 strcpy(tilemap,"world_1x1");
 tilemap[6]='0'+world;tilemap[8]='0'+lv; 
 if(!tilemap_load(&level,tilemap))
  return 0;
 strcat(tilemap,".a");
 if(!img_load(&offscreen,tilemap))
  return 0;
 
 mapw=level.tilemap[layer_elements].mapw;maph=level.tilemap[layer_elements].maph;
 tw=level.tilemap[layer_elements].tilew;th=level.tilemap[layer_elements].tilew;

 worldareascnt=0; 
 for(start=end=x=0;x<mapw;x++)
  {
   word tile=tile_get(layer_elements,x,0);
   if(tile==worldarea_start)
    start=x;
   else
    if(tile==worldarea_end)
    {
     for(y=1;y<maph;y++)
      if(tile_get(layer_elements,x,y)==worldarea_end)
       break;
     worldarea=&worldareas[worldareascnt++];
     
     worldarea->x1=(float)start*tw;
     worldarea->x2=(float)worldarea->x1+(x-start)*tw;
     worldarea->y1=0;
     worldarea->y2=(float)worldarea->y1+y*th+th;

     worldarea->bkg=x+1;

     worldarea->tx1=start;worldarea->tx2=x+1;
     worldarea->ty1=0;worldarea->ty2=y+1;

     end=x;
    }
  }

 input_clear(gm);

 level_loadarea(gm,0,flags);

 secs=300*GAME_FRAMERATE;
 rsecs=secs/GAME_FRAMERATE;
 
 return worldareascnt;
}

void level_start(_game*gm)
{ 
 level_load(gm,wrld,lv,1|2); 
}

// ************************************************************

int ingame_enter(_game*gm)
{ 

 lives=3;coins=0;score=0;
 wrld=1;lv=1;

 anim_load(&gui,"gui");

 charanim_cnt=0;

 mario_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"mario");
 anim_load(&charanim[charanim_cnt++],"goomba");
 anim_load(&charanim[charanim_cnt++],"greentroopa");
 anim_load(&charanim[charanim_cnt++],"redtroopa");
 anim_load(&charanim[charanim_cnt++],"coin");
 anim_load(&charanim[charanim_cnt++],"magicmushroom");
 anim_load(&charanim[charanim_cnt++],"mushroom1up");
 anim_load(&charanim[charanim_cnt++],"star");
 anim_load(&charanim[charanim_cnt++],"poleflag");
 fireflower_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"fireflower");

 brickpieces_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"brickfragments");
 fireball_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"fireball"); 
 fireballexplosion_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"fireballhit"); 

 score_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"score");

 mariohi_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"mariohi");
 mariofire_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"mariofire");

 hero=NULL;

 level_start(gm);
 
 return 1;
}

int ingame_leave(_game*gm,_scene*next)
{
 while(charanim_cnt--)
  anim_unload(&charanim[charanim_cnt]);

 anim_unload(&gui);
 
 img_delete(&offscreen); 
 return 1;
}

void background_blit(_img*canvas,int bx,int by,int bw,int bh)
{
 img_box(canvas,bx,by,bw,bh,0xfffc945c);
}

int tile_get(int mapid,int x,int y)
{
 word*map=&level.maps[level.tilemap[mapid].mappos];
 if(isbetween(x,0,level.tilemap[mapid].mapw-1)&&isbetween(y,0,level.tilemap[mapid].maph-1))
  {
   word tile;
   if(patch_get(mapid,x,y,&tile))
    return tile;
   else
    return map[x+y*level.tilemap[mapid].mapw];
  }
 else
  return 0;
}

int tile_set(int mapid,int x,int y,word ntile)
{
 word*map=&level.maps[level.tilemap[mapid].mappos];
 if(isbetween(x,0,level.tilemap[mapid].mapw-1)&&isbetween(y,0,level.tilemap[mapid].maph-1))
  {
   if(map[x+y*level.tilemap[mapid].mapw]!=ntile)
    patch_add(mapid,x,y,ntile);
   return 1;
  }
 else
  return 0;
}

void tilebackground_blit(_img*canvas,int bx,int by,int bw,int bh,_tilemaps*tm,_img*i,int cx,int cy)
{
 int      x,y,tx=0,ty,played=0;
 word     tw=tm->tilemap[layer_map].tilew,th=tm->tilemap[layer_map].tileh;
 for(y=0;y<tm->tilemap[layer_map].maph;y++)
  if(by+y*th-cy+th<0)
    ;
   else
   if(by+y*th-cy>canvas->h)
    break;
   else
    for(x=0;x<tm->tilemap[layer_map].mapw;x++)
     if(bx+x*tw-cx+tw<0)
      ;
     else
      if(bx+x*tw-cx>canvas->w)
       break;
      else
       {
        word tile=tile_get(layer_map,worldarea->bkg,y);
        if(tile)
         {
          ty=th*tile;
          img_blit(canvas,bx+x*tw-cx,by+y*th-cy,i,tx,ty,tw,th,0);
          played++;
         }
       }
}

void tilemap_blit(_img*canvas,int bx,int by,int bw,int bh,_tilemaps*tm,_img*i,int cx,int cy)
{
 int  x,y,tx=0,ty,played=0;
 word tw=tm->tilemap[layer_map].tilew,th=tm->tilemap[layer_map].tileh;
 for(y=0;y<tm->tilemap[layer_map].maph;y++)
  if(by+y*th-cy+th<0)
    ;
   else
   if(by+y*th-cy>canvas->h)
    break;
   else
    for(x=0;x<tm->tilemap[layer_map].mapw;x++)
     if(bx+x*tw-cx+tw<0)
      ;
     else
      if(bx+x*tw-cx>canvas->w)
       break;
      else
       {
        word tile=tile_get(layer_map,x,y);
        if(tile)
         {
          if(tile==tile_background_blockQ1)
           tile=blockQ[(secs/(GAME_FRAMERATE/2))%sizeof(blockQ)];
          else
          if(tile==tile_background_blockC1)
           tile=blockC[(secs/(GAME_FRAMERATE/2))%sizeof(blockC)];
          ty=th*tile;
          if((blockhitX==x)&&(blockhitY==y))
           {
            int displacement;
            blockhitTD=(blockhitT/(GAME_FRAMERATE/15));
            displacement=blockdisplacement[blockhitTD];
            ty=th*tile;
            img_blit(canvas,bx+x*tw-cx,by+y*th-cy-displacement,i,tx,ty,tw,th,0);
            blockhitT++;
            if((blockhitT/(GAME_FRAMERATE/15))>=sizeof(blockdisplacement))
             {
              word ltile=tile_get(layer_map,x,y);
              blockhitX=blockhitY=-1;blockhitT=0;
              if((ltile==tile_background_blockQ1)||(ltile==tile_background_blockQH))
               tile_set(layer_map,x,y,tile_background_blockQX);
             }
           }
          else
           img_blit(canvas,bx+x*tw-cx,by+y*th-cy,i,tx,ty,tw,th,0);
          played++;
         }
       }
}

void gui_draw(_game*gm)
{
 int y=1,x=1,w=6,digit=3;
 int marioid=2,coinid=0,xid=1;
 
 img_blit(&canvas,x,y,&gui.atlas,marioid*w,0,6,6,0);x+=6;
 img_blit(&canvas,x,y,&gui.atlas,xid*w,0,6,6,0);x+=4;
 gui_drawdigits(x,y,lives,1);

 y=7;x=2;
 img_blit(&canvas,x,y,&gui.atlas,coinid*w,0,6,6,0);x+=4;x++;
 img_blit(&canvas,x,y,&gui.atlas,xid*w,0,6,6,0);x+=4;
 gui_drawdigits(x,y,coins,2);

 y=1;x=(GAME_WIDTH-6*4)/2+2;
 gui_drawdigits(x,y,score,6);

 y=1;x=GAME_WIDTH-6*2-2;
 gui_drawstring(x,y,"TIME");
 y=7;x=GAME_WIDTH-4*3-1;
 gui_drawdigits(x,y,rsecs,3);

}

void canvas_update(_game*gm)
{
 int     i;

 qsort(&pactors[0],actors_count,sizeof(pactors[0]),actors_zxsort);

 // draw background
 tilebackground_blit(&canvas,0,0,GAME_WIDTH,GAME_HEIGHT,&level,&offscreen,f2int(cam.x),f2int(cam.y));

 // draw behind blocks elements
 for(i=0;i<actors_count;i++)
  if(pactors[i]->zorder<0)
   act_draw(gm,pactors[i]);

 // draw blocks elements
 tilemap_blit(&canvas,0,0,GAME_WIDTH,GAME_HEIGHT,&level,&offscreen,f2int(cam.x),f2int(cam.y));
 
 // draw front elements
 for(i=0;i<actors_count;i++)
  if(pactors[i]->zorder>=0)
   act_draw(gm,pactors[i]);

 // draw gui
 gui_draw(gm);

 if(gm->scene->status==scene_playing)
  if(secs>0)
   {
    secs--;
    rsecs=secs/GAME_FRAMERATE;
   }

 if((gm->scene->status==scene_playing)||(gm->scene->status==scene_levelcompleted))
 {  
  // draw gui message
  if(*guimsg)
   gui_drawstring(-1,-1,guimsg);
 }

}

void camera_update(_game*gm)
{
 _framedesc*fr=getframe(hero);
 float      w=fr->w/2.0f;
 float      h=fr->h;
 
 if((hero->pos.x+w)-cam.x>GAME_WIDTH*3/5)
  cam.x=(hero->pos.x+w)-GAME_WIDTH*3/5;
 if((hero->pos.x-w)-cam.x<GAME_WIDTH*2/5)
  cam.x=(hero->pos.x-w)-GAME_WIDTH*2/5;

 if(cam.x<worldarea->x1) 
  cam.x=worldarea->x1;
 else
  if(cam.x+GAME_WIDTH>worldarea->x2)
   cam.x=(float)(worldarea->x2-GAME_WIDTH);

 if(hero->pos.y-cam.y>GAME_HEIGHT*3/5)
  cam.y=hero->pos.y-GAME_HEIGHT*3/5;
 if((hero->pos.y-h)-cam.y<GAME_HEIGHT*2/5)
  cam.y=(hero->pos.y-h)-GAME_HEIGHT*2/5;

 if(cam.y<worldarea->y1) 
  cam.y=worldarea->y1;
 else
  if(cam.y+GAME_HEIGHT>worldarea->y2)
   cam.y=(float)(worldarea->y2-GAME_HEIGHT);

 if(forcedforward)
  worldarea->x1=float_max(worldarea->x1,cam.x-GAME_WIDTH);
}

// ************************************************************

int aabb_intersect(_aabb*a,_aabb*b,_fpos*delta)
{
 if(aabb_check(a,b,delta->x,delta->y)) 
  {
   int    mask=0;
   if((delta->x!=0)&&aabb_check(a,b,delta->x,0))
    mask|=1;
   if((delta->y!=0)&&aabb_check(a,b,0,delta->y))
    mask|=2;
   if(mask==0)
    if(fabs(delta->x)>fabs(delta->y))
     mask|=1;
    else
     mask|=2;
   if(mask&1)
    if(delta->x>0)
     {
      if(a->x+a->w+delta->x>b->x) 
       {
        float dx=b->x-(a->x+a->w);
        if(dx<0)
         ;
        else
         delta->x=dx;
       }
     }
    else
    if(delta->x<0)
     {
      if(a->x+delta->x<b->x+b->w) 
       {
        float dx=(b->x+b->w)-a->x;
        if(dx>0)
         ;
        else
         delta->x=dx;
       }
     }
   if(mask&2)
    if(delta->y>0)
     {
      if(a->y+a->h+delta->y>b->y) 
       {
        float dy=b->y-(a->y+a->h);
        delta->y=dy;
       }
     }
    else
    if(delta->y<0)
     {
      if(a->y+delta->y<b->y+b->h) 
       {
        float dy=(b->y+b->h)-a->y;
        if(dy>0)
         ;
        else
         delta->y=dy;
       }
     }
   return 1;
  }
 else
  return 0;
}

int handle_aabbcollisioncore(_aabb*box,_fpos*delta,int solid,int simple)
{
 int   txx,tyy,x,y,tw=level.tilemap[layer_map].tilew,th=level.tilemap[layer_map].tileh,cnt=0,ay,by,ax,bx,lx,ly,stop=0;
 _aabb tile[8];
 _ipos ptile[8];
 word  wtile[8];
 txx=f2int((box->x+box->w/2)/tw);
 tyy=f2int((box->y+box->h/2)/th);
 lx=(int)float_max(1,box->w/tw)+1;ly=(int)float_max(1,box->h/th)+1;
 if(delta->x>0) {ax=0;bx=lx;}
 else
 if(delta->x<0) {ax=-lx;bx=0;}
 else           {ax=-1;bx=1;}
 if(delta->y>0) {ay=0;by=ly;}
 else
 if(delta->y<0) {ay=-ly;by=0;}
 else           {ay=-1;by=1;}
 for(y=ay;(stop==0)&&(y<=by);y++)
  if(isbetween(tyy+y,0,level.tilemap[layer_map].maph-1))
   for(x=ax;(stop==0)&&(x<=bx);x++)
    if(isbetween(txx+x,0,level.tilemap[layer_map].mapw-1))
     {
      word what=tile_get(layer_map,(txx+x),(tyy+y));  
      if(what)
       if(solid==0)
        {
         if(isbetween(what,tile_background_blockC1,tile_background_blockC3))
          {
           tile[cnt].x=(float)(txx+x)*tw;tile[cnt].y=(float)(tyy+y)*th;
           tile[cnt].w=(float)tw;tile[cnt].h=(float)th;
           if(aabb_check(box,&tile[cnt],delta->x,delta->y))
            {
             coin_take(1);
             tile_set(layer_map,(txx+x),(tyy+y),0);      
            }
          }
        }
       else
        if(isbetween(what,tile_solid_start,tile_solid_end))
         {         
          tile[cnt].x=(float)(txx+x)*tw;tile[cnt].y=(float)(tyy+y)*th;
          tile[cnt].w=(float)tw;tile[cnt].h=(float)th;
          if(aabb_check(box,&tile[cnt],delta->x,delta->y))
           {
            ptile[cnt].x=(word)txx+x;
            ptile[cnt].y=(word)tyy+y;
            wtile[cnt]=what;
            if(cnt&&(ptile[cnt].x==ptile[cnt-1].x)&&(ptile[cnt].y==ptile[cnt-1].y+1))
             tile[cnt-1].h+=tile[cnt].h;
            else
             cnt++;
            if(simple||(cnt>4))
             stop=1;
           }
         }
     }
 if(cnt)
  if(simple)
   ;
  else
   {
    int   n,rcnt=0;
    if(cnt>1)
     {
      n=0;
      while(n<cnt-1)
       {
        float distA=aabb_distance(box,&tile[n],delta->x,delta->y);
        float distB=aabb_distance(box,&tile[n+1],delta->x,delta->y);
        if(distA>distB)
         {
          _aabb t=tile[n];
          _ipos tp=ptile[n];
          word  tw=wtile[n];
          tile[n]=tile[n+1];
          tile[n+1]=t;
          ptile[n]=ptile[n+1];
          ptile[n+1]=tp;
          wtile[n]=wtile[n+1];
          wtile[n+1]=tw;
          if(n)
           n--;
         }
        else
         n++;
       }
     }
    colX=-1;
    for(n=0;n<cnt;n++)
     if(aabb_intersect(box,&tile[n],delta))
      {
       rcnt++;
       if(colX==-1)
        {
         colX=ptile[n].x;
         colY=ptile[n].y;
         colKIND=wtile[n];
        }
      }
    if(rcnt>1)
     rcnt=0;
   }
 return cnt;
}

void act_getmappos(_act*c,_fpos*delta,int*tx,int*ty)
{
 _framedesc*fr=getframe(c);
 float      w=fr->w,h=fr->h;
 _aabb      box;
 box.x=c->pos.x-w/2;box.w=w;
 box.y=c->pos.y-h;box.h=h;
 if(delta) {box.x+=delta->x;box.y+=delta->y;}
 if(tx) *tx=f2int((box.x+box.w/2)/level.tilemap[layer_map].tilew);
 if(ty) *ty=f2int((box.y+box.h/2)/level.tilemap[layer_map].tileh);  
}

void coin_take(int addscore)
{
 coins++;
 if(addscore) score+=200;
}

int coin_collect(_act*c,_fpos*delta)
{
 _aabb box; 
 act_getaabb(c,&box); 
 if(handle_aabbcollisioncore(&box,delta,0,1))
  return 1;
 return 0;
}

int handle_collisions(_act*c,_fpos*delta)
{
 _aabb box; 
 act_getaabb(c,&box); 
 if(handle_aabbcollisioncore(&box,delta,1,0))
  return 1;
 return 0;
}

int hero_onpipeup(_act*c)
{
 int        tx,ty;
 word       tile;
 act_getmappos(c,NULL,&tx,&ty);
 tile=tile_get(layer_map,tx,ty);
 if(isbetween(tile,tile_background_pipeupTL,tile_background_pipeupMR))
  return 1;
 return 0;
}

int act_ondownsecretpssage(_act*c,short*newarea)
{
 int        tx,ty;
 word       tile;
 act_getmappos(c,NULL,&tx,&ty);
 tile=tile_get(layer_elements,tx,ty+1);
 if(isbetween(tile,worldarea_enter,worldarea_enter+3))
  {
   if(newarea) *newarea=tile-worldarea_enter;
   return 1;
  }
 else
  return 0;
}

int act_onleftrightsecretpssage(_act*c,short*newarea)
{
 if((c->dpos.x)&&(c->dpos.y==0))
  {
   int        tx,ty;
   word       tile;
   act_getmappos(c,NULL,&tx,&ty);
   if(c->dpos.x>0)
    tile=tile_get(layer_elements,tx+1,ty);
   else
    if(c->dpos.x<0)
     tile=tile_get(layer_elements,tx-1,ty);
   if(isbetween(tile,worldarea_enter,worldarea_exit-1))
    {
     if(newarea) *newarea=tile-worldarea_enter;
     return 1;
    }
  }
 return 0;
}

int act_onleftpole(_act*c)
{
 if(c->dpos.x>0)
  {
   int        tx,ty;
   word       tile;
   act_getmappos(c,NULL,&tx,&ty);
   tile=tile_get(layer_map,tx,ty);
   if(tile==tile_background_pole)
    return 1;
  }
 return 0;
}

int act_onlevelexit(_act*c)
{
 if(c->dpos.x>0)
  {
   int        tx,ty;
   word       tile;
   act_getmappos(c,NULL,&tx,&ty);
   tile=tile_get(layer_elements,tx,ty);
   if(tile==worldlevel_exit)
    return 1;
  }
 return 0;
}

int act_onhitblock(_act*c)
{
 int        tx,ty;
 act_getmappos(c,NULL,&tx,&ty);
 if((tx==blockhitX)&&(ty+1==blockhitY)&&(blockhitTD<3))
  return 1;
 else
  return 0;
}

int act_ontheground(_act*c)
{
 _fpos      delta={0,1};
 _framedesc*fr=getframe(c);
 float      w=fr->w,h=fr->h;
 _aabb      box;
 box.x=c->pos.x-w/2;box.w=w;
 box.y=c->pos.y-h;box.h=h;
 if(handle_aabbcollisioncore(&box,&delta,1,1))
  return 1;
 return 0;
}

// ************************************************************

int wait_play(_game*gm,_act*goomba)
{
 if(!fbox_ispointinborder(&goomba->pos,worldarea,(float)(GAME_WIDTH*2),(float)16))
  return 0;
 if(goomba->animid==0)
  return 0;
 else
  return 1;
}

int movingelements_rise(_act*goomba)
{
 if(goomba->zorder<0)
  if(goomba->timer)
   {
    goomba->timer--;
    goomba->pos.y--;
    return 1;
   }
  else
   goomba->zorder=0;
 return 0;
}

void explosion_add(_act*goomba)
{
 _act*tmp=actor_get();    
 int  tile=fireballexplosion_anim+1;
 tmp->pos.x=goomba->pos.x;tmp->pos.y=goomba->pos.y;
 tmp->play=wait_play;
 tmp->animset=&charanim[tile-1];
 tmp->kind=tile;tmp->flags|=sprite_visible;
 act_setanim(tmp,anim_idle);   
}

void fragments_add(int x,int y)
{
 _act*tmp=actor_get();    
 int  id=1,tw=level.tilemap[id].tilew,th=level.tilemap[id].tileh,tile=brickpieces_anim+1;
 int  px=x*tw,py=(y-1)*th;
 tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);  
 tmp->defspeed=goomba_speed;
 tmp->play=movingelements_play;
 tmp->dpos.y=-6; 
 tmp->animset=&charanim[tile-1];
 tmp->kind=tile;tmp->flags|=sprite_visible;
 tmp->status|=status_fall;
 act_setanim(tmp,anim_idle);   

 tmp=actor_get();
 tmp->flags|=sprite_hflip;
 tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);  
 tmp->defspeed=goomba_speed;
 tmp->play=movingelements_play;
 tmp->dpos.y=-6;
 tmp->animset=&charanim[tile-1];
 tmp->kind=tile;tmp->flags|=sprite_visible;
 tmp->status|=status_fall;
 act_setanim(tmp,anim_idle);   
}

void bonus_add(_game*gm,int x,int y,int tile)
{
 if(tile)
  {
   _act*tmp=actor_get();    
   int  tw=level.tilemap[layer_map].tilew,th=level.tilemap[layer_map].tileh;
   int  px=x*tw,py=(y-1)*th;
   tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);   
   if(tile==bonus_coin)
    {
     px=x*tw,py=(y-1)*th;
     tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);   
     tmp->play=coin_play;
     tmp->dpos.y=-6;
     play_sound(gm,snd_coin,1.0f);
    }
   else
    {
     px=x*tw,py=y*th;
     tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);    
     tmp->zorder=-1;
     tmp->defspeed=goomba_speed;
     tmp->play=movingelements_play;tmp->timer=th;
     if(tile==bonus_magicmushroom)
      {       
       if(hero->status&status_magic)
        {
         tile=fireflower_anim+1;
         tmp->play=fireflower_play;
        }
      }
     else
     if(tile==bonus_mushroom1up)
      ;      
     else
      if(tile==bonus_star)
       {
        tmp->defspeed=goomba_speed*2;
        tmp->dpos.y=-6;
       }
    }
   tmp->animset=&charanim[tile-1];
   tmp->kind=tile;tmp->flags|=sprite_visible;
   act_setanim(tmp,anim_idle);   
  }
}

void score_add(float x,float y,int value)
{
 _act*tmp=actor_get();    
 tmp->pos.x=x;tmp->pos.y=y;
 tmp->animset=&charanim[score_anim];
 tmp->play=score_play;tmp->flags|=sprite_visible;score+=value;
 tmp->dpos.y=-6;
 act_setanim(tmp,tmp->animset->anim[(value/100)-1].id);
}

int fireflower_play(_game*gm,_act*goomba)
{
 if(!fbox_ispointinborder(&goomba->pos,worldarea,(float)(GAME_WIDTH*2),(float)16))
  return 0;
 if(movingelements_rise(goomba))
  return 1;
 if((fabs(goomba->pos.x-hero->pos.x)<10)&&(fabs(goomba->pos.y-hero->pos.y)<10))
  if(act_intersect(goomba,hero))
   {
    if((hero->status&status_magic)==0)
     hero_grow(hero);     
    else
     {
      hero->animset=&charanim[mariofire_anim];
      hero->status|=status_fire;
     }
    return 0;
   }
 return 1;
}

int coin_play(_game*gm,_act*hero)
{
 float gravity;
 if(hero->dpos.y<0)
  gravity=gravityup;
 else
  gravity=gravitydown/2;
 hero->dpos.y=float_min(hero->dpos.y+gravity,hero_maxfallspeed);
 hero->pos.y+=hero->dpos.y;
 if(hero->dpos.y>2)
  {
   coin_take(0);
   score_add(hero->pos.x,hero->pos.y,200);
   return 0;
  }
 else
  return 1;
}

int score_play(_game*gm,_act*hero)
{
 hero->timer++;
 hero->dpos.y=-0.5f;
 hero->pos.y+=hero->dpos.y;
 if(hero->timer>20)
  return 0;
 else
  return 1;
}

void goomba_backdie(_act*goomba)
{
 goomba->dpos.y=-6; 
 goomba->status|=status_fall;
 act_setanim(goomba,anim_backdie);
 score_add(goomba->pos.x,goomba->pos.y,100);
}

int enemy_collision(_game*gm,_act*fire)
{
 int i,fnd=0;
 for(i=0;i<actors_count;i++)
  if((pactors[i]->flags&(sprite_activated|sprite_drawn))==(sprite_activated|sprite_drawn))
   if(isbetween(pactors[i]->kind,enemy_goomba,enemy_redtroopa))
    if(fire==pactors[i])
     ;
    else
    if(act_intersect(fire,pactors[i]))
     {
      goomba_backdie(pactors[i]);
      explosion_add(pactors[i]);
      fnd++;
     }
 return fnd;
}

void troopa_shellrun(_act*goomba)
{
 goomba->defspeed=goomba_speed*4;
 if(hero->pos.x<goomba->pos.x)
  {
   goomba->pos.x=hero->pos.x+8;
   bitclear(goomba->flags,sprite_hflip)
  }
 else
  if(hero->pos.x>=goomba->pos.x)
   {
    goomba->pos.x=hero->pos.x-8;
    goomba->flags|=sprite_hflip;
   }
}

int movingelements_play(_game*gm,_act*goomba)
{
 float speed=goomba->defspeed;
 if(!fbox_ispointinborder(&goomba->pos,worldarea,(float)(GAME_WIDTH*2),(float)16))
  return 0;
 if(movingelements_rise(goomba))
  return 1;
 if(goomba->animid==anim_die)
 {
  goomba->timer++;
  if(goomba->timer>20)
   return 0;
  else
   return 1;
 }
 if(!act_ontheground(goomba))
  goomba->dpos.y=float_min(goomba->dpos.y+gravitydown,hero_maxfallspeed);
 else
  if((goomba->kind==bonus_star)||(goomba->kind==bonus_fireball))
   {
    if(goomba->dpos.y==0)
     goomba->dpos.y=-4;
   }
 
 if(goomba->flags&sprite_hflip)
  goomba->dpos.x=-speed;
 else
  goomba->dpos.x=+speed;

 if((goomba->animid==anim_shell)&&(goomba->defspeed!=0))
  enemy_collision(gm,goomba);

 if(goomba->status&status_fall)
  ;
 else
 if(goomba->kind==bonus_fireball)
  {
   if(enemy_collision(gm,goomba))
    return 0;
  }
 else
 if((fabs(goomba->pos.x-hero->pos.x)<10)&&(fabs(goomba->pos.y-hero->pos.y)<10))
  if(hero->animid==anim_die)
   ;
  else
  if(act_intersect(goomba,hero))
   if(goomba->kind==bonus_star)
    {
     hero->nextstatus|=status_star;
     return 0;
    }
   else
   if(goomba->kind==bonus_mushroom1up)
    {
     lives++;
     return 0;
    }
   else
   if(goomba->kind==bonus_magicmushroom)
    {
     hero->nextstatus|=status_grow;     
     return 0;
    }
   else
   if((hero->pos.y<goomba->pos.y+1)&&(hero->dpos.y>0)&&(hero->animid!=anim_die))
    {
     if((goomba->kind==enemy_greentroopa)||(goomba->kind==enemy_redtroopa))
      {
       if(goomba->defspeed==goomba_speed)
       {
        goomba->defspeed=0;
        act_setanim(goomba,anim_shell);
        score_add(goomba->pos.x,goomba->pos.y,100);
       }
       else
       if(goomba->defspeed==0)
        troopa_shellrun(goomba);
      }
     else
      {
       act_setanim(goomba,anim_die);
       score_add(goomba->pos.x,goomba->pos.y,100);
      }
     hero->nextstatus|=status_autojump;     
    }
   else
    if(hero->flags&sprite_outlined)
     goomba_backdie(goomba);      
    else
    if(((goomba->kind==enemy_greentroopa)||(goomba->kind==enemy_redtroopa))&&(goomba->animid==anim_shell)&&(goomba->defspeed==0))
     troopa_shellrun(goomba);
    else
     hero->nextstatus|=status_killed;
     
 if(goomba->animid==anim_die)
  ;
 else
 if(goomba->dpos.x||goomba->dpos.y)
  {
   _fpos delta={goomba->dpos.x,goomba->dpos.y};
   if(goomba->status&status_fall)
    ;
   else
    {     
     if(handle_collisions(goomba,&delta))
      {     
       if(goomba->dpos.x!=delta.x)
        if(goomba->kind==bonus_fireball)
         {
          explosion_add(goomba);
          return 0;
         }
        else
        if(goomba->dpos.y==0)
         {
         if(goomba->flags&sprite_hflip)
          goomba->flags-=sprite_hflip;      
         else
          goomba->flags|=sprite_hflip;      
         }
       if(goomba->dpos.x!=delta.x)
        goomba->dpos.x=0;
       if(goomba->dpos.y!=delta.y)
        goomba->dpos.y=0;
      }    
     else
      if(act_onhitblock(goomba))
       {
        goomba->dpos.y=-2;
        if(goomba->flags&sprite_hflip)
         goomba->flags-=sprite_hflip;      
        else
         goomba->flags|=sprite_hflip;      
       }
    }
   goomba->pos.x+=delta.x;
   goomba->pos.y+=delta.y;
   if(goomba->animid==anim_shell)
    ;
   else
   if((goomba->status&status_mainmask)==status_normal)
    if(goomba->dpos.x)
     act_setanim(goomba,anim_walk);
  }
 else
  if(goomba->animid==anim_shell)
   ;
  else
   act_setanim(goomba,anim_idle); 
 return 1;
}

void hero_grow(_act*hero)
{
 hero->animset=&charanim[mariohi_anim];
 hero->status|=status_magic;
 hero->flags|=sprite_flashing;
 act_setanim(hero,anim_grow);     
}

void hero_fire(_act*hero)
{
 _act*tmp=actor_get();    
 if(tmp)
  {
   word tile=fireball_anim+1;
   tmp->pos.x=hero->pos.x;
   if(hero->flags&sprite_hflip)
    {
     tmp->flags|=sprite_hflip;
     tmp->pos.x-=5;
    }
   else
    tmp->pos.x+=5;   
   tmp->pos.y=hero->pos.y-4;
   tmp->defspeed=goomba_speed*3;
   tmp->play=movingelements_play;
   tmp->animset=&charanim[tile-1];
   tmp->kind=bonus_fireball;tmp->flags|=sprite_visible;
   act_setanim(hero,anim_fire);
   act_setanim(tmp,anim_idle);   
  }
}

void gui_setmsg(const char*msg)
{
 strcpy(guimsg,msg);
}

void hero_died(_act*hero)
{
 if(lives>1)
  gui_setmsg("OUCH");
 else
  gui_setmsg("GAME OVER");
 act_setanim(hero,anim_die); 
 bitclear(hero->status,status_invincible)
 bitclear(hero->flags,sprite_flashing)
 hero->status|=status_dead;
}

void hero_killed(_act*hero)
{
 if((hero->status&status_dead)==0)
  {
   if(hero->status&status_invincible)
    ;
   else
   if(hero->status&status_magic)
    {
     if((hero->animid==anim_shrink)||((hero->animid==0)&&(hero->prevanimid==anim_shrink)))
      ;
     else
      act_setanim(hero,anim_shrink);
    }
   else
    {
     hero_died(hero);
     hero->dpos.y=-hero_topjumpspeed*1.25f;
    }
  }
}

void hero_handlenextstatus(_game*gm,_act*hero)
{
 if(hero->status&status_dead)
   ;
  else
   {
    if(hero->nextstatus&status_star)
     {
      bitclear(hero->status,status_invincible);
      bitclear(hero->flags,sprite_flashing)
      hero->flags|=sprite_outlined;
      hero->timer=0;
     }
    else
    if(hero->nextstatus&status_killed)
     hero_killed(hero);
    if(hero->status&status_dead)
     ;
    else
     {      
      if(hero->nextstatus&status_autojump)
       {
        act_setanim(hero,anim_jump);
        hero->status|=status_jump;
        hero->dpos.y=-hero_topjumpspeed;
        play_sound(gm,snd_jumpA,1);
       }
      if(hero->nextstatus&status_grow)
       hero_grow(hero);
     }
    hero->nextstatus=0;
   }
}

void hero_setautopipemode(_act*hero,int automode)
{
 hero->timer=0;hero->zorder=-1;
 hero->status|=status_automode;hero->autostatus=automode;
}

void hero_clearautomode(_act*hero)
{
 bitclear(hero->status,status_automode)
 hero->autostatus=0;hero->zorder=1;
}

void hero_leavingarea(_game*gm,_act*hero)
{
 hero_clearautomode(hero);
 bitclear(hero->flags,sprite_visible) 
 gm->scene->status=scene_leaving;
}


void hero_playautomode(_game*gm,_act*hero)
{
 input_clear(gm);
 switch(hero->autostatus)
  {
   case autostatus_demoplay:
    act_setanim(hero,anim_walk);
    if(gm->timer<gm->maxtimer*2)
    {
     if(hero->flags&sprite_hflip)
      gm->input.key_left=true;
     else
      gm->input.key_right=true;
     gm->timer++;
    }
    else
    {
     gm->timer=0;
     if(hero->flags&sprite_hflip)
      hero->flags-=sprite_hflip;
     else
      hero->flags|=sprite_hflip;
    }
   break;
   case autostatus_downthepipe:
    hero->pos.y+=0.5f;
    hero->timer++;
    if(hero->timer==32)
     hero_leavingarea(gm,hero);       
   break;
   case autostatus_rightthepipe:
    hero->pos.x+=0.5f;
    hero->timer++;
    if(hero->timer==20)
     hero_leavingarea(gm,hero);
   break;
   case autostatus_upthepipe:
    hero->pos.y-=0.5f;
    hero->timer++;
    if(hero->timer==32)
     hero_clearautomode(hero);
   break;
   case autostatus_runright:
    gm->input.key_right=true;
   break;
   case autostatus_flagandleaving:
    {
     _framedesc*fr=getframe(hero);
     if((hero->animid==anim_climb)&&((hero->dpos.y)||(flag->pos.y<hero->pos.y-fr->h)))
      {
       if(flag->pos.y<hero->pos.y-fr->h)
        flag->pos.y+=2.0f;
      }
     else
      {
       if(hero->animid==anim_climb)
        {
         score_add(flag->pos.x,flag->pos.y,400);
         act_setanim(hero,anim_walk);
        }
       gm->input.key_right=true;
      }
    }
  }
}

int hero_play(_game*gm,_act*hero)
{
 int move=0;
 
 // we need to have a centralized way to handle potential multiple status change
 // to sort priorities
 if(hero->nextstatus)
  hero_handlenextstatus(gm,hero);

 if(!fbox_ispointinborder(&hero->pos,worldarea,(float)(GAME_WIDTH*2),(float)16))
  {
   hero_died(hero);
   return 0; 
  }

 if(gm->scene->status==scene_entering)
  input_clear(gm);
 else
 if(hero->status&status_automode)
  hero_playautomode(gm,hero);
 
 if(gm->input.key_control)
  if(hero->status&status_fire)
   {
    gm->input.key_control=false;
    hero_fire(hero);
   }

 if(hero->animid==0)
  if(hero->prevanimid==anim_grow)
   {
    if(hero->flags&sprite_flashing)
     {
      hero->flags-=sprite_flashing;
      score_add(hero->pos.x,hero->pos.y-12,1000);
     }
   }
  else
  if(hero->prevanimid==anim_shrink)
   {
    if(hero->status&status_magic)
     {
      hero->animset=&charanim[mario_anim];      
      bitclear(hero->status,status_magic);
      bitclear(hero->status,status_fire);
      hero->status|=status_invincible;
      hero->flags|=sprite_flashing;
      hero->timer=0;
     }
   }
 if((hero->animid==anim_attack)||(hero->animid==anim_fire)||(hero->animid==anim_grow)||(hero->animid==anim_shrink))
  ;
 else
 if(hero->animid==anim_die)
  {
   float gravity=gravityup;
   if(hero->dpos.y<0)
    gravity=gravityup;
   else
    gravity=gravityuppow;
   hero->dpos.y=float_min(hero->dpos.y+gravity,hero_maxfallspeed);
   hero->pos.y+=hero->dpos.y;
   camera_update(gm);
  }
 else
  {
   int onground=act_ontheground(hero);
   
   if(gm->input.key_left)
    {
     if(hero->dpos.x>=-hero_maxspeed)
      hero->dpos.x-=hero_speed;
     hero->flags|=sprite_hflip;     
    }
   else
    if(gm->input.key_right)
     {
      if(hero->dpos.x<=hero_maxspeed)
       hero->dpos.x+=hero_speed;
      if(hero->flags&sprite_hflip) hero->flags-=sprite_hflip;      
     }    
    else
     {
      float friction;
      if(onground)
       friction=ground_friction;
      else
       friction=air_friction;
      if(hero->dpos.x>0)
       hero->dpos.x=float_max(0,hero->dpos.x-friction);
      else
       if(hero->dpos.x<0)
        hero->dpos.x=float_min(0,hero->dpos.x+friction);
     }
   
    if((hero->dpos.y<0)||(!onground))
     {
      float gravity;
      if(hero->dpos.y<0)
       if(gm->input.key_up)
        gravity=gravityuppow;
       else
        gravity=gravityup;
      else
       gravity=gravitydown;

      hero->dpos.y=float_min(hero->dpos.y+gravity,hero_maxfallspeed);
     }
    else     
      if((hero->status&status_mainmask)==status_jump)
       hero->status^=status_jump;

   if(gm->input.key_up)
    {
     if((hero->status&status_mainmask)==status_normal)
      {
       act_setanim(hero,anim_jump);
       hero->status|=status_jump;
       hero->dpos.y-=hero_topjumpspeed+float_min(0.5f,abs(hero->dpos.x));
       play_sound(gm,snd_jumpA,1);
      }
    }
   else
    if(gm->input.key_down)
     {
      if(act_ondownsecretpssage(hero,&changearea))
       hero_setautopipemode(hero,autostatus_downthepipe);              
     }   
   if(hero->dpos.x||hero->dpos.y)
    {
     _fpos delta={hero->dpos.x,hero->dpos.y};  
     int   fnd=0,nomove=0;
     if(coin_collect(hero,&delta))
      fnd++;
     if((hero->dpos.x>0)&&act_onleftrightsecretpssage(hero,&changearea))
      {
       hero_setautopipemode(hero,autostatus_rightthepipe);      
       act_setanim(hero,anim_walk);
      }
     else
     if(act_onleftpole(hero)&&((hero->status&status_automode)==0))
      {
       hero->dpos.x=0;
       hero->status|=status_automode;hero->autostatus=autostatus_flagandleaving;
       act_setanim(hero,anim_climb);
      }
     else
     if(act_onlevelexit(hero))
      {       
       bitclear(hero->flags,sprite_visible)
       bitclear(hero->status,status_automode)
       hero->autostatus=0;
       gm->scene->status=scene_levelcompleted;
       gm->timer=0;gm->maxtimer=GAME_FRAMERATE;
      }
     else
     if(handle_collisions(hero,&delta))
      {       
       if(isbetween(colKIND,tile_background_wallA,tile_background_wallB)||(colKIND==tile_background_blockQ1)||(colKIND==tile_background_blockQH))
        if((hero->dpos.y<0)&&isbetween(hero->pos.x,colX*level.tilemap[layer_map].tilew-3,(colX+1)*level.tilemap[layer_map].tilew+3))
         {
          word what=tile_get(layer_elements,colX,colY);    
          if(tile_get(layer_map,colX,colY-1)==tile_background_blockC1)
           {
            bonus_add(gm,colX,colY-1,bonus_coin);
            tile_set(layer_map,colX,colY-1,0);
           }
          if((hero->status&status_magic)&&isbetween(colKIND,tile_background_wallA,tile_background_wallB)&&(what==0))
           {
            tile_set(layer_map,colX,colY,0);
            fragments_add(colX,colY);
           }
          else
           {
            if((blockhitX==colX)&&(blockhitY==colY))
             ;
            else
             {blockhitX=colX;blockhitY=colY;blockhitTD=0;blockhitT=0;}
            if(what)
             {
              bonus_add(gm,blockhitX,blockhitY,what);              
              tile_set(layer_elements,colX,colY,0);
             }
            if(colKIND==tile_background_blockQH)
             tile_set(layer_map,colX,colY,tile_background_blockQX);
           }
         }

       if(hero->dpos.x!=delta.x)
        hero->dpos.x=0;
       if(hero->dpos.y!=delta.y)
        hero->dpos.y=0;

       if(hero->dpos.x&&hero->dpos.y)
        if((delta.x==0)&&(delta.y==0))
         nomove=1;
      }

     hero->pos.x+=delta.x;
     hero->pos.y+=delta.y;

     if((hero->status&status_mainmask)==status_normal)
      if(hero->animid==anim_climb)
       ;
      else
       if(hero->dpos.y>0)
        act_setanim(hero,anim_jump);
       else
        if(hero->dpos.x!=0)
         act_setanim(hero,anim_walk);
        else
         if(hero->dpos.y==0)
          act_setanim(hero,anim_idle); 

     if(hero->pos.x<worldarea->x1)
      hero->pos.x=worldarea->x1;
     else
      if(hero->pos.x>worldarea->x2)
       hero->pos.x=worldarea->x2;
     
    }
   else
    if(hero->animid==anim_climb)
     ;
    else
     act_setanim(hero,anim_idle); 

   camera_update(gm);
  }
 if(hero->status&status_invincible)
  {
   hero->timer++;
   if(hero->timer>=GAME_FRAMERATE*3)
    {
     bitclear(hero->status,status_invincible);
     bitclear(hero->flags,sprite_flashing);
    }
  }
 else
 if(hero->flags&sprite_outlined)
  {
   hero->timer++;
   if(hero->timer>=GAME_FRAMERATE*7)
    hero->flags-=sprite_outlined;
  }
 return 1;
}

// ************************************************************

int ingame_update(_game*gm)
{
 if((hero->flags&sprite_used)==0)
  gm->scene->status=scene_leaving;

 if(gm->scene->status==scene_playing)
  if(gm->input.key_escape)
   return splash_leave(gm,&home);
  else
   {
    int i,del=0;
    for(i=0;i<actors_count;i++)
     if(pactors[i]->flags&sprite_activated)
      {
       if(pactors[i]->play==NULL)
        ;
       else
       if(pactors[i]->play(gm,pactors[i])==0)
        {bitclear(pactors[i]->flags,sprite_used);del++;}
      }
     else
      if(pactors[i]->pos.x<cam.x+GAME_WIDTH*2)
       pactors[i]->flags|=sprite_activated;
    // pack used sprites
    if(del)
     {
      int j;
      i=j=0;
      while(i<actors_count)
       if(pactors[i]->flags&sprite_used)
        pactors[j++]=pactors[i++];
       else
        i++;
      actors_count=j;
     }
   }

 canvas_update(gm);    

 if(gm->scene->status==scene_levelcompleted)
  {
   if(rsecs)
    {
     if(rsecs>10)
      {
       rsecs-=10;
       score+=500;
      }
     else
      {
       rsecs--;
       score+=50;
      }
    }
   else
    {
     if(gm->timer==0) gui_setmsg("LEVEL COMPLETED");
     gm->timer++;
    }
   if(gm->timer==gm->maxtimer)
    {
      lv++;
      level_start(gm);
    }
  }
 else
 if(gm->scene->status==scene_entering)
  {
   if(hero)
    hero->play(gm,hero);
   efx_fade(&canvas,gm->timer,gm->maxtimer,1);
   gm->timer++;
   if(gm->timer==gm->maxtimer)
    {gm->scene->status=scene_playing;gm->timer=0;gm->maxtimer=GAME_FRAMERATE;}
  }
 else
 if(gm->scene->status==scene_leaving)
  {   
   efx_fade(&canvas,gm->timer,gm->maxtimer,-1);
   gm->timer++;
   if(gm->timer==gm->maxtimer)
    {
     if(changearea!=-1)
      level_loadarea(gm,changearea,1|2|4);
     else
      {
       if(score>topscore)
        topscore=score;
       lives--;
       if(lives>0)
        level_start(gm);
       else
        splash_leave(gm,&home);
      }
    }
  }

 return 1;
}

_scene ingame={
                ingame_enter,
                ingame_leave,
                ingame_update
               };

// ************************************************************