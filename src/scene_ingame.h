// ************************************************************
// INGAME CODE
// ************************************************************

#define tile_background_groundA 1
#define tile_background_groundB 2

#define tile_background_wallA   3
#define tile_background_wallB   4

#define tile_background_block   5

#define tile_background_blockQ1 6
#define tile_background_blockQ2 7
#define tile_background_blockQ3 8

#define tile_background_blockQH 9

#define tile_background_blockQX 10

#define tile_background_pipeA   11
#define tile_background_pipeZ   22

#define tile_background_coin    23

#define tile_solid_start        tile_background_groundA 
#define tile_solid_end          tile_background_pipeZ

// ************************************************************

#define bonus_coin          4
#define bonus_magicmushroom 5
#define bonus_mushroom1up   6
#define bonus_star          7
#define bonus_fireflower    8
#define bonus_fragments     9
#define bonus_fireball      10

#define forcedforward 1

#define status_normal      0
#define status_jump        1
#define status_dead        2
#define status_fall        4

#define status_magic       8
#define status_invincible 16
#define status_fire       32

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

#define level_map         0
#define level_elements    1

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

int  coin_play(_game*gm,_act*hero);
int  score_play(_game*gm,_act*goomba);
void bonus_add(int x,int y,int tile);
void fragments_add(int x,int y);

int  fireflower_play(_game*gm,_act*hero);

int  goomba_play(_game*gm,_act*goomba);
void goomba_backdie(_act*goomba);

char      guimsg[32];
_anim     gui;
_actplay  charplay[]={hero_play,goomba_play,goomba_play,coin_play,goomba_play,goomba_play};
short     lives,coins,score,secs;
short     wrld,lv;

byte      blockQ[]={tile_background_blockQ1,tile_background_blockQ2,tile_background_blockQ3,tile_background_blockQ2};
short     blockhitX=-1,blockhitY=-1,blockhitT=0,colX,colY,colKIND;
byte      blockdisplacement[]={1,2,3,3,2,1,0};

// ************************************************************

#define worldarea_start 10
#define worldarea_end   11
int level_load(int world,int lv,int flags)
{
 int  x,y,start,end; 
 word mapw,maph,tw,th;
 char tilemap[32];

 *guimsg=0;
 actor_reset(); 
 patch_reset();

 strcpy(tilemap,"world_1x1");
 tilemap[6]='0'+world;tilemap[8]='0'+lv; 
 if(!tilemap_load(&level,tilemap))
  return 0;
 strcat(tilemap,".a");
 if(!img_load(&offscreen,tilemap))
  return 0;
 
 mapw=level.tilemap[level_elements].mapw;maph=level.tilemap[level_elements].maph;
 tw=level.tilemap[level_elements].tilew;th=level.tilemap[level_elements].tilew;

 worldareascnt=0; 
 for(start=end=x=0;x<mapw;x++)
  {
   word tile=tile_get(level_elements,x,0);
   if(tile==worldarea_start)
    start=x;
   else
    if(tile==worldarea_end)
    {
     for(y=1;y<maph;y++)
      if(tile_get(level_elements,x,y)==worldarea_end)
       break;
     worldarea=&worldareas[worldareascnt++];
     
     worldarea->x1=start*tw;
     worldarea->x2=(float)worldarea->x1+(x-start)*tw;
     worldarea->y1=0;
     worldarea->y2=(float)worldarea->y1+y*th+th;

     worldarea->bkg=x+1;

     worldarea->tx1=start;worldarea->tx2=x+1;
     worldarea->ty1=0;worldarea->ty2=y+1;

     end=x;
    }
  }
 worldarea=&worldareas[0];

 if(worldareascnt)
  if(flags&(1|2))
   {
   int y,tw=level.tilemap[level_elements].tilew,th=level.tilemap[level_elements].tileh;
   for(x=worldarea->tx1;x<worldarea->tx2;x++)
    for(y=worldarea->ty1;y<worldarea->ty2;y++)   
     {
      word     tile=tile_get(level_elements,x,y);
      if(tile)
       {
        int px=x*tw;
        int py=y*th;
        switch(tile)
         {
          case 1:
           if(flags&1)
            {
            hero=actor_get();    
            hero->pos.x=(float)(px+tw/2);
            hero->pos.y=(float)(py+th);
            hero->animset=&charanim[tile-1];
            hero->play=hero_play;hero->flags|=sprite_visible;
            act_setanim(hero,anim_idle);
            }
          break;
          case 2:
          case 3:
           if(flags&2)
            {
             _act*tmp=actor_get();    
             tmp->kind=tile;
             tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);
             tmp->animset=&charanim[tile-1];tmp->flags|=sprite_hflip;
             tmp->play=goomba_play;tmp->flags|=sprite_visible;
             act_setanim(tmp,anim_walk);
            }
           break;
         }
       }
     }   
  }
 return worldareascnt;
}

