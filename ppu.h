#pragma once
#ifndef PPU_H
#define PPU_H
#define WINDOW_WIDTH 256
#define WINDOW_HEIGHT 240
#define WINDOW_AREA 61440
#define PIXEL_WIDTH 32
#define PIXEL_HEIGHT 30
#define PIXEL_AREA 960
#define REGISTERX 0x041F
#define REGISTERY 0x7BE0
#define RENDERON 0x18

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>

struct emulator;
struct Bus;
struct ppu_2C02 {
  // external registers
  uint8_t PPUCTRL;   // WRITE 0x2000
  uint8_t PPUMASK;   // WRITE 0x2001
  uint8_t PPUSTATUS; // READ 0x2002
  uint8_t OAMADDR;   // WRITE 0x2003
  uint8_t OAMDATA;   // WRITE/READ 0x2004
  uint8_t PPUSCROLL; // WRITE 2x 0x2005
  uint8_t PPUADDR;   // WRITE 2x 0x2006
  uint8_t PPUDATA;   // WRITE/READ 0x2007
  uint8_t OAMDMA;    // WRITE/READ 0x4014
  // internal registers
  uint16_t T;
  uint16_t V;
  uint8_t X;
  bool W;
  // helpful things
  uint16_t scanline;
  uint16_t cycles;
  bool is_even_frame;
  uint8_t memory[0x10000];
  uint32_t pallete_ram[0x40];
  struct nsdl_manager *nsdl;
  struct Bus *bus;
};

struct nsdl_manager {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
};

struct nsdl_manager *start_window();

bool destroy_window();

bool renderLoop(struct nsdl_manager *manager);

void ppu_tick(struct ppu_2C02 *const ppu);

void ppuctrl_write(struct ppu_2C02 *const ppu, const uint8_t value);
void ppumask_write(struct ppu_2C02 *const ppu, const uint8_t value);
void oamaddr_write(struct ppu_2C02 *const ppu, const uint8_t value);
void oamdata_write(struct ppu_2C02 *const ppu, const uint8_t value);
void oamdma_write(struct ppu_2C02 *const ppu, const uint8_t value);
void ppuscroll_write(struct ppu_2C02 *const ppu, const uint8_t value);
void ppuaddr_write(struct ppu_2C02 *const ppu, const uint8_t value);
void ppudata_write(struct ppu_2C02 *const ppu, const uint8_t value);

uint8_t ppustatus_read(struct ppu_2C02 *const ppu);
uint8_t oamdata_read(struct ppu_2C02 *const ppu);
uint8_t ppudata_read(struct ppu_2C02 *const ppu);

struct ppu_2C02 *ppu_build();
#endif
