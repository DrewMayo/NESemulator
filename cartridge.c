#include "cartridge.h"
#include "bitmask.h"
#include "emulator.h"
#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdio.h>

#define PRG_ROM_SIZE 16384
#define CHR_ROM_SIZE 8192
#define PRG_RAM_SIZE 8192

bool is_nes(const uint8_t *header);

struct cartridge *cart_build(const char *filename) {
  FILE *fp = fopen(filename, "rb");
  struct cartridge *cart = (struct cartridge *)malloc(sizeof(struct cartridge));
  fread(cart->header, sizeof(uint8_t), 16, fp);
  if (!is_nes(cart->header)) {
    return NULL;
  }
  cart->ines = 1;
  // malloc the prg_rom and chr_rom
  cart->prg_rom = (uint8_t *)malloc(sizeof(uint8_t) * PRG_ROM_SIZE * cart->header[4]);
  cart->chr_rom = (uint8_t *)malloc(sizeof(uint8_t) * CHR_ROM_SIZE * cart->header[5]);

  // control byte 1, header byte 6
  cart->mirror = cart->header[6] & BIT0 ? VERTICAL : HORIZONTAL;
  cart->contains_pgr_ram = cart->header[6] & BIT1;
  cart->contains_trainer = cart->header[6] & BIT2;
  cart->alternate_name_table = cart->header[6] & BIT3;
  cart->mapper = (cart->header[6] >> 4) & 0x0F;

  // control byte 2, header byte 7
  cart->VS_unisystem = cart->header[7] & BIT0;
  if ((cart->header[7] & 0b00001100) >> 2 == 2) {
    cart->ines = 2;
  }
  cart->mapper = cart->mapper | (cart->header[7] & 0xF0);

  // malloc the pgr_ram
  cart->prg_ram = (uint8_t *)malloc(sizeof(uint8_t) * PRG_RAM_SIZE * cart->header[8]);

  // mapper 0
  switch (cart->mapper) {
  case 0: {
    fread(cart->prg_rom, sizeof(uint8_t), PRG_ROM_SIZE * cart->header[4], fp);
    fread(cart->chr_rom, sizeof(uint8_t), CHR_ROM_SIZE * cart->header[5], fp);
  }
  }
  fclose(fp);
  return cart;
}

uint8_t cart_read_prg_memory(struct cartridge *cart, uint16_t mem_location) {
  switch (cart->mapper) {
  case 0: {
    mem_location %= 0x8000;
    if (cart->header[4] == 1) {
      mem_location %= PRG_ROM_SIZE;
      return cart->prg_rom[mem_location];
    } else {
      return cart->prg_rom[mem_location];
    }
  }
  }
  return 0;
}

void cart_write_prg_memory(struct cartridge *cart, uint16_t mem_location, uint8_t value) {
  switch (cart->mapper) {
  case 0: {
    mem_location %= 0x8000;
    if (cart->header[4] == 1) {
      mem_location %= PRG_ROM_SIZE;
      cart->prg_rom[mem_location] = value;
    } else {
      cart->prg_rom[mem_location] = value;
    }
  }
  }
}

uint8_t cart_read_chr_memory(struct cartridge *cart, uint16_t mem_location) {
  switch (cart->mapper) {
  case 0: {
    return cart->chr_rom[mem_location];
  }
  }
  return 0;
}
void cart_write_chr_memory(struct cartridge *cart, uint16_t mem_location, uint8_t value) {
  switch (cart->mapper) {
  case 0: {
    cart->chr_rom[mem_location] = value;
  }
  }
}

// All nes files start with
// 4E 45 53 1A or "NES^Z" so
// it checks for this part
bool is_nes(const uint8_t *header) {
  if (header[0] != 0x4E) {
    return false;
  }
  if (header[1] != 0x45) {
    return false;
  }
  if (header[2] != 0x53) {
    return false;
  }
  if (header[3] != 0x1A) {
    return false;
  }
  return true;
}

void cart_delete(struct cartridge *cart) {
  free(cart->prg_rom);
  free(cart->chr_rom);
  free(cart->prg_ram);
}
