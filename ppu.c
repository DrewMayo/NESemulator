#include "ppu.h"
#include "bitmask.h"
#include "cartridge.h"
#include "cpu.h"
#include "emulator.h"

#define PPU_WIDTH 341
#define PPU_HEIGHT 262

void ppu_read_cpu(struct emulator *const emu);
void ppu_write_cpu(const struct emulator *const emu);
void ppu_run(struct emulator *const emu);

void ppu_tick(struct emulator *const emu) {
  if (emu->ppu->cycles % 341 == 0) {
    ppu_run(emu);
    emu->ppu->scanline++;
  }
  emu->ppu->cycles++;
}

void ppu_run(struct emulator *const emu) {
  ppu_read_cpu(emu);
  if (emu->ppu->scanline == 241) {
    emu->ppu->PPUSTATUS |= BIT7;
    emu->cpu->interrupt_state = INONMASKABLE;
  } else if (emu->ppu->scanline == 261) {
    emu->ppu->PPUSTATUS &= 0b01111111;
  } else if (emu->ppu->scanline > 320) {
    emu->ppu->scanline = -1;
  } else {
  }
  ppu_write_cpu(emu);
}
struct ppu_2C02 *ppu_build() {
  struct ppu_2C02 *ppu = (struct ppu_2C02 *)malloc(sizeof(struct ppu_2C02));
  for (int i = 0; i < 0xFFFF; i++) {
    ppu->memory[i] = 0;
  }
  ppu->scanline = -1;
  ppu->cycles = 0;
  return ppu;
}

void ppu_read_cpu(struct emulator *const emu) {
  emu->ppu->PPUCTRL = emu->cpu->memory[0x2000];
  emu->ppu->PPUMASK = emu->cpu->memory[0x2001];
  emu->ppu->PPUSTATUS = emu->cpu->memory[0x2002];
  emu->ppu->OAMADDR = emu->cpu->memory[0x2003];
  emu->ppu->OAMDATA = emu->cpu->memory[0x2004];
  emu->ppu->PPUSCROLL = emu->cpu->memory[0x2005];
  emu->ppu->PPUADDR = emu->cpu->memory[0x2006];
  emu->ppu->PPUDATA = emu->cpu->memory[0x2007];
  emu->ppu->OAMDMA = emu->cpu->memory[0x4014];
}

void ppu_write_cpu(const struct emulator *const emu) {
  emu->cpu->memory[0x2000] = emu->ppu->PPUCTRL;
  emu->cpu->memory[0x2001] = emu->ppu->PPUMASK;
  emu->cpu->memory[0x2002] = emu->ppu->PPUSTATUS;
  emu->cpu->memory[0x2003] = emu->ppu->OAMADDR;
  emu->cpu->memory[0x2004] = emu->ppu->OAMDATA;
  emu->cpu->memory[0x2005] = emu->ppu->PPUSCROLL;
  emu->cpu->memory[0x2006] = emu->ppu->PPUADDR;
  emu->cpu->memory[0x2007] = emu->ppu->PPUDATA;
  emu->cpu->memory[0x4014] = emu->ppu->OAMDMA;
}
