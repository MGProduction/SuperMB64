// ****************************************************************************
//
// SuperMB64
// 
// [Yet Another Super Mario Bros clone]
// with 64x64 (or 64x64 or 96x64) pixel resolution
// done for educational purpouses 
//
// Marco Giorgini [ @marcogiorgini ]
//
// ****************************************************************************
//
// MIT License
// 
// Copyright (c) 2023 Marco Giorgini
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// ****************************************************************************
//
//  Graphic (redrawn by me) is heavily based on original Super Mario Bros NES 
//  game by Nintendo
//  Game level is a clone of World 1x1 of original game
//
//  This project created with the intent of writing some blog posts about it
//
// This C game uses
//
// - Mattias Gustavsson game framework
// - stb_vorbis / minimp3_ex for audio
// - QOI decoder
//
// ************************************************************

#define GAMEPAD_SUPPORT
#define AUDIO_SUPPORT
//#define USE_OGG
#define USE_MP3

// ************************************************************
// INCLUDES
// ************************************************************

#define _CRT_NONSTDC_NO_DEPRECATE 
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <string.h>
#include <math.h>

// ************************************************************
// INCLUDES [GAME RESOURCES]
// ************************************************************

#include "mg/minilib.h"
#include "resources.h"

// ************************************************************
// INCLUDES [FRAMEWORK CODE]
// ************************************************************

#include "gustavsson/app.h"
#ifndef __wasm__
#include "gustavsson/thread.h"
#endif
#define FRAMETIMER_IMPLEMENTATION
#include "gustavsson/frametimer.h"

// ************************************************************
// INCLUDES [QOI IMAGE DECODING SUPPORT]
// ************************************************************

#if defined(_DEBUG)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define _GIF_SUPPORT
#endif

#define QOI_IMPLEMENTATION
#include "libs/qoi.h"

#if defined(_GIF_SUPPORT)
#define MSF_GIF_IMPL
#include "libs/msf_gif.h"
#endif

// ************************************************************
// INCLUDES [AUDIO DECODING]
// ************************************************************
#if defined(AUDIO_SUPPORT)
#if defined(USE_QOA)
#define QOA_IMPLEMENTATION
#include "libs/qoa.h"
#endif
#if defined(USE_MINIAUDIO)
#define MINIAUDIO_IMPLEMENTATION
//#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_NO_FLAC
#define MA_NO_MP3
#define MA_NO_ENCODING
#include "libs/miniaudio.h"
#endif
#if defined(USE_OGG)
#include "stb/stb_vorbis.c"
#endif
#if defined(USE_MP3)
#define MINIMP3_IMPLEMENTATION
#define MINIMP3_NO_SIMD
#include "libs/minimp3_ex.h"
#endif
#endif

// ************************************************************
// INCLUDES [GAMEPAD]
// ************************************************************

#if defined(GAMEPAD_SUPPORT)
#ifdef _WIN32
#define GAMEPAD_IMPLEMENTATION
#include "gustavsson/gamepad.h"
#else
    typedef struct gamepad_state_t {
	    uint16_t buttons;
	    uint8_t trigger_left;
	    uint8_t trigger_right;
	    int16_t stick_left_x;
	    int16_t stick_left_y;
	    int16_t stick_right_x;
	    int16_t stick_right_y;
	} gamepad_state_t;


    typedef enum gamepad_button_t {
	    GAMEPAD_DPAD_UP = 0x0001,
	    GAMEPAD_DPAD_DOWN = 0x0002,
	    GAMEPAD_DPAD_LEFT = 0x0004,
	    GAMEPAD_DPAD_RIGHT = 0x0008,
	    GAMEPAD_START = 0x0010,
	    GAMEPAD_BACK = 0x0020,
	    GAMEPAD_STICK_LEFT = 0x0040,
	    GAMEPAD_STICK_RIGHT = 0x0080,
	    GAMEPAD_SHOULDER_LEFT = 0x0100,
	    GAMEPAD_SHOULDER_RIGHT = 0x0200,
	    GAMEPAD_A = 0x1000,
	    GAMEPAD_B = 0x2000,
	    GAMEPAD_X = 0x4000,
	    GAMEPAD_Y = 0x8000,
	} gamepad_button_t;
#endif
#endif

// ************************************************************

// ************************************************************
// INCLUDES [STRING / IMAGES / AUDIO]
// ************************************************************

#define MINILIB_IMPLEMENTATION
#include "mg/minilib.h"
#if defined(AUDIO_SUPPORT)
#include "mg/audio.h"
#endif
#define MGIMG_IMPLEMENTATION
#include "mg/img.h"

// ************************************************************
// INCLUDES [GAME GENERIC CODE IS THERE]
// ************************************************************

int GAME_WIDTH,GAME_HEIGHT,GAME_FRAMERATE;

#include "scenes.h"
#include "game.h"

// ************************************************************
// INCLUDES [GAME SCENES ARE THERE]
// ************************************************************

#include "scene_splash.h"
#include "scene_home.h"
#include "scene_ingame.h"

// ************************************************************
// startup code
// ************************************************************

