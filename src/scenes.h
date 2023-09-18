typedef int (*scene_enter)(struct __game*gm);
typedef int (*scene_leave)(struct __game*gm,struct __scene*next);
typedef int (*scene_update)(struct __game*gm);

enum scene_status{
 scene_undef,
 scene_entering,
 scene_playing,
 scene_leaving,
 scene_levelcompleted
};

typedef struct __scene{
 scene_enter  enter;
 scene_leave  leave;
 scene_update update;
 scene_status status;
}_scene;

typedef struct{
 bool key_left,key_right,key_up,key_down;
 bool key_enter,key_space,key_control;
 bool key_escape;
}_input;

typedef struct __game{
 app_t           *app;
 bool             fullscreen;
 _scene          *scene;
 _input           input;
 dword            timer,maxtimer,tick;

 int              error;

#if defined(AUDIO_SUPPORT)
 _sound_channel   music;
 _sound_channel   queuedsounds[MAX_CONTEXTSOUNDS];
 int              queuedlen,emittedsounds;
 _sound_context   sound_context;
 int              soundid;
#endif
}_game;

int game_setscene(_game*gm,_scene*next)
{
 gm->scene=next;
 if(gm->scene)
  {
   gm->scene->enter(gm);
   return 1;
  }
 else
  return 0;
}

#if defined(AUDIO_SUPPORT)

void play_music( _game* game, int musicid, float volume, float delay )
{
    _sound*music=audio_get(musicid);
    game->music.age = game->tick;
    game->music.current_position = -(int)( 44100.0f * delay );
    game->music.volume = volume;
    game->music.sound=music;
    game->music.id=game->soundid++;
}


void queue_sound( _game* game, _sound*sound, float volume, float delay )
{
 if(sound)
  if(game->queuedlen<sizeof( game->queuedsounds ) / sizeof( *game->queuedsounds ))
   {
    _sound_channel* channel = &game->queuedsounds[ game->queuedlen++ ];
    channel->age = game->tick;
    channel->current_position = -(int)( 44100.0f * delay );
    channel->volume = volume;
    channel->sound=sound;   
    channel->id=game->soundid++;
   }
}


void play_sound( _game* game, int soundid, float volume )
{
    queue_sound( game, audio_get(soundid), volume, 0.0f );
}

void audio_new(_game*gm)
{ 
 memset( &gm->sound_context, 0, sizeof( gm->sound_context ) );
 #ifndef __wasm__
 thread_mutex_init( &gm->sound_context.mutex );
 #endif
 app_sound( gm->app, 735 * 5, sound_callback,  &gm->sound_context );
}

void audio_render(_game* game)
{
 #ifndef __wasm__
 thread_mutex_lock( &game->sound_context.mutex );
 #endif
 if( game->sound_context.music.sound != game->music.sound )
  game->sound_context.music=game->music;
 if(game->queuedlen)
  {
   while(game->queuedlen--)
    {
     int j,dj=-1,distantage=game->tick,hm=sizeof( game->sound_context.sounds ) / sizeof( *game->sound_context.sounds );
     for(j=0;j<hm;j++)
      if(game->sound_context.sounds[j].sound==NULL)
       {
        game->sound_context.sounds[j]=game->queuedsounds[game->queuedlen];
        break;
       }
      else
       if(game->sound_context.sounds[j].age<distantage)
        {distantage=game->sound_context.sounds[j].age;dj=j;}
     if(j==hm)
      if(dj!=-1)
       game->sound_context.sounds[dj]=game->queuedsounds[game->queuedlen];
    } 
   game->emittedsounds++;
   game->queuedlen=0;
  }
 #ifndef __wasm__
 thread_mutex_unlock( &game->sound_context.mutex );
 #endif
}

void audio_delete(_game* gm)
{
 app_sound( gm->app, 0, NULL, NULL );
 #ifndef __wasm__
 thread_mutex_term( &gm->sound_context.mutex );
 #endif
}
#endif

