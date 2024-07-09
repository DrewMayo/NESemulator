#pragma once
#ifndef PPU_H
#define PPU_H
#include "cartridge.h"
#include <stdint.h>
struct emulator;
struct Bus;
struct ppu_2C02 {
  uint8_t PPUCTRL; // WRITE
  uint8_t PPUMASK;
  uint8_t PPUSTATUS;
  uint8_t OAMADDR;
  uint8_t OAMDATA;
  uint8_t PPUSCROLL;
  uint8_t PPUADDR;
  uint8_t PPUDATA;
  uint8_t OAMDMA;
  uint8_t memory[0x10000];
  int16_t scanline;
  int cycles;
  struct Bus *bus;
};
struct cartridge;
void ppu_tick(struct ppu_2C02 *const ppu);
struct ppu_2C02 *ppu_build();
#endif
