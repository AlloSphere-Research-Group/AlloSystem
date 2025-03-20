// Since SDL can be used as a backend for multiple subsystems, this file should be included to define some standard helper macros.

// Prevents SDL from taking control of the main() function
#define SDL_MAIN_HANDLED

#ifndef AL_SDL_VERSION
	#define AL_SDL_VERSION 2
#endif

#if AL_SDL_VERSION == 3
	#define USING_SDL3
	#include <SDL3/SDL.h>
#elif AL_SDL_VERSION == 2
	#define USING_SDL2
	#include <SDL2/SDL.h>
#else
	#error Unsupported version of SDL
#endif

#ifdef USING_SDL3
	#define SDL_INIT_ERROR(res) !res
#elif defined USING_SDL2
	#define SDL_INIT_ERROR(res) res < 0
#endif