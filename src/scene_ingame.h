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

#define tile_background_blockQX 9

#define tile_background_pipeA   10
#define tile_background_pipeZ   21

#define tile_background_coin    22

#define tile_solid_start        tile_background_groundA 
#define tile_solid_end          tile_background_pipeZ

// ************************************************************

#define forcedforward 1

#define status_normal 0
#define status_jump   1
#define status_dead   2

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

// ************************************************************

void camera_update(_game*gm);
int  hero_play(_game*gm,_act*hero);
void hero_killed(_act*hero);
int  goomba_play(_game*gm,_act*hero);

// ************************************************************

_img      offscreen;
_act     *hero;
_anim     charanim[3],gui;
int       anim_idle,anim_walk,anim_attack,anim_jump,anim_die;
_tilemaps level;
_fbox*    world;
_fbox     worlds[8]; 
short     lives,coins,score,secs;

byte      blockQ[]={tile_background_blockQ1,tile_background_blockQ2,tile_background_blockQ3,tile_background_blockQ2};
short     blockhitX=-1,blockhitY=-1,blockhitT=0,colX,colY,colKIND;
byte      blockdisplacement[]={1,2,3,3,2,1,0};

/*_fpos     logz[8192];
int       ilog;*/

// ************************************************************

int ingame_enter(_game*gm)
{ 
 int x,y;

 tilemap_load(&level,"world_1x1");
 img_load(&offscreen,"world_1x1.a");

 anim_load(&gui,"gui");

 anim_load(&charanim[0],"mario");
 anim_load(&charanim[1],"goomba");
 anim_load(&charanim[2],"greentroopa");

 anim_idle=strhash("idle");
 anim_walk=strhash("walk");
 anim_attack=strhash("attack");
 anim_jump=strhash("jump");
 anim_die=strhash("die");

 lives=3;coins=0;score=0;secs=300*GAME_FRAMERATE;
 
 world=&worlds[0];
 {
  int      id=0;
  word     *map=&level.maps[level.tilemap[id].mappos];
  int      tw=level.tilemap[id].tilew,th=level.tilemap[id].tileh;
  for(x=0;x<level.tilemap[id].mapw;x++)
   if(map[x]==0)
    {
     world->x1=0;world->x2=(float)world->x1+(x-1)*tw;
     world->y1=0;world->y2=(float)world->y1+level.tilemap[id].rmaph;
     break;
    }
 }

 {
  int      id=1;
  word     *map=&level.maps[level.tilemap[id].mappos];
  int      tw=level.tilemap[id].tilew,th=level.tilemap[id].tileh;
  for(x=0;x<level.tilemap[id].mapw;x++)
   for(y=0;y<level.tilemap[id].maph;y++)   
    {
     word     tile=map[x+y*level.tilemap[id].mapw];
     if(tile)
      {
       int px=x*tw;
       int py=y*th;
       switch(tile)
        {
         case 1:
          hero=actor_get();    
          hero->pos.x=(float)(px+tw/2);
          hero->pos.y=(float)(py+th);
          hero->animset=&charanim[tile-1];
          hero->play=hero_play;hero->flags|=sprite_visible;
          act_setanim(hero,anim_idle);
         break;
         case 2:
         case 3:
          {
           _act*tmp=actor_get();    
           tmp->pos.x=(float)(px+tw/2);tmp->pos.y=(float)(py+th);
           tmp->animset=&charanim[tile-1];tmp->flags|=sprite_hflip;
           tmp->play=goomba_play;tmp->flags|=sprite_visible;
           act_setanim(tmp,anim_walk);
          }
          break;
        }
      }
    }
  if(hero)
   camera_update(gm);
 }
 
 gm->scene->status=scene_entering;
 gm->timer=0;gm->maxtimer=GAME_FRAMERATE;

 return 1;
}

int ingame_leave(_game*gm,_scene*next)
{
 anim_unload(&charanim[0]);
 anim_unload(&charanim[1]);
 anim_unload(&charanim[1]);

 anim_unload(&gui);
 
 img_delete(&offscreen); 
 return 1;
}

void background_blit(_img*canvas,int bx,int by,int bw,int bh)
{
 img_box(canvas,bx,by,bw,bh,0xfffc945c);
}

