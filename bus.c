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
    break;
  case PPUMEM: {
    if (addr <= 0x1FFF) {
      // CHR-ROM
      bus_write(bus, addr, value, CHRCARTMEM);
    } else if (addr >= 0x2000 && addr <= 0x2FFF) {
      // INTERNAL VRAM 2 KiB
      bus->ppu->memory[addr] = value;
      break;
    } else if (addr >= 0x3000 && addr <= 0x3EFF) {
      // UNUSED
      addr %= 0x0400 + 0x2000;
    } else if (addr >= 0x3F00 && addr <= 0x3F1F) {
      // INTERNAL PALLETE RAM
      bus->ppu->memory[addr] = value;
      break;
    } else if (addr >= 0x3F20 && addr <= 0x3FFF) {
      addr %= 0x0020 + 0x3F00;
    }
    bus_write(bus, addr, value, PPUMEM);
    break;
  }
  case PRGCARTMEM:
    cart_write_prg_memory(bus->cart, addr, value);
    break;
  case CHRCARTMEM:
    cart_write_chr_memory(bus->cart, addr, value);
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
    if (addr <= 0x1FFF) {
      // CHR-ROM
      return bus_read(bus, addr, CHRCARTMEM);
    } else if (addr >= 0x2000 && addr <= 0x2FFF) {
      // INTERNAL VRAM 2 KiB
      return bus->ppu->memory[addr];
    } else if (addr >= 0x3000 && addr <= 0x3EFF) {
      // UNUSED
      addr %= 0x0400 + 0x2000;
    } else if (addr >= 0x3F00 && addr <= 0x3F1F) {
      // INTERNAL PALLETE RAM
      return bus->ppu->memory[addr];
    } else if (addr >= 0x3F20 && addr <= 0x3FFF) {
      addr %= 0x0020 + 0x3F00;
    }
    return bus_read(bus, addr, PPUMEM);
  }
  case PRGCARTMEM: {
    return cart_read_prg_memory(bus->cart, addr);
  }
  case CHRCARTMEM: {
    return cart_read_chr_memory(bus->cart, addr);
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
