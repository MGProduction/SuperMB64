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

 //data_t          *data;
 int              frame_counter;
#if defined(AUDIO_SUPPORT)
 sound_t          music;
 sound_channel_t sounds[ 16 ];
 sound_context_t sound_context;
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
sound_t sound( _game* game, int soundfile_id/*const char* filename*/ ) {
    sound_t snd;
    snd.sample_pairs = NULL;//load_samples( game->data, filename, &snd.sample_pairs_count );
    return snd;
}


void play_music( _game* game, sound_t music ) {
    game->music = music;
}


void queue_sound( _game* game, sound_t sound, float volume, float delay ) {
    int index = -1;
    int age = game->frame_counter + 1;
    for( int i = 0; i < sizeof( game->sounds ) / sizeof( *game->sounds ); ++i ) {
        if( game->sounds[ i ].age < age ) {
            index = i;
            age = game->sounds[ i ].age;
        }
    }
    sound_channel_t* channel = &game->sounds[ index ];
    channel->age = game->frame_counter;
    channel->current_position = -(int)( 44100.0f * delay );
    channel->sample_pairs = sound.sample_pairs;
    channel->sample_pairs_count = sound.sample_pairs_count;
    channel->volume = volume;
}


void play_sound( _game* game, sound_t sound, float volume ) {
    queue_sound( game, sound, volume, 0.0f );
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
 int i;
 #ifndef __wasm__
 thread_mutex_lock( &game->sound_context.mutex );
 #endif
 if( game->sound_context.music_sample_pairs != game->music.sample_pairs )
 {
    game->sound_context.music_sample_pairs = game->music.sample_pairs;
    game->sound_context.music_sample_pairs_count = game->music.sample_pairs_count;
    game->sound_context.music_current_position = 0;
 }
 for( i = 0; i < sizeof( game->sound_context.sounds ) / sizeof( *game->sound_context.sounds ); ++i ) {
    if( game->sounds[ i ].age > game->sound_context.sounds[ i ].age ) {
        game->sound_context.sounds[ i ] = game->sounds[ i ];
    }
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

