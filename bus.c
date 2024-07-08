#include "bus.h"
#include "cartridge.h"
#include "emulator.h"
#include <stdint.h>
// This function is for memory mapping the function of the cpu
//
void bus_write(struct Bus *const bus, uint16_t addr, const uint8_t value, const enum Memtype memtype) {
  switch (memtype) {
  case CPUMEM:
    // Proper ram mirroring
    if (addr > 0x7FF && addr < 0x2000) {
      addr %= 0x0800;
      // PPU map mirroring
    } else if (addr > 0x2007 && addr < 0x4000) {
      addr = addr % 0x0008 + 0x2000;
    } else if (addr >= 0x4000) {
      bus_write(bus, addr, value, PRGCARTMEM);
    }
    bus->cpu->memory[addr] = value;
  case PPUMEM:
    break;
  case PRGCARTMEM:
    cart_write_prg_memory(bus->cart, addr, value);
    break;
  case CHRCARTMEM:
    break;
  }
}

uint8_t bus_read(const struct Bus *const bus, uint16_t addr, const enum Memtype memtype) {
  switch (memtype) {
  case CPUMEM: {
    if (addr > 0x7FF && addr < 0x2000) {
      addr %= 0x0800;
    } else if (addr > 0x2007 && addr < 0x4000) {
      addr = addr % 0x0008 + 0x2000;
    } else if (addr >= 0x4000) {
      return bus_read(bus, addr, PRGCARTMEM);
    }
    return bus->cpu->memory[addr];
  }
  case PPUMEM: {
    return 0;
  }
  case PRGCARTMEM: {
    return cart_read_prg_memory(bus->cart, addr);
  }
  case CHRCARTMEM: {
    return 0;
  }
  }
  return 0;
}

struct Bus *bus_build(struct emulator *const emu) {
  struct Bus *bus = (struct Bus *)malloc(sizeof(struct Bus));
  bus->cpu = emu->cpu;
  bus->ppu = emu->ppu;
  bus->cart = emu->cart;
  emu->cpu->bus = bus;
  emu->ppu->bus = bus;
  emu->cart->bus = bus;
  return bus;
}
