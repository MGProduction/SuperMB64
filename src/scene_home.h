// ************************************************************
// TITLE SCREEN CODE
// ************************************************************

_img      home_logo;

int home_enter(_game*gm)
{ 
 actor_reset();

 charanim_cnt=0;
 mario_anim=charanim_cnt;anim_load(&charanim[charanim_cnt++],"mario");
 anim_idle=strhash("idle");

 level_load(1,1,1);

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
  if((gm->tick&30)<15)
   gui_drawstring((canvas.w-home_logo.w)/2,GAME_HEIGHT-10,"SPACE TO PLAY");
  if(gm->input.key_space)
   {gm->scene->status=scene_leaving;gm->timer=0;gm->maxtimer=GAME_FRAMERATE;}
  }
 else
 if(gm->scene->status==scene_entering)
  {
    efx_fade(&canvas,gm->timer,gm->maxtimer,1);
    gm->timer++;
    if(gm->timer==gm->maxtimer)
     {gm->scene->status=scene_playing;}
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

_scene home={
                home_enter,
                home_leave,
                home_update
               };

// ************************************************************