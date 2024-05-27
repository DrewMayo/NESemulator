#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
} nsdl_manager;

nsdl_manager* start_window();

bool destroy_window();

bool renderLoop(nsdl_manager *manager);
