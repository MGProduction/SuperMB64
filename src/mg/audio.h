#define MAX_SOUNDS 32

typedef struct sound_channel_t {
    int age;
    short* sample_pairs;
    int sample_pairs_count;
    int current_position;
    float volume;
} sound_channel_t;

typedef struct sound_t {
    short* sample_pairs;
    int sample_pairs_count;
} sound_t;

typedef struct sound_context_t {
    #ifndef __wasm__
        thread_mutex_t mutex;
    #endif
    short* music_sample_pairs;
    int music_sample_pairs_count;
    int music_current_position;
    sound_channel_t sounds[ 16 ];
    int streamed;
} sound_context_t;


void sound_callback( APP_S16* sample_pairs, int sample_pairs_count, void* user_data ) {
    sound_context_t* context = (sound_context_t*) user_data;

    #ifndef __wasm__
        thread_mutex_lock( &context->mutex );
    #endif

    for( int i = 0; i < sample_pairs_count; ++i ) {
        int left = 0;
        int right = 0;
        if( context->music_sample_pairs ) {
            context->streamed++;
            left = context->music_sample_pairs[ context->music_current_position * 2 + 0 ];
            right = context->music_sample_pairs[ context->music_current_position * 2 + 1 ];
            context->music_current_position++;
            if( context->music_current_position >= context->music_sample_pairs_count ) {
                context->music_current_position = 0;
            }
        }
        for( int j = 0; j < sizeof( context->sounds ) / sizeof( *context->sounds ); ++j ) {
            sound_channel_t* sound = &context->sounds[ j ];
            if( sound->current_position < sound->sample_pairs_count ) {
                if( sound->current_position >= 0 ) {
                    int sl = sound->sample_pairs[ sound->current_position * 2 + 0 ];
                    int sr = sound->sample_pairs[ sound->current_position * 2 + 1 ];
                    sound->current_position++;
                    left += (int)( sl * sound->volume );
                    right += (int)( sr * sound->volume );
                } else {
                    sound->current_position++;
                }
            }
        }
        left = left > 32767 ? 32767 : left < -32767 ? -32767 : left;
        right = right > 32767 ? 32767 : right < -32767 ? -32767 : right;
        sample_pairs[ i * 2 + 0 ] = (short)left;
        sample_pairs[ i * 2 + 1 ] = (short)right;
    }

    #ifndef __wasm__
        thread_mutex_unlock( &context->mutex );
    #endif
}

typedef struct{
    char     name[32];
    sound_t  snd;
} sound_file_t;

sound_file_t sounds[MAX_SOUNDS];
uint8_t      sounds_cnt;

sound_t audio_get(int id)
{
 return sounds[id].snd;
}

int audio_load(const char*name)
{
 uint32_t size;
 int      i;
 uint8_t *mem;
 for(i=0;i<MAX_SOUNDS;i++)
  if(strcmp(sounds[i].name,name)==0)
   return i; 
 mem=res_get(name,&size);
 if(mem)
  {
   int channels = 0;
   int sample_rate = 0;
   short* output = NULL;   
#if defined(USE_MP3)
   mp3dec_t mp3d;
   mp3dec_file_info_t info;
   if (mp3dec_load_buf(&mp3d, mem,size, &info, NULL, NULL))
   {
       /* error */
    return -1;
   }   
   else
   {    
     for(i=0;i<MAX_SOUNDS;i++)
      if(*sounds[i].name==0)
       {
         strcpy(sounds[i].name,name);
         sounds[i].snd.sample_pairs = info.buffer;
         sounds[i].snd.sample_pairs_count = info.samples;
         return i;
       }
     return -2;
   }
#endif
#if defined(USE_OGG)
   int samples = stb_vorbis_decode_memory( mem, size, &channels, &sample_rate, &output );
   if( samples > 0 && output )
    {    
     for(i=0;i<MAX_SOUNDS;i++)
      if(*sounds[i].name==0)
       {
         strcpy(sounds[i].name,name);
         sounds[i].snd.sample_pairs = output;
         sounds[i].snd.sample_pairs_count = samples;
         return i;
       }
     return -2;
    }   
   else
    if(samples==-1)
     {
      int        error;
      stb_vorbis*vorbos=stb_vorbis_open_memory(mem,size,&error,NULL);
      return -10-error;
     }
     else
      if(samples==-2)
       return -4;
      else
       return -5;
#endif
  }
 return -1;
}

