#include "cartridge.h"
#include "emulator.h"
#include "ppu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc == 1 || argc > 2) {
    printf("Usage: ./nes [<romName>]\n");
    return -1;
  }
  // init emulator
  struct emulator *emu = emu_build(argv[1]);
  if (emu == NULL) {
    return -2;
  }
  // running
  //  emu->cpu->PC = 0xC000;
  bool quit = false;
  SDL_Event e;
  while (!quit) {
    int cycles = cpu_run(emu->cpu);
    for (int i = 0; i < cycles; i++) {
      ppu_tick(emu->ppu);
    }
    // YOU SHOULD PROFILE WHY THIS IS SO SLOW
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }
  }
  destroy_window();
  cart_delete(emu->cart);
  free(emu);
  return 0;
}
