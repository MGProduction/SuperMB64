//
// Copyright (c) 2023, Marco Giorgini [ @marcogiorgini ]
// Distributed under the MIT License
//
// splash screen code for SuperMB64
//

// ************************************************************
// SPLASH SCREEN CODE
// ************************************************************

extern _scene ingame;
extern _scene home;

#define SPLASH_IMAGE      "splash"
_img logo;

// ************************************************************

int splash_enter(_game*gm)
{
#if defined(AUDIO_SUPPORT)
 int musicid=audio_load("ingame");
 if(musicid>=0)
  play_music(gm,audio_get(musicid));
 else
  gm->error=-musicid;
#endif
 img_load(&logo,SPLASH_IMAGE);
 gm->scene->status=scene_entering;
 gm->timer=0;gm->maxtimer=GAME_FRAMERATE;
 return 1;
}

int splash_leave(_game*gm,_scene*next)
{
 img_delete(&logo);
 return game_setscene(gm,next);
}

int splash_update(_game*gm)
{
 int centerx=(canvas.w-logo.w)/2,centery=(canvas.h-logo.h)/2;
 img_blit(&canvas,centerx,centery,&logo,0,0,logo.w,logo.h,0);
 if(gm->scene->status==scene_entering)
  {
    efx_fade(&canvas,gm->timer,gm->maxtimer,1);
    gm->timer++;
    if(gm->timer==gm->maxtimer)
     {gm->scene->status=scene_playing;gm->timer=0;gm->maxtimer=GAME_FRAMERATE*2;}
  }
 else
 if(gm->scene->status==scene_playing)
  {
   gm->timer++;
   if(gm->timer==gm->maxtimer)
     {gm->scene->status=scene_leaving;gm->timer=0;gm->maxtimer=GAME_FRAMERATE;}
  }
 else
 if(gm->scene->status==scene_leaving)
  {
   efx_fade(&canvas,gm->timer,gm->maxtimer,-1);
   gm->timer++;
   if(gm->timer==gm->maxtimer)
    splash_leave(gm,&home);
  }
 return 1;
}

// ************************************************************

_scene splash={
                splash_enter,
                splash_leave,
                splash_update
               };

// ************************************************************