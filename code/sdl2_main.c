#include <SDL2/SDL.h>

#define JAI_UTILS_IMPLEMENTATION
#define JAI_ALLOCATORS_IMPLEMENTATION
#include "sdl2_main.h"

global b32 running = true; 

int 
main(void) {
  SDL_Init(SDL_INIT_VIDEO);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
  SDL_Window* window = SDL_CreateWindow("JED",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        640, 480, 0);
#pragma clang diagnostic pop
  SDL_Event event;

  while (running) {
    SDL_WaitEvent(&event);
    switch(event.type) {
      case SDL_QUIT: {
       running = false;
      } break;
    }
  } 
  SDL_Quit();

  return 0;
}
