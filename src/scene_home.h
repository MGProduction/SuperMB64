//
// Copyright (c) 2023, Marco Giorgini [ @marcogiorgini ]
// Distributed under the MIT License
//
// home screen code for SuperMB64
//

// ************************************************************
// HOME SCREEN CODE
// ************************************************************

_img      home_logo;

void animid_init()
{
 anim_idle=strhash("idle");
 anim_walk=strhash("walk");
 
 anim_attack=strhash("attack");
 anim_fire=strhash("fire");

 anim_jump=strhash("jump");
 anim_dress=strhash("dress");
 
 anim_die=strhash("die");
 anim_backdie=strhash("backdie");

 anim_grow=strhash("grow");
 anim_shrink=strhash("shrink");

 anim_shell=strhash("shell");
 
 anim_climb=strhash("climb");
}

void snd_load()
{
 snd_coin=audio_load("snd_coin");
 snd_jumpA=audio_load("snd_jump");
 snd_jumpB=audio_load("snd_jumpsuper");
 snd_stomp=audio_load("snd_stomp");
}

void snd_unload()
{
}

int home_enter(_game*gm)
{ 
 actor_reset();

 charanim_cnt=0;
 mario_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"mario");
 
 animid_init();

 snd_load();

#if defined(AUDIO_SUPPORT)
 int musicid=audio_load("ingame");
 if(musicid>=0)
  play_music(gm,musicid,1,0);
 else
  gm->error=-musicid;
#endif

 hero=NULL;

 level_load(gm,1,1,1);

 hero->status|=status_automode;
 hero->autostatus=autostatus_demoplay;

 camera_update(gm);

 img_load(&home_logo,"logo");
 
 gm->scene->status=scene_entering;
 gm->timer=0;gm->maxtimer=GAME_FRAMERATE;

 return 1;
}

int home_leave(_game*gm,_scene*next)
{
 while(charanim_cnt--)
  anim_unload(&charanim[charanim_cnt]);

 img_delete(&home_logo); 
 img_delete(&offscreen); 
 return 1;
}

int home_update(_game*gm)
{
 int i,x,y;

 tilebackground_blit(&canvas,0,0,GAME_WIDTH,GAME_HEIGHT,&level,&offscreen,f2int(cam.x),f2int(cam.y));

 tilemap_blit(&canvas,0,0,GAME_WIDTH,GAME_HEIGHT,&level,&offscreen,f2int(cam.x),f2int(cam.y));
 
 for(i=0;i<actors_count;i++)
  act_draw(gm,pactors[i]);

 y=1;x=gui_drawstring((canvas.w-4*10)/2,y,"TOP ");gui_drawdigits(x,y,topscore,6);

 y+=6+1;
 img_blit(&canvas,(canvas.w-home_logo.w)/2,y,&home_logo,0,0,home_logo.w,home_logo.h,0);

 if(gm->scene->status==scene_playing)
  {
   if(hero) hero->play(gm,hero);
   if((gm->tick&30)<15)
    gui_drawstring((canvas.w-home_logo.w)/2,GAME_HEIGHT-10,"SPACE TO PLAY");
   if(gm->input.key_space)
    {gm->scene->status=scene_leaving;gm->timer=0;gm->maxtimer=GAME_FRAMERATE;}
#ifndef __wasm__
   else
    if(gm->input.key_escape)
     splash_leave(gm,NULL);
#endif
  }
 else
 if(gm->scene->status==scene_entering)
  {
    efx_fade(&canvas,gm->timer,gm->maxtimer,1);
    gm->timer++;
    if(gm->timer==gm->maxtimer)
     {gm->scene->status=scene_playing;gm->timer=0;}
  }
 else
 if(gm->scene->status==scene_leaving)
  {
   act_setanim(hero,anim_idle);
   efx_fade(&canvas,gm->timer,gm->maxtimer,-1);
   gm->timer++;
   if(gm->timer==gm->maxtimer)
    splash_leave(gm,&ingame);
  }

 return 1;
}

_scene home={
                home_enter,
                home_leave,
                home_update
               };

// ************************************************************