_anim font;

#if defined(GAMEPAD_SUPPORT)
#ifdef _WIN32
gamepad_t* gamepad;
#endif
#endif

void game_start(_game*game)
{
 anim_load(&font,"font");
#if defined(GAMEPAD_SUPPORT)
 #ifdef _WIN32
 gamepad = gamepad_create( NULL );
 #endif
#endif

#if defined(_DEBUG)
 game->scene=&home;
#else
 game->scene=&splash;
#endif

 game->scene->enter(game);
}

void game_end(_game*game)
{
#if defined(GAMEPAD_SUPPORT)
 #ifdef _WIN32
 gamepad_destroy( gamepad );
 #endif
#endif
 anim_unload(&font);
 if(game->scene)
  game->scene->leave(game,NULL);
}

int gui_drawstring(int x,int y,const char*sz)
{
 int w=3,h=6;
 if(x==-1)
 {
  int w=0,i=0;
  while(sz[i])
   {
    char ch=sz[i++];
    int  v=0;
    if(ch==' ')
     w+=2;
    else
     {
      if(isbetween(ch,'A','Z'))
       v=ch-'A';
      else
       if(isbetween(ch,'0','9'))
       v=28+(ch-'0');
      if(ch=='I')
       w+=2;
      else
       w+=4;
     }
   }
  x=(GAME_WIDTH-w)/2;
 }
 if(y==-1)
  {
   int h=6;
   y=(GAME_HEIGHT-h)/2;
  }
 while(*sz)
  {
   char ch=*sz++;
   int  v=0;
   if(ch==' ')
    x+=2;
   else
    {
     if(isbetween(ch,'A','Z'))
      v=ch-'A';
     else
      if(isbetween(ch,'0','9'))
      v=28+(ch-'0');
     img_blit(&canvas,x,y,&font.atlas,v*w,0,w,h,0);
     if(ch=='I')
      x+=2;
     else
      x+=4;
    }
  }
 return x;
}

int gui_drawdigits(int x,int y,int val,int digitcnt)
{
 int digit=28,w=3,h=6,ox;
 ox=x+4*digitcnt;
 x+=4*(digitcnt-1);
 while(digitcnt--)
  {
   img_blit(&canvas,x,y,&font.atlas,(digit+(val%10))*w,0,w,h,0);
   val/=10;x-=4;
  }
 return ox;
}

// ************************************************************
// framework-related code - based on Mattias Gustavsson sample projects
// ************************************************************

int game_getinput(_game*gm)
{
 int i;
 app_input_t appinp = app_input( gm->app );
 gm->input.key_escape = false; // single shot keys
 for(i = 0; i < appinp.count; ++i )
  {
      app_input_event_t* event = &appinp.events[ i ];
      if( event->type == APP_INPUT_KEY_DOWN ) {
       if( event->data.key == APP_KEY_LEFT ) gm->input.key_left = true;
          if( event->data.key == APP_KEY_RIGHT ) gm->input.key_right = true;
          if( event->data.key == APP_KEY_UP ) gm->input.key_up = true;
          if( event->data.key == APP_KEY_DOWN ) gm->input.key_down = true;
          if( event->data.key == APP_KEY_RETURN ) gm->input.key_enter = true;
          if( event->data.key == APP_KEY_SPACE ) gm->input.key_space = true;
          if( event->data.key == APP_KEY_CONTROL ) gm->input.key_control = true;          
      } else if( event->type == APP_INPUT_KEY_UP ) {
          if( event->data.key == APP_KEY_LEFT ) gm->input.key_left = false;
          if( event->data.key == APP_KEY_RIGHT ) gm->input.key_right = false;
          if( event->data.key == APP_KEY_UP ) gm->input.key_up = false;
          if( event->data.key == APP_KEY_DOWN ) gm->input.key_down = false;
          if( event->data.key == APP_KEY_RETURN ) gm->input.key_enter = false;
          if( event->data.key == APP_KEY_SPACE ) gm->input.key_space = false;
          if( event->data.key == APP_KEY_CONTROL ) gm->input.key_control = false;
          if( event->data.key == APP_KEY_ESCAPE ) gm->input.key_escape = true;
          if( event->data.key == APP_KEY_F11 ) {
              gm->fullscreen = !gm->fullscreen;
              app_screenmode( gm->app, gm->fullscreen ? APP_SCREENMODE_FULLSCREEN : APP_SCREENMODE_WINDOW );
          }
#if defined(_GIF_SUPPORT)
          if( event->data.key == APP_KEY_F10 ) {
           stbi_write_png("bin/SuperMB64_screenshot.png",canvas.w,canvas.h,4,canvas.col,0);
          }
          if( event->data.key == APP_KEY_F5 ) {
           gif_start();
          }
          if( event->data.key == APP_KEY_F6 ) {
           gif_stop("bin/SuperMB64_video.gif");
          }
#endif
      }
  }

#if defined(GAMEPAD_SUPPORT)
 gamepad_state_t padstate = { 0 };
 #ifdef _WIN32
 gamepad_read( gamepad, 0, &padstate );
 #endif
 if( padstate.buttons & GAMEPAD_A )
  gm->input.key_control = true;
#endif

 return 1;
}

