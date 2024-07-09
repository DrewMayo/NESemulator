#include "ppu.h"
#include "bitmask.h"
#include "bus.h"
#include "cartridge.h"
#include "emulator.h"

#define PPU_WIDTH 341
#define PPU_HEIGHT 262

void ppu_read_cpu(struct ppu_2C02 *const ppu);
void ppu_write_cpu(const struct ppu_2C02 *const ppu);
void ppu_run(struct ppu_2C02 *const ppu);

void ppu_tick(struct ppu_2C02 *const ppu) {
  if (ppu->cycles % 341 == 0) {
    ppu_run(ppu);
    ppu->scanline++;
  }
  ppu->cycles++;
}

void ppu_run(struct ppu_2C02 *const ppu) {
  ppu_read_cpu(ppu);
  if (ppu->scanline == 241) {
    ppu->PPUSTATUS |= BIT7;
    ppu->bus->cpu->interrupt_state = INONMASKABLE;
  } else if (ppu->scanline == 261) {
    ppu->PPUSTATUS &= 0b01111111;
  } else if (ppu->scanline > 320) {
    ppu->scanline = -1;
  } else {
  }
  ppu_write_cpu(ppu);
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

void ppu_read_cpu(struct ppu_2C02 *const ppu) {
  ppu->PPUCTRL = bus_read(ppu->bus, 0x2000, CPUMEM);
  ppu->PPUMASK = bus_read(ppu->bus, 0x2001, CPUMEM);
  ppu->PPUSTATUS = bus_read(ppu->bus, 0x2002, CPUMEM);
  ppu->OAMADDR = bus_read(ppu->bus, 0x2003, CPUMEM);
  ppu->OAMDATA = bus_read(ppu->bus, 0x2004, CPUMEM);
  ppu->PPUSCROLL = bus_read(ppu->bus, 0x2005, CPUMEM);
  ppu->PPUADDR = bus_read(ppu->bus, 0x2006, CPUMEM);
  ppu->PPUDATA = bus_read(ppu->bus, 0x2007, CPUMEM);
  ppu->OAMDMA = bus_read(ppu->bus, 0x4014, CPUMEM);
}

void ppu_write_cpu(const struct ppu_2C02 *const ppu) {
  bus_write(ppu->bus, 0x2000, ppu->PPUCTRL, CPUMEM);
  bus_write(ppu->bus, 0x2001, ppu->PPUMASK, CPUMEM);
  bus_write(ppu->bus, 0x2002, ppu->PPUSTATUS, CPUMEM);
  bus_write(ppu->bus, 0x2003, ppu->OAMADDR, CPUMEM);
  bus_write(ppu->bus, 0x2004, ppu->OAMDATA, CPUMEM);
  bus_write(ppu->bus, 0x2005, ppu->PPUSCROLL, CPUMEM);
  bus_write(ppu->bus, 0x2006, ppu->PPUADDR, CPUMEM);
  bus_write(ppu->bus, 0x2007, ppu->PPUDATA, CPUMEM);
  bus_write(ppu->bus, 0x2007, ppu->PPUDATA, CPUMEM);
}
