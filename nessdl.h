#pragma once
#ifndef NESSDL_H
#define NESSDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>

#define WINDOW_WIDTH 256
#define WINDOW_HEIGHT 240

struct nsdl_manager {
  SDL_Window *window;
  SDL_Renderer *renderer;
};

struct nsdl_manager *start_window();

bool destroy_window();

bool renderLoop(struct nsdl_manager *manager);

#endif // !NESSDL_H