void level_start(_game*gm)
{ 
 level_load(wrld,lv,1|2);
 if(hero)
  camera_update(gm);
 
 secs=300*GAME_FRAMERATE;

 gm->scene->status=scene_entering;
 gm->timer=0;gm->maxtimer=GAME_FRAMERATE;
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
 anim_load(&charanim[charanim_cnt++],"coin");
 anim_load(&charanim[charanim_cnt++],"magicmushroom");
 anim_load(&charanim[charanim_cnt++],"mushroom1up");
 anim_load(&charanim[charanim_cnt++],"star");
 fireflower_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"fireflower");

 brickpieces_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"brickfragments");
 fireball_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"fireball"); 

 score_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"score");

 mariohi_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"mariohi");
 mariofire_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"mariofire");

 anim_idle=strhash("idle");
 anim_walk=strhash("walk");
 
 anim_attack=strhash("attack");
 anim_fire=strhash("fire");

 anim_jump=strhash("jump");
 
 anim_die=strhash("die");
 anim_backdie=strhash("backdie");

 anim_grow=strhash("grow");
 anim_shrink=strhash("shrink");
 
 anim_climb=strhash("climb");

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
   //map[x+y*level.tilemap[mapid].mapw]=ntile;
   return 1;
  }
 else
  return 0;
}

