#pragma once
#include "cartridge.h"
#include "emulator.h"
#include "ppu.h"
#include <stdint.h>
#include <stdlib.h>

struct Bus {
  struct cpu_6502 *cpu;
  struct ppu_2C02 *ppu;
  struct cartridge *cart;
};

enum Memtype {
  CPUMEM,
  PPUMEM,
  PRGCARTMEM,
  CHRCARTMEM,
};

void bus_write(struct Bus *const bus, uint16_t addr, const uint8_t value, const enum Memtype memtype);
uint8_t bus_read(struct Bus *const bus, uint16_t addr, const enum Memtype memtype);
struct Bus *bus_build(struct emulator *const emu);
