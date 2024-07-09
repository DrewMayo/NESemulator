#pragma once
#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include "cpu.h"
#include "ppu.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// bool cart_build(const char *filename, struct cartridge *cart);
struct Bus;

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
  struct Bus *bus;
};

struct ppu_2C02;
struct cartridge *cart_build(const char *filename);
uint8_t cart_read_prg_memory(struct cartridge *cart, uint16_t mem_location);
void cart_write_prg_memory(struct cartridge *cart, uint16_t mem_location, uint8_t value);
uint8_t cart_read_chr_memory(struct cartridge *cart, uint16_t mem_location);
void cart_write_chr_memory(struct cartridge *cart, uint16_t mem_location, uint8_t value);
void cart_delete(struct cartridge *cart);
#endif
