#include "bus.h"
#include "cartridge.h"
#include "emulator.h"
#include <stdint.h>
#include <sys/types.h>
// This function is for memory mapping to the correct part of (whereever it gets its memory)
void bus_ppuaddr_write(struct Bus *const bus, const uint8_t value);
void bus_ppuscroll_write(struct Bus *const bus, const uint8_t value);
uint16_t calculate_mirror_addr(const struct Bus *const bus, uint16_t addr);
uint16_t increment(struct Bus *bus, uint16_t value);

void bus_write(struct Bus *const bus, uint16_t addr, const uint8_t value, const enum Memtype memtype) {
  switch (memtype) {
  case CPUMEM:
    // Proper ram mirroring
    if (addr < 0x0800) {
      bus->cpu->memory[addr] = value;
    } else if (addr > 0x07FF && addr < 0x2000) {
      addr %= 0x0800;
      bus_write(bus, addr, value, CPUMEM);
      // PPU IO
    } else if (addr == 0x2000) {
      ppuctrl_write(bus->ppu, value);
    } else if (addr == 0x2001) {
      ppumask_write(bus->ppu, value);
    } else if (addr == 0x2002) {
      // skip
    } else if (addr == 0x2003) {
      oamaddr_write(bus->ppu, value);
    } else if (addr == 0x2004) {
      oamdata_write(bus->ppu, value);
    } else if (addr == 0x2005) {
      ppuscroll_write(bus->ppu, value);
    } else if (addr == 0x2006) {
      ppuaddr_write(bus->ppu, value);
    } else if (addr == 0x2007) {
      ppudata_write(bus->ppu, value);
    } else if (addr > 0x2007 && addr < 0x4000) {
      addr = addr % 0x0008 + 0x2000;
      bus_write(bus, addr, value, CPUMEM);
      break;
    } else if (addr >= 0x4000 && addr != 0x4014) {
      bus_write(bus, addr, value, PRGCARTMEM);
    }
    break;

  case PPUMEM: {
    // addr &= 0x3FFF;
    if (addr <= 0x1FFF) {
      // CHR-ROM
      bus_write(bus, addr, value, CHRCARTMEM);
    } else if (addr >= 0x2000 && addr <= 0x2FFF) {
      // INTERNAL VRAM 2 KiB
      addr = calculate_mirror_addr(bus, addr);
      bus->ppu->memory[addr] = value;
      break;
    } else if (addr >= 0x3000 && addr <= 0x3EFF) {
      // UNUSED
      addr %= 0x0400 + 0x2000;
    } else if (addr >= 0x3F00 && addr <= 0x3F1F) {
      // INTERNAL PALLETE RAM bus->ppu->memory[addr] = value;
      break;
    } else if (addr >= 0x3F20 && addr <= 0x3FFF) {
      addr %= 0x0020 + 0x3F00;
    }
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

uint8_t bus_read(struct Bus *const bus, uint16_t addr, const enum Memtype memtype) {
  switch (memtype) {
  case CPUMEM: {
    if (addr > 0x7FF && addr < 0x2000) {
      addr %= 0x0800;
    } else if (addr == 0x2000) {
      // skip
    } else if (addr == 0x2001) {
      // skip
    } else if (addr == 0x2002) {
      return ppustatus_read(bus->ppu);
    } else if (addr == 0x2003) {
      // skip
    } else if (addr == 0x2004) {
      return oamdata_read(bus->ppu);
    } else if (addr == 0x2005) {
      // skip
    } else if (addr == 0x2006) {
      // skip
    } else if (addr == 0x2007) {
      return ppudata_read(bus->ppu);
    } else if (addr > 0x2007 && addr < 0x4000) {
      addr = addr % 0x0008 + 0x2000;
    } else if (addr >= 0x4000) {
      return bus_read(bus, addr, PRGCARTMEM);
    }
    return bus->cpu->memory[addr];
  }
  case PPUMEM: {
    // addr &= 0x3FFF;
    if (addr <= 0x1FFF) {
      // CHR-ROM
      return bus_read(bus, addr, CHRCARTMEM);
    } else if (addr >= 0x2000 && addr <= 0x2FFF) {
      // INTERNAL VRAM 2 KiB
      addr = calculate_mirror_addr(bus, addr);
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

uint16_t calculate_mirror_addr(const struct Bus *const bus, uint16_t addr) {
  if (bus->cart->mirror == VERTICAL) {
    addr = addr % 0x0800 + 0x2000;
  } else if (bus->cart->mirror == HORIZONTAL) {
    if (addr < 0x2800) {
      addr = addr % 0x0400 + 0x2000;
    } else {
      addr = addr % 0x0400 + 0x2800;
    }
  }
  return addr;
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