void tilebackground_blit(_img*canvas,int bx,int by,int bw,int bh,_tilemaps*tm,_img*i,int cx,int cy)
{
 int      x,y,tx=0,ty,played=0;
 word     tw=tm->tilemap[level_map].tilew,th=tm->tilemap[level_map].tileh;
 for(y=0;y<tm->tilemap[level_map].maph;y++)
  if(by+y*th-cy+th<0)
    ;
   else
   if(by+y*th-cy>canvas->h)
    break;
   else
    for(x=0;x<tm->tilemap[level_map].mapw;x++)
     if(bx+x*tw-cx+tw<0)
      ;
     else
      if(bx+x*tw-cx>canvas->w)
       break;
      else
       {
        word tile=tile_get(level_map,worldarea->bkg,y);
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
 word tw=tm->tilemap[level_map].tilew,th=tm->tilemap[level_map].tileh;
 for(y=0;y<tm->tilemap[level_map].maph;y++)
  if(by+y*th-cy+th<0)
    ;
   else
   if(by+y*th-cy>canvas->h)
    break;
   else
    for(x=0;x<tm->tilemap[level_map].mapw;x++)
     if(bx+x*tw-cx+tw<0)
      ;
     else
      if(bx+x*tw-cx>canvas->w)
       break;
      else
       {
        word tile=tile_get(level_map,x,y);
        if(tile)
         {
          if(tile==tile_background_blockQ1)
           tile=blockQ[(secs/(GAME_FRAMERATE/2))%sizeof(blockQ)];
          ty=th*tile;
          if((blockhitX==x)&&(blockhitY==y))
           {
            int displacement=blockdisplacement[(blockhitT/(GAME_FRAMERATE/15))];
            ty=th*tile;
            img_blit(canvas,bx+x*tw-cx,by+y*th-cy-displacement,i,tx,ty,tw,th,0);
            blockhitT++;
            if((blockhitT/(GAME_FRAMERATE/15))>=sizeof(blockdisplacement))
             {
              word ltile=tile_get(level_map,x,y);
              blockhitX=blockhitY=-1;blockhitT=0;
              if((ltile==tile_background_blockQ1)||(ltile==tile_background_blockQH))
               tile_set(level_map,x,y,tile_background_blockQX);
               //map[x+y*tm->tilemap[t].mapw]=tile_background_blockQX;
             }
           }
          else
           img_blit(canvas,bx+x*tw-cx,by+y*th-cy,i,tx,ty,tw,th,0);
          played++;
         }
       }
}

void gui_draw()
{
 int y=1,x=1,w=6,rsecs=secs/GAME_FRAMERATE,digit=3;
 int marioid=2,coinid=0,xid=1;
 
 img_blit(&canvas,x,y,&gui.atlas,marioid*w,0,6,6,0);x+=6;
 img_blit(&canvas,x,y,&gui.atlas,xid*w,0,6,6,0);x+=4;
 gui_drawdigits(x,y,lives,1);

 y=7;x=2;
 img_blit(&canvas,x,y,&gui.atlas,coinid*w,0,6,6,0);x+=4;x++;
 img_blit(&canvas,x,y,&gui.atlas,xid*w,0,6,6,0);x+=4;
 gui_drawdigits(x,y,coins,2);

 //y=1;x=(GAME_WIDTH-3*6)/2+1;
 //gui_drawstring(x,y,"SCORE");
 //y=7;
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

 tilebackground_blit(&canvas,0,0,GAME_WIDTH,GAME_HEIGHT,&level,&offscreen,f2int(cam.x),f2int(cam.y));

 tilemap_blit(&canvas,0,0,GAME_WIDTH,GAME_HEIGHT,&level,&offscreen,f2int(cam.x),f2int(cam.y));
 
 qsort(&pactors[0],actors_count,sizeof(pactors[0]),actors_ysort);
 for(i=0;i<actors_count;i++)
  act_draw(gm,pactors[i]);

 gui_draw();

 if(gm->scene->status==scene_playing)
 {
  if(secs>0)
   secs--;
  if(*guimsg)
   gui_drawstring(-1,-1,guimsg);
 }

}

void camera_update(_game*gm)
{
 _framedesc*fr=getframe(hero);
 float      w=fr->w/2;
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
    ;
   else
    {
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
     }
   return 1;
  }
 else
  return 0;
}

int handle_aabbcollisioncore(_aabb*box,_fpos*delta)
{
 int txx,tyy,x,y,tw=level.tilemap[level_map].tilew,th=level.tilemap[level_map].tileh,cnt=0,ay,by,ax,bx;
 //word    *map=&level.maps[level.tilemap[id].mappos];
 txx=f2int((box->x+box->w/2)/tw);
 tyy=f2int((box->y+box->h/2)/th);  
 if(delta->x>0) {ax=0;bx=2;}
 else
 if(delta->x<0) {ax=-2;bx=0;}
 else           {ax=-1;bx=1;}
 if(delta->y>0) {ay=0;by=2;}
 else
 if(delta->y<0) {ay=-2;by=0;}
 else           {ay=-1;by=1;}
 for(y=ay;y<=by;y++)
  if(isbetween(tyy+y,0,level.tilemap[level_map].maph-1))
   for(x=ax;x<=bx;x++)
    if(isbetween(txx+x,0,level.tilemap[level_map].mapw-1))
     {
      word what=tile_get(level_map,(txx+x),(tyy+y));
      if(isbetween(what,tile_solid_start,tile_solid_end))
       {
        _aabb tile;
        tile.x=(float)(txx+x)*tw;
        tile.y=(float)(tyy+y)*th;
        tile.w=(float)tw;tile.h=(float)th;
        if(aabb_intersect(box,&tile,delta))
        {
         colX=txx+x;colY=tyy+y;colKIND=what;
         cnt++;
        }
       }
     }
 return cnt;
}

