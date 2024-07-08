#include "emulator.h"
#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "emulator.h"
#include "ppu.h"

struct emulator *emu_build(char *filename) {
  struct emulator *emu = (struct emulator *)malloc(sizeof(struct emulator));
  emu->cpu = cpu_build();
  emu->ppu = ppu_build();
  emu->cart = cart_build(filename);
  emu->bus = bus_build(emu);
  if (emu->cpu == NULL) {
    printf("ERROR BUILDING CPU\n");
    return NULL;
  }
  if (emu->ppu == NULL) {
    printf("ERROR BUILDING PPU\n");
    return NULL;
  }
  if (emu->cart == NULL) {
    printf("ERROR BUILDING CARTRIDGE\n");
    return NULL;
  }
  cpu_reset(emu->cpu);
  return emu;
}
