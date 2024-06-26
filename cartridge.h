#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include "cpu.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//bool cart_build(const char *filename, struct cartridge *cart);

enum mirroring {
  VERTICAL,
  HORIZONTAL,
  FOUR_SCREEN
};

struct cartridge {
  uint8_t ines;
  uint8_t header[0x16];
  uint8_t *prg_rom;
  uint8_t *chr_rom;
  uint8_t chr_ram[2048];
  uint8_t *prg_ram;
  uint8_t trainer[512];
  uint8_t mapper;
  bool contains_trainer;
  bool contains_pgr_ram;
  bool alternate_name_table;
  bool VS_unisystem;
  enum mirroring mirror;
};

bool cart_build(const char *filename, struct cartridge *cart, struct cpu_6502 *cpu);
void cart_delete(struct cartridge *cart);
#endif