int handle_collisions(_act*c,_fpos*delta)
{
 _aabb box; 
 act_getaabb(c,&box); 
 if(handle_aabbcollisioncore(&box,delta))
  return 1;
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
 if(handle_aabbcollisioncore(&box,&delta))
  return 1;
 return 0;
}

// ************************************************************

void fragments_add(int x,int y)
{
 _act*tmp=actor_get();    
 int  id=1,tw=level.tilemap[id].tilew,th=level.tilemap[id].tileh,tile=brickpieces_anim+1;
 int  px=x*tw,py=(y-1)*th;
 tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);  
 tmp->play=goomba_play;
 tmp->dpos.y=-6; 
 tmp->animset=&charanim[tile-1];
 tmp->kind=tile;tmp->flags|=sprite_visible;
 tmp->status|=status_fall;
 act_setanim(tmp,anim_idle);   

 tmp=actor_get();
 tmp->flags|=sprite_hflip;
 tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);  
 tmp->play=goomba_play;
 tmp->dpos.y=-6;
 tmp->animset=&charanim[tile-1];
 tmp->kind=tile;tmp->flags|=sprite_visible;
 tmp->status|=status_fall;
 act_setanim(tmp,anim_idle);   
}

void bonus_add(int x,int y,int tile)
{
 if(tile)
  {
   _act*tmp=actor_get();    
   int  id=1,tw=level.tilemap[id].tilew,th=level.tilemap[id].tileh;
   int  px=x*tw,py=(y-1)*th;
   tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);   
   if(tile==bonus_coin)
    {
     tmp->play=coin_play;
     tmp->dpos.y=-6;
    }
   else
   if(tile==bonus_magicmushroom)
   {
    if(hero->status&status_magic)
     {
      tile=fireflower_anim+1;
      tmp->play=fireflower_play;
     }
    else
     tmp->play=goomba_play;
   }
   else
   if(tile==bonus_mushroom1up)
    tmp->play=goomba_play;
   else
    if(tile==bonus_star)
     {
      tmp->play=goomba_play;
      tmp->dpos.y=-6;
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
   coins++;
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
   if(isbetween(pactors[i]->kind,2,3))
    if(act_intersect(fire,pactors[i]))
    {
     goomba_backdie(pactors[i]);
     fnd++;
    }
 return fnd;
}

int goomba_play(_game*gm,_act*goomba)
{
 float speed;

 if(!fbox_ispointinborder(&goomba->pos,worldarea,(float)(GAME_WIDTH*2),(float)16))
  return 0;
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
 speed=goomba_speed;
 if(goomba->kind==bonus_star)
  speed*=2;
 else
  if(goomba->kind==bonus_fireball)
   speed*=3;

 if(goomba->flags&sprite_hflip)
  goomba->dpos.x=-speed;
 else
  goomba->dpos.x=+speed;

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
  if(act_intersect(goomba,hero))
   if(goomba->kind==bonus_star)
    {
     bitclear(hero->status,status_invincible);
     hero->flags|=sprite_outlined;
     hero->timer=0;
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
     hero_grow(hero);
     return 0;
    }
   else
   if((hero->pos.y<goomba->pos.x+1)&&(hero->dpos.y>0)&&(hero->animid!=anim_die))
    {
     act_setanim(goomba,anim_die);
     score_add(goomba->pos.x,goomba->pos.y,100);
     act_setanim(hero,anim_jump);
     hero->status|=status_jump;
     hero->dpos.y=-hero_topjumpspeed;
    }
   else
    if(hero->flags&sprite_outlined)
     goomba_backdie(goomba);      
    else
     hero_killed(hero);
 if(goomba->animid==anim_die)
  ;
 else
 if(goomba->dpos.x||goomba->dpos.y)
  {
   _fpos delta={goomba->dpos.x,goomba->dpos.y};
   if(goomba->status&status_fall)
    ;
   else
   if(handle_collisions(goomba,&delta))
    {     
     if(goomba->dpos.x!=delta.x)
      if(goomba->kind==bonus_fireball)
       return 0;
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
   goomba->pos.x+=delta.x;
   goomba->pos.y+=delta.y;
   if((goomba->status&status_mainmask)==status_normal)
    if(goomba->dpos.x)
     act_setanim(goomba,anim_walk);
  }
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
   tmp->play=goomba_play;
   tmp->animset=&charanim[tile-1];
   tmp->kind=tile;tmp->flags|=sprite_visible;
   act_setanim(hero,anim_fire);
   act_setanim(tmp,anim_idle);   
  }
}

void hero_killed(_act*hero)
{
 if((hero->status&status_dead)==0)
  {
   if(hero->status&status_invincible)
    ;
   else
   if(hero->status&status_magic)
    act_setanim(hero,anim_shrink);
   else
    {
     if(lives>1)
      strcpy(guimsg,"OUCH");
     else
      strcpy(guimsg,"GAME OVER");
     act_setanim(hero,anim_die);
     hero->status|=status_dead;
     hero->dpos.y=-hero_topjumpspeed*1.25f;
    }
  }
}

int hero_play(_game*gm,_act*hero)
{
 int move=0;
 //if(hero->status&status_dead)
 if(!fbox_ispointinborder(&hero->pos,worldarea,(float)(GAME_WIDTH*2),(float)16))
  return 0; 
 
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
      }
    }
   else
    if(gm->input.key_down)
     {
     }   
   if(hero->dpos.x||hero->dpos.y)
    {
     _fpos delta={hero->dpos.x,hero->dpos.y};     
     if(handle_collisions(hero,&delta))
      {
       
       if(isbetween(colKIND,tile_background_wallA,tile_background_wallB)||(colKIND==tile_background_blockQ1)||(colKIND==tile_background_blockQH))
        if(hero->dpos.y<0)
         {
          word what=tile_get(level_elements,colX,colY);            
          if((hero->status&status_magic)&&isbetween(colKIND,tile_background_wallA,tile_background_wallB)&&(what==0))
           {
            tile_set(level_map,colX,colY,0);
            fragments_add(colX,colY);
           }
          else
           {            
            blockhitX=colX;blockhitY=colY;blockhitT=0;          
            if(what)
             {
              bonus_add(blockhitX,blockhitY,what);              
              tile_set(level_elements,colX,colY,0);
             }
            if(colKIND==tile_background_blockQH)
             tile_set(level_map,colX,colY,tile_background_blockQX);
           }
         }

       if(hero->dpos.x!=delta.x)
        hero->dpos.x=0;
       if(hero->dpos.y!=delta.y)
        hero->dpos.y=0;
      }
     hero->pos.x+=delta.x;
     hero->pos.y+=delta.y;
     if((hero->status&status_mainmask)==status_normal)
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
  {
   int i,del=0;
   for(i=0;i<actors_count;i++)
    if(pactors[i]->flags&sprite_activated)
     {
      if(pactors[i]->play(gm,pactors[i])==0)
       {pactors[i]->flags-=sprite_used;del++;}
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

 if(gm->scene->status==scene_entering)
  {
    efx_fade(&canvas,gm->timer,gm->maxtimer,1);
    gm->timer++;
    if(gm->timer==gm->maxtimer)
     {gm->scene->status=scene_playing;gm->timer=0;gm->maxtimer=GAME_FRAMERATE*2;}
  }
 else
 if(gm->scene->status==scene_leaving)
  {
   efx_fade(&canvas,gm->timer,gm->maxtimer,-1);
   gm->timer++;
   if(gm->timer==gm->maxtimer)
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

 return 1;
}

_scene ingame={
                ingame_enter,
                ingame_leave,
                ingame_update
               };

// ************************************************************