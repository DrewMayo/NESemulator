#include "cartridge.h"
#include "bitmask.h"
#include "cpu.h"
#include <stdint.h>
#include <stdio.h>

#define PRG_ROM_SIZE 16348
#define CHR_ROM_SIZE 8192
#define PRG_RAM_SIZE 8192

bool is_nes(const uint8_t *header);
bool cart_build(const char *filename, struct cartridge *cart, struct cpu_6502 *cpu) {
  FILE *fp = fopen(filename, "rb");
  fread(cart->header, sizeof(uint8_t), 16, fp);
  if (!is_nes(cart->header)) {
    printf("Could not load rom! Not an NES file!\n");
    fclose(fp);
    return false;
  }
  cart->ines = 1;
  // malloc the prg_rom and chr_rom
  cart->prg_rom = (uint8_t *)malloc(sizeof(uint8_t) * PRG_ROM_SIZE * cart->header[4]);
  cart->chr_rom = (uint8_t *)malloc(sizeof(uint8_t) * CHR_ROM_SIZE * cart->header[5]);

  // control byte 1, header byte 6
  cart->mirror = cart->header[6] & BIT0 ? HORIZONTAL : VERTICAL;
  cart->contains_pgr_ram = cart->header[6] & BIT1;
  cart->contains_trainer = cart->header[6] & BIT2;
  cart->alternate_name_table = cart->header[6] & BIT3;
  cart->mapper = (cart->header[6] >> 4) & 0x0F;

  // control byte 2, header byte 7:""
  cart->VS_unisystem = cart->header[7] & BIT0;
  if ((cart->header[7] & 0b00001100) >> 2 == 2) {
    cart->ines = 2;
  }
  cart->mapper = cart->mapper | (cart->header[7] & 0xF0);

  // malloc the pgr_ram
  cart->prg_ram = (uint8_t *)malloc(sizeof(uint8_t) * PRG_RAM_SIZE * cart->header[8]);

  // mapper 0
  switch(cart->mapper) {
    case 0: {
      fread(cart->prg_rom, sizeof(uint8_t), PRG_ROM_SIZE * cart->header[4], fp);
      fread(cart->chr_rom, sizeof(uint8_t), CHR_ROM_SIZE, fp);
      if (cart->header[4] == 1) {
        memcpy(&cpu->memory[0x8000], cart->prg_rom, PRG_ROM_SIZE);
        memcpy(&cpu->memory[0xC000], cart->prg_rom, PRG_ROM_SIZE);
      } else {
        memcpy(&cpu->memory[0x8000], cart->prg_rom, PRG_ROM_SIZE * 2);
      }
    }
  }
  fclose(fp);
  return true;
}
/*
uint8_t read_cartridge_prg_memory(struct cartridge *cart, uint16_t mem_location) {
  switch (cart->mapper) {
    case 0: {
      mem_location %= 0x8000;
      if (cart->header[4] == 1) {
        return cart->prg_rom[mem_location % PRG_ROM_SIZE];
      } else {
        return cart->prg_rom[mem_location];
      }
    }
  }
  printf("ERROR ON READ IN PRG_ROM");
  return 0;
}

void write_cartridge_prg_memory(struct cartridge *cart, uint16_t mem_location, uint8_t *ptr) { 
  switch (cart->mapper) {
    case 0: {
      mem_location %= 0x8000;
      if (cart->header[4] == 1) {
        cart->prg_rom[mem_location % PRG_ROM_SIZE] = *ptr;
      } else {
        cart->prg_rom[mem_location] = *ptr;
      }
    }
  }
}
*/
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
