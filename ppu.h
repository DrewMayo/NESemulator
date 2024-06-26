#ifndef PPU_H
#define PPU_H
#include <stdint.h>
struct ppu_2C02 {
  uint8_t PPUCTRL; //WRITE
  uint8_t PPUMASK;
  uint8_t PPUSTATUS;
  uint8_t OAMADDR;
  uint8_t OAMDATA;
  uint8_t PPUSCROLL;
  uint8_t PPUADDR;
  uint8_t PPUDATA;
  uint8_t OAMDMA;
  uint8_t ppu_memory[0x10000];
};

void ppu_run(uint8_t *memory);
#endif