int app_proc( app_t* app, void* user_data )
{
    (void) user_data;            
    _game   game;
    APP_U32 dummy = 0;
    memset(&game,0,sizeof(game));

    GAME_WIDTH=res_getvalue("#width",64);
    GAME_HEIGHT=res_getvalue("#height",64);
    GAME_FRAMERATE=res_getvalue("#framerate",30);

    game.app=app;
    //memset( canvas, 0xC0, sizeof( canvas ) );
    app_screenmode( app, APP_SCREENMODE_FULLSCREEN );

    app_displays_t displays = app_displays( app );
    
     if( displays.count > 0 ) {
        // find main display
        int disp = 0;
        for( int i = 0; i < displays.count; ++i ) {
            if( displays.displays[ i ].x == 0 && displays.displays[ i ].y == 0 ) {
                disp = i;
                break;
            }
        }
        // calculate aspect locked width/height
        int scrwidth = displays.displays[ disp ].width;
        int scrheight = displays.displays[ disp ].height;
        int aspect_width = (int)( ( scrheight * GAME_WIDTH ) / GAME_HEIGHT );
        int aspect_height = (int)( ( scrwidth * GAME_HEIGHT ) / GAME_WIDTH );
        int target_width, target_height;
        if( aspect_height <= scrheight ) {
            target_width = scrwidth;
            target_height = aspect_height;
        } else {
            target_width = aspect_width;
            target_height = scrheight;
        }
        // set window size and position
        int x = displays.displays[ disp ].x + ( displays.displays[ disp ].width - target_width ) / 2;
        int y = displays.displays[ disp ].y + ( displays.displays[ disp ].height - target_height ) / 2;
        int w = target_width;
        int h = target_height;
        app_window_pos( app, x, y );
        app_window_size( app, w, h );
    }

    #ifndef __wasm__
#if defined(_DEBUG)
        game.fullscreen = false;
        app_window_size( game.app, GAME_WIDTH*8, GAME_HEIGHT*8 );
#else
        game.fullscreen = true;
#endif
    #else
        game.fullscreen = false;
    #endif
    app_interpolation( app, APP_INTERPOLATION_NONE );
    app_screenmode( app, game.fullscreen ? APP_SCREENMODE_FULLSCREEN : APP_SCREENMODE_WINDOW );
    app_title( app, res_getstring("#title","miniMG") );

     // No mouse cursor    
    app_pointer( app, 1, 1, &dummy, 0, 0 );

    frametimer_t* frametimer = frametimer_create( NULL );
    frametimer_lock_rate( frametimer, GAME_FRAMERATE );
#if defined(AUDIO_SUPPORT)
    audio_new(&game);
#if defined(USE_MINIAUDIO)
    ma_result result;
    ma_engine engine;
    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        return -1;
    }
#endif
#endif

    img_new(&canvas,GAME_WIDTH,GAME_HEIGHT);

    game_start(&game);

    while( app_yield( app ) != APP_STATE_EXIT_REQUESTED )
    {
        frametimer_update( frametimer );
        game_getinput(&game);
#if defined(AUDIO_SUPPORT)
        audio_render(&game);
#endif
        game.tick++;

        if(game.scene==NULL)
         break;
        else
         game.scene->update(&game); 

#if defined(_GIF_SUPPORT)
        gif_addframe((byte*)canvas.col);
#endif

        app_present( app, canvas.col, GAME_WIDTH, GAME_HEIGHT, 0xffffff, 0x000000 );
    }

    game_end(&game);    
    
    img_delete(&canvas);

    frametimer_destroy( frametimer );
#if defined(AUDIO_SUPPORT)
#if defined(USE_MINIAUDIO)
    ma_engine_uninit(&engine);
#endif
    audio_delete(&game);
#endif

    return 0;
}


int main( int argc, char** argv ) {
    (void) argc, (void ) argv;
    return app_run( app_proc, NULL, NULL, NULL, NULL );
}


// pass-through so the program will build with either /SUBSYSTEM:WINDOWS or /SUBSYSTEM:CONSOLE
#if defined( _WIN32 ) && !defined( __TINYC__ )
    #ifdef __cplusplus 
        extern "C" int __stdcall WinMain( struct HINSTANCE__*, struct HINSTANCE__*, char*, int ) { 
            return main( __argc, __argv ); 
        }
    #else
        struct HINSTANCE__;
        int __stdcall WinMain( struct HINSTANCE__* a, struct HINSTANCE__* b, char* c, int d ) { 
            (void) a, b, c, d; return main( __argc, __argv ); 
        }
    #endif
#endif


#define APP_IMPLEMENTATION
#ifdef _WIN32 
    #define APP_WINDOWS
#elif __wasm__
    #define APP_WASM
#else 
    #define APP_SDL
#endif
#define APP_LOG( ctx, level, message ) 
#include "gustavsson//app.h"

#ifndef __wasm__
#define THREAD_IMPLEMENTATION
#include "gustavsson//thread.h"
#endif