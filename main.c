#include "nessdl.h"
#include "test.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdio.h>

void read_rom(emulator_t *emu, uint16_t memLoc, char *romName);

int main(int argc, char *argv[]) {
  (void)argv;
  if (argc == 1) {
    printf("Usage: ./nes [<romName>]\n");
    return 1;
  }
  // init emulator
  emulator_t *emu = malloc(sizeof(emulator_t));
  emu->cpu.PC = 0xC000;
  emu->cpu.SP = 0xFD;
  expand_SR(&emu->cpu, 0x24);
  emu->cpu.cycles = 7;
  read_rom(emu, 0x8000, argv[1]);
  read_rom(emu, 0xC000, argv[1]);
  while (emu->cpu.cycles <= 26554) {
    run(&emu->cpu, emu->memory);
    //    SDL_Delay(100);
  }
  // run_unit_tests();
  /*
    // SDL
    nsdl_manager *manager = start_window();
    if (renderLoop(manager)) {
      destroy_window();
    }
  */
  free(emu);
  return 0;
}

void read_rom(emulator_t *emu, uint16_t memLoc, char *romName) {
  FILE *fp = fopen(romName, "rb");
  int counter = 0;
  fseek(fp, 16, SEEK_SET);
  while (!feof(fp)) {
    fread(&emu->memory[memLoc + counter], sizeof(uint8_t), 1, fp);
    counter++;
    if ((uint32_t)memLoc + (uint32_t)counter > 0xFFFF) {
      // printf("ERROR: ROM TOO BIG");
      return;
    }
  }
}
