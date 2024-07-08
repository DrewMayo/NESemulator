#include "nessdl.h"

//This starts the window,creates the manager, and creates 
//the renderer to use.
struct nsdl_manager* start_window() {

  //Initialize the SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) == -1) {
    printf("Could not init SDL: %s.\n", SDL_GetError());
    return NULL;
  }
  //allocate the manager
  struct nsdl_manager* manager = malloc(sizeof(struct nsdl_manager));

  //create the window and check for errors
  manager->window = SDL_CreateWindow(
    "NES Emulator",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    SDL_WINDOW_RESIZABLE
  );

  if (manager->window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create the window: %s\n", SDL_GetError());
    return NULL;
  }

  //create renderer and check for errors
  manager->renderer = SDL_CreateRenderer(manager->window, -1, 0);
  if (manager->renderer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create the renderer: %s\n", SDL_GetError());
    return NULL;
  }
  return manager;
}

//Destroys the created window by SDL

bool destroy_window() {
  SDL_Quit();
  return true;
}

/*
bool renderLoop(struct nsdl_manager *manager) {
  bool quit = false;
  SDL_Event e;
  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }
    SDL_RenderClear(manager->renderer);
    SDL_RenderPresent(manager->renderer);
  }
  return true;
} */