void tilebackground_blit(_img*canvas,int bx,int by,int bw,int bh,_tilemaps*tm,_img*i,int cx,int cy)
{
 int      x,y,tx=0,ty,t=0,played=0;
 word    *map=&tm->maps[tm->tilemap[t].mappos];
 word     tw=tm->tilemap[t].tilew,th=tm->tilemap[t].tileh;
 for(y=0;y<tm->tilemap[t].maph;y++)
  if(by+y*th-cy+th<0)
    ;
   else
   if(by+y*th-cy>canvas->h)
    break;
   else
    for(x=0;x<tm->tilemap[t].mapw;x++)
     if(bx+x*tw-cx+tw<0)
      ;
     else
      if(bx+x*tw-cx>canvas->w)
       break;
      else
       {
        word     tile=map[224+y*tm->tilemap[t].mapw];
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
 int      x,y,tx=0,ty,t=0,played=0;
 word    *map=&tm->maps[tm->tilemap[t].mappos];
 word     tw=tm->tilemap[t].tilew,th=tm->tilemap[t].tileh;
 for(y=0;y<tm->tilemap[t].maph;y++)
  if(by+y*th-cy+th<0)
    ;
   else
   if(by+y*th-cy>canvas->h)
    break;
   else
    for(x=0;x<tm->tilemap[t].mapw;x++)
     if(bx+x*tw-cx+tw<0)
      ;
     else
      if(bx+x*tw-cx>canvas->w)
       break;
      else
       {
        word     tile=map[x+y*tm->tilemap[t].mapw];
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
              blockhitX=blockhitY=-1;blockhitT=0;
              if(map[x+y*tm->tilemap[t].mapw]==tile_background_blockQ1)
               map[x+y*tm->tilemap[t].mapw]=tile_background_blockQX;
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

 y=1;x=(GAME_WIDTH-3*6)/2+1;
 gui_drawstring(x,y,"SCORE");
 y=7;x=(GAME_WIDTH-6*4)/2+2;
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
  act_draw(pactors[i]);

 gui_draw();

 if(gm->scene->status==scene_playing)
  if(secs>0)
   secs--;

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

 if(cam.x<world->x1) 
  cam.x=world->x1;
 else
  if(cam.x+GAME_WIDTH>world->x2)
   cam.x=(float)(world->x2-GAME_WIDTH);

 if(hero->pos.y-cam.y>GAME_HEIGHT*3/5)
  cam.y=hero->pos.y-GAME_HEIGHT*3/5;
 if((hero->pos.y-h)-cam.y<GAME_HEIGHT*2/5)
  cam.y=(hero->pos.y-h)-GAME_HEIGHT*2/5;

 if(cam.y<world->y1) 
  cam.y=world->y1;
 else
  if(cam.y+GAME_HEIGHT>world->y2)
   cam.y=(float)(world->y2-GAME_HEIGHT);

 if(forcedforward)
  world->x1=float_max(world->x1,cam.x-GAME_WIDTH);
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
 int      id=0,txx,tyy,x,y,tw=level.tilemap[id].tilew,th=level.tilemap[id].tileh,cnt=0,ay,by,ax,bx;
 word    *map=&level.maps[level.tilemap[id].mappos];
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
  if(isbetween(tyy+y,0,level.tilemap[id].maph-1))
   for(x=ax;x<=bx;x++)
    if(isbetween(txx+x,0,level.tilemap[id].mapw-1))
     {
      word what=map[(txx+x)+(tyy+y)*level.tilemap[id].mapw];
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

int goomba_play(_game*gm,_act*goomba)
{
 if(!fbox_ispointinborder(&goomba->pos,world,(float)(GAME_WIDTH*2),(float)16))
  return 0;
 if(goomba->animid==anim_die)
  return 1;
 if(!act_ontheground(goomba))
   goomba->dpos.y=float_min(goomba->dpos.y+gravitydown,hero_maxfallspeed);
 if(goomba->flags&sprite_hflip)
  goomba->dpos.x=-goomba_speed;
 else
  goomba->dpos.x=+goomba_speed;
 if((fabs(goomba->pos.x-hero->pos.x)<10)&&(fabs(goomba->pos.y-hero->pos.y)<10))
  if(act_intersect(goomba,hero))
   if((hero->pos.y<goomba->pos.x+1)&&(hero->dpos.y>0)&&(hero->animid!=anim_die))
    {
     act_setanim(goomba,anim_die);score+=100;
     act_setanim(hero,anim_jump);
     hero->status=status_jump;
     hero->dpos.y=-hero_topjumpspeed;
    }
   else
    hero_killed(hero);
 if(goomba->animid==anim_die)
  ;
 else
 if(goomba->dpos.x||goomba->dpos.y)
  {
   _fpos delta={goomba->dpos.x,goomba->dpos.y};
   if(handle_collisions(goomba,&delta))
    {
     if(goomba->pos.x!=delta.x)
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
   if(goomba->status==status_normal)
    if(goomba->dpos.x)
     act_setanim(goomba,anim_walk);
  }
 else
  act_setanim(goomba,anim_idle); 
 return 1;
}

void hero_killed(_act*hero)
{
 if(hero->status!=status_dead)
  {
   act_setanim(hero,anim_die);
   hero->status=status_dead;
   hero->dpos.y-=hero_topjumpspeed+float_min(0.5f,abs(hero->dpos.x));
  }
}

int hero_play(_game*gm,_act*hero)
{
 int move=0;
 if(gm->input.key_control)
  act_setanim(hero,anim_attack);
 if(hero->animid==anim_attack)
  ;
 else
 if(hero->animid==anim_die)
  {
   float gravity;
   if(hero->dpos.y<0)
    gravity=gravityup;
   else
    gravity=gravitydown/2;
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
      if(hero->status==status_jump)
       hero->status=status_normal;

   if(gm->input.key_up)
    {
     if(hero->status==status_normal)
      {
       act_setanim(hero,anim_jump);
       hero->status=status_jump;
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
       
       if(isbetween(colKIND,tile_background_wallA,tile_background_wallB)||(colKIND==tile_background_blockQ1))
        if(hero->dpos.y<0)
         {blockhitX=colX;blockhitY=colY;blockhitT=0;}

       if(hero->dpos.x!=delta.x)
        hero->dpos.x=0;
       if(hero->dpos.y!=delta.y)
        hero->dpos.y=0;
      }
     hero->pos.x+=delta.x;
     hero->pos.y+=delta.y;
     if(hero->status==status_normal)
      if(hero->dpos.x!=0)
       act_setanim(hero,anim_walk);

     if(hero->pos.x<world->x1)
      hero->pos.x=world->x1;
     else
      if(hero->pos.x>world->x2)
       hero->pos.x=world->x2;
     
    }
   else
    act_setanim(hero,anim_idle); 

   camera_update(gm);
  }
 return 1;
}

// ************************************************************

int ingame_update(_game*gm)
{
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
    splash_leave(gm,&ingame);
  }

 return 1;
}

_scene ingame={
                ingame_enter,
                ingame_leave,
                ingame_update
               };

// ************************************************************