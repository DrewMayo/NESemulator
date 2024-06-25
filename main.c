#include "nessdl.h"
#include "test.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

void read_rom(emulator_t *emu, uint16_t memLoc, char *romName);

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Usage: ./nes [<romName>]\n");
    return 1;
  }
  // init emulator
  emulator_t *emu = malloc(sizeof(emulator_t));
  for (int i = 0; i < 0xFFFF; i++) {
    emu->memory[i] = 0;
  }
  emu->cpu.AC = 0;
  emu->cpu.X = 0;
  emu->cpu.Y = 0;
  emu->cpu.SP = 0;
  emu->cpu.SR.Negative = false;
  emu->cpu.SR.Overflow = false;
  emu->cpu.SR.Break = true;
  emu->cpu.SR.Decimal = false;
  emu->cpu.SR.Interrupt = false;
  emu->cpu.SR.Zero = false;
  emu->cpu.SR.Carry = false;
  emu->cpu.PC = 0xC000;
  emu->cpu.SP = 0xFD;
  expand_SR(&emu->cpu, 0x24);
  emu->cpu.cycles = 7;
  read_rom(emu, 0x8000, argv[1]);
  read_rom(emu, 0xC000, argv[1]);
  int counter = 0;
  clock_t start = clock();
  while (emu->cpu.cycles <= 1789773) {
    run(&emu->cpu, emu->memory);
    counter++;
    // SDL_Delay(100);
  }

  clock_t end = clock();
  printf("\ntime: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
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
      // printf("ERROR: ROM TOO BIG")
      fclose(fp);
      return;
    }
  }
  fclose(fp);
}
