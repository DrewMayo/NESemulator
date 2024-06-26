#include "ppu.h"

struct ppu_2C02 ppu_build(uint8_t *cpu_memory);

void ppu_run(uint8_t *cpu_memory) {
  struct ppu_2C02 ppu = ppu_build(cpu_memory);
}

struct ppu_2C02 ppu_build(uint8_t *cpu_memory) {
  struct ppu_2C02 ppu;
  ppu.PPUCTRL = cpu_memory[0x2000];
  ppu.PPUMASK = cpu_memory[0x2001];
  ppu.PPUSTATUS = cpu_memory[0x2002];
  ppu.OAMADDR = cpu_memory[0x2003];
  ppu.OAMDATA = cpu_memory[0x2004];
  ppu.PPUSCROLL = cpu_memory[0x2005];
  ppu.PPUADDR = cpu_memory[0x2006];
  ppu.PPUDATA = cpu_memory[0x2007];
  ppu.OAMDMA = cpu_memory[0x4014];
  return ppu;
}
