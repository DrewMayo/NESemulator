#include "ppu.h"
#include "bitmask.h"
#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "emulator.h"
#include "test.h"
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <sys/types.h>

void render_ppu_scanline(struct ppu_2C02 *const ppu);
void ppu_run(struct ppu_2C02 *const ppu);
SDL_Texture *nsdl_create_texture(struct nsdl_manager *manager);
void nsdl_update_texture(struct nsdl_manager *manager, uint32_t *rgba_arr);
void check_scanline(struct ppu_2C02 *const ppu);
void build_pallete_ram(uint32_t *pallete_ram);
void coarse_x_increment(struct ppu_2C02 *const ppu);
void y_increment(struct ppu_2C02 *const ppu);
void ppudata_update_v(struct ppu_2C02 *const ppu);
void populate_frame(struct ppu_2C02 *const ppu, const uint16_t pattern_table_lower, const uint16_t pattern_table_upper);
void add_pixel(struct ppu_2C02 *const ppu, uint16_t *const pattern_table_hi, uint16_t *const pattern_table_lo);
void ppu_render_cycle(struct ppu_2C02 *const ppu, uint16_t *const shift_reg_hi, uint16_t *const shift_reg_lo);
void check_generate_nmi(enum intrpt_states *const cpu_intrpt_state, const uint8_t ppuctrl, const uint8_t ppustatus);

void shift_registers(uint16_t *const shift_reg_hi, uint16_t *const shift_reg_lo) {
  *shift_reg_hi = (*shift_reg_hi << 1);
  *shift_reg_lo = (*shift_reg_lo << 1);
}

void ppu_tick(struct ppu_2C02 *const ppu) {
  static uint16_t shift_reg_hi = 0;
  static uint16_t shift_reg_lo = 0;
  // pre proccessings such as printing info and generating the NMI for use in the cpu
  // test_ppu(*ppu);
  // actually tick
  if ((ppu->scanline < 240 || ppu->scanline == 261) && (ppu->PPUMASK & RENDERON)) {
    ppu_render_cycle(ppu, &shift_reg_hi, &shift_reg_lo);
  } else if (ppu->scanline == 241 && ppu->cycles == 1) {
    ppu->PPUSTATUS |= BIT7;
  }
  check_generate_nmi(&ppu->bus->cpu->interrupt_state, ppu->PPUCTRL, ppu->PPUSTATUS);
  // shift_registers(ppu, &shift_reg_hi, &shift_reg_lo);
  check_scanline(ppu);
}

void ppu_render_cycle(struct ppu_2C02 *const ppu, uint16_t *const shift_reg_hi, uint16_t *const shift_reg_lo) {
  static uint16_t pattern_table_lo = 0;
  static uint16_t pattern_table_hi = 0;
  static uint8_t nametable = 0;
  static uint8_t attribute_table = 0;
  if ((ppu->cycles > 1 && ppu->cycles < 258) || (ppu->cycles > 320 && ppu->cycles < 338)) {
    shift_registers(shift_reg_hi, shift_reg_lo);
    add_pixel(ppu, shift_reg_hi, shift_reg_lo);
    //  this is relative
    switch ((ppu->cycles - 1) % 8) {
    // nametable fetch
    case 0: {
      *shift_reg_lo = (*shift_reg_lo & 0xFF00) | (pattern_table_lo);
      *shift_reg_hi = (*shift_reg_hi & 0xFF00) | (pattern_table_hi);
      nametable = bus_read(ppu->bus, 0x2000 | (ppu->V & 0x0FFF), PPUMEM);
      break;
    }
    // attribute fetch
    case 2: {
      attribute_table =
          bus_read(ppu->bus, 0x23C0 | (ppu->V & 0x0C00) | ((ppu->V >> 4) & 0x38) | ((ppu->V >> 2) & 0x07), PPUMEM);
      break;
    }
    // patern table lo fetch
    case 4: {
      pattern_table_lo = bus_read(
          ppu->bus, (((ppu->PPUCTRL & BIT4) >> 4) << 12) | (nametable << 4) | ((ppu->V & 0x7000) >> 12), PPUMEM);
      break;
    }
    // patern table hi fetch
    case 6: {
      pattern_table_hi = bus_read(
          ppu->bus, ((((ppu->PPUCTRL & BIT4) >> 4) << 12) | (nametable << 4) | (((ppu->V & 0x7000) >> 12) + 0x08)),
          PPUMEM);
      break;
    }
    // inc hori(v)
    case 7: {
      if (ppu->cycles != 256) {
        coarse_x_increment(ppu);
      }
      break;
    }
    }
  }
  // Events that happen once a scanline
  if (ppu->cycles == 256) {
    y_increment(ppu);
  }
  if (ppu->cycles == 257) {
    // coarse x and nametable X
    ppu->V &= ~REGISTERX;
    ppu->V |= (ppu->T & REGISTERX);
  }
  if (ppu->cycles > 279 && ppu->cycles < 305 && ppu->scanline == 261) {
    // vert(v) = vert(t)
    // this is true for ticks 280 to 304
    //  coarse y and nametable y
    ppu->V &= ~REGISTERY;
    ppu->V |= (ppu->T & REGISTERY);
  }
  if (ppu->scanline == 261 && ppu->cycles == 1) {
    ppu->PPUSTATUS &= ~BIT7;
  }
}

void check_scanline(struct ppu_2C02 *const ppu) {
  if (ppu->cycles == 340 || (ppu->cycles == 339 && !ppu->is_even_frame && (ppu->PPUMASK & RENDERON))) {
    ppu->scanline++;
    if (ppu->scanline == 262) {
      ppu->scanline = 0;
    }
    ppu->cycles = 0;
    ppu->is_even_frame = !ppu->is_even_frame;
    // DON'T INCREMENT CYCLES
    return;
  }
  ppu->cycles++;
}

void check_generate_nmi(enum intrpt_states *const cpu_intrpt_state, const uint8_t ppuctrl, const uint8_t ppustatus) {
  static uint8_t old_ppuctrl;
  static uint8_t old_ppustatus;
  static bool nmi_high = false;
  // make sure that many nmi calls don't happen unintentionally
  // does not exist in hardware
  if ((old_ppustatus & BIT7) != (ppustatus & BIT7)) {
    nmi_high = true;
    old_ppustatus = ppustatus;
  }
  if ((old_ppuctrl & BIT7) != (ppuctrl & BIT7)) {
    nmi_high = true;
    old_ppuctrl = ppuctrl;
  }
  // generate an nmi if it is high and the conditions BIT7 of status and ctrl
  // registers are high
  if ((ppuctrl & ppustatus & BIT7) && nmi_high) {
    *cpu_intrpt_state = INONMASKABLE;
    nmi_high = false;
  }
}

void add_pixel(struct ppu_2C02 *const ppu, uint16_t *const shift_register_hi, uint16_t *const shift_register_lo) {
  static uint32_t pixels[WINDOW_AREA];
  static uint32_t offset = 0;
  uint32_t color = 0;
  uint8_t value = (*shift_register_hi & (0x8000 >> ppu->X)) >> (14) | (*shift_register_lo & (0x8000 >> ppu->X)) >> (15);
  if (offset == WINDOW_AREA) {
    nsdl_update_texture(ppu->nsdl, pixels);
    offset = 0;
    /*printf("vram:\n");
    for (int i = 0; i < 0x800; i++) {
      printf("%2X ", ppu->memory[i]);
      if (i % 32 == 0) {
        printf("\n");
      }
      if (i % 0x400 == 0) {
        printf("\n");
      }
    }
    printf("\n");*/
  }
  switch (value) {
  case 0: {
    color = 0x000000FF;
    break;
  }
  case 1: {
    color = 0x00FF00FF;
    break;
  }
  case 2: {
    color = 0xFF00FFFF;
    break;
  }
  case 3: {
    color = 0xFFFFFF0A;
    break;
  }
  }
  /*printf("scanline: %d, cycles %d, shift_register_hi: %16b, shift_register_lo:
     %16b, value: %d, color: %d, offset: %d\n", ppu->scanline, ppu->cycles,
     *shift_register_hi, *shift_register_lo, value, color, offset);*/
  // subtract 2 because it is NOT shifting on cycles 0 and 1
  if (ppu->cycles - 2 < 256 && ppu->scanline < 240) {
    pixels[ppu->cycles - 2 + ppu->scanline * 256] = color;
    offset++;
  }
}

// **********************
// Register writes
// **********************

void ppuctrl_write(struct ppu_2C02 *const ppu, const uint8_t value) {
  ppu->PPUCTRL = value;
  ppu->T = (ppu->T & ~NAMETABLE) | ((ppu->PPUCTRL & 0x3) << 10);
}

void ppumask_write(struct ppu_2C02 *const ppu, const uint8_t value) { ppu->PPUMASK = value; }
// note: normal writes in the 2C02 corrupt the oamdata but not
void oamaddr_write(struct ppu_2C02 *const ppu, const uint8_t value) { ppu->OAMADDR = value; }

void oamdata_write(struct ppu_2C02 *const ppu, const uint8_t value) {
  ppu->OAMDATA = value;
  ppu->OAMADDR += 1;
}
// This function is not finished and needs to be fixed
// but is ommited for the time being
void oamdma_write(struct ppu_2C02 *const ppu, const uint8_t value) { ppu->OAMDMA = value; }

void ppuscroll_write(struct ppu_2C02 *const ppu, const uint8_t value) {
  if (!ppu->W) {
    ppu->T = (ppu->T & 0b1111111111100000) | ((value & 0b11111000) >> 3);
    ppu->X = value & 0b00000111;
    ppu->W = 1;
  } else {
    ppu->T =
        (ppu->T & 0b1000110000011111) | ((uint16_t)(value & 0b00000111) << 12) | ((uint16_t)(value & 0b11111000) << 2);
    // not in spec but this is supposed to be 15bit
    ppu->T = ppu->T & 0x7FFF;
    ppu->W = 0;
  }
  ppu->PPUSCROLL = value;
}

void ppuaddr_write(struct ppu_2C02 *const ppu, const uint8_t value) {
  if (!ppu->W) {
    ppu->T = (ppu->T & 0b1100000011111111) + ((((uint16_t)value) & 0b00111111) << 8);
    ppu->T = ppu->T & 0x3FFF;
    ppu->W = 1;
  } else {
    ppu->T = (ppu->T & 0xFF00) | value;
    ppu->V = ppu->T;
    ppu->W = 0;
  }
}
void ppudata_write(struct ppu_2C02 *const ppu, const uint8_t value) {
  if ((ppu->scanline < 241 || ppu->scanline > 260)) {
    // printf("PPUDATA WRITE OUTSIDE OF VBLANK, cycles: %d, ppuscanline: %d, ppucycles: %d\n", ppu->bus->cpu->cycles,
    //      ppu->scanline, ppu->cycles);
  }
  if (ppu->V > 0x1FFF && ppu->V < 0x3000) {
    bus_write(ppu->bus, ppu->V, value, PPUMEM);
  }
  ppu->PPUDATA = value;
  ppudata_update_v(ppu);
}

// ***********************
// Register reads
// ***********************

uint8_t ppustatus_read(struct ppu_2C02 *const ppu) {
  const uint8_t return_val = ppu->PPUSTATUS;
  ppu->PPUSTATUS &= ~BIT7;
  ppu->W = 0;
  return return_val;
}

uint8_t oamdata_read(struct ppu_2C02 *const ppu) { return bus_read(ppu->bus, ppu->OAMADDR, PPUMEM); }

uint8_t ppudata_read(struct ppu_2C02 *const ppu) {
  if ((ppu->scanline < 241 || ppu->scanline > 260)) {
    // printf("PPUDATA READ OUTSIDE OF VBLANK, cycles: %d, ppuscanline: %d, ppucycles: %d\n", ppu->bus->cpu->cycles,
    //     ppu->scanline, ppu->cycles);
  }
  static uint8_t buffer = 0;
  const uint8_t return_val = buffer;
  if (ppu->V > 0x1FFF && ppu->V < 0x3000) {
    buffer = bus_read(ppu->bus, ppu->V, PPUMEM);
  }
  ppudata_update_v(ppu);
  return return_val;
}

// ***********************
// Helper functions
// ***********************

void ppudata_update_v(struct ppu_2C02 *const ppu) {
  // printf("inc_v, scanline: %d, cycles %d\n", ppu->scanline, ppu->cycles);
  ppu->V += (ppu->PPUCTRL & BIT2) ? 32 : 1;
}

// copied from: https://www.nesdev.org/wiki/PPU_scrolling#Coarse_X_increment
void coarse_x_increment(struct ppu_2C02 *const ppu) {
  // printf("scanline: %d, cycle: %d, ppu->V: 0x%4X, counter: %d\n",
  // ppu->scanline, ppu->cycles,
  //       0x2000 | (ppu->V & 0x0FFF), counter);
  if ((ppu->V & 0x001F) == 31) { // if course X == 31
    ppu->V &= ~0x001F;           // coarse X = 0
    ppu->V ^= 0x0400;            // Switch horizontal nametable
  } else {
    ppu->V += 1; // increment coarse x
  }
}
// THIS IS MESSED UP
// copied from: https://www.nesdev.org/wiki/PPU_scrolling#Y_increment
void y_increment(struct ppu_2C02 *const ppu) {
  if ((ppu->V & 0x7000) != 0x7000) { // if fine Y < 7
    ppu->V += 0x1000;                // incremenet fine Y
  } else {
    ppu->V &= ~0x7000;                   // fine Y = 0
    uint16_t y = (ppu->V & 0x03E0) >> 5; // let y = course Y
    if (y == 29) {
      y = 0;            // coarse Y = 0
      ppu->V ^= 0x0800; // switch vertical nametable
    } else if (y == 31) {
      y = 0; // coarse Y = 0, nametable not switched
    } else {
      y += 1; // increment coarse Y
    }
    ppu->V = (ppu->V & ~0x03E0) | (y << 5); // put coarse Y back into V
  }
}

struct ppu_2C02 *ppu_build() {
  struct ppu_2C02 *ppu = (struct ppu_2C02 *)malloc(sizeof(struct ppu_2C02));
  for (int i = 0; i < 0x0800; i++) {
    ppu->memory[i] = 0;
  }
  ppu->scanline = 0;
  ppu->cycles = 21;
  ppu->X = 0;
  ppu->W = false;
  ppu->T = 0;
  ppu->V = 0;
  ppu->is_even_frame = false;
  ppu->nsdl = start_window();
  ppu->nsdl->texture = nsdl_create_texture(ppu->nsdl);
  return ppu;
}

void populate_frame(struct ppu_2C02 *const ppu, const uint16_t pattern_table_lower,
                    const uint16_t pattern_table_upper) {
  static uint32_t pixels[WINDOW_AREA];
  static uint32_t offset = 0;
  uint8_t value = 0;
  uint32_t color = 0;
  // printf("scanline: %d, cycle: %d, offset: %d\n ", ppu->scanline,
  // ppu->cycles, offset);
  if (offset == WINDOW_AREA) {
    nsdl_update_texture(ppu->nsdl, pixels);
    offset = 0;
    printf("vram:\n");
    for (int i = 0; i < 0x800; i++) {
      printf("%2X ", ppu->memory[i]);
      if (i % 32 == 0) {
        printf("\n");
      }
      if (i % 0x400 == 0) {
        printf("\n");
      }
    }
    printf("\n");
  }
  // printf("nametable: 0x%X, attribute_table: 0x%X, pattern_table_lower: 0x%X,
  // pattern_table_upper: 0x%X\n", nametable, attribute_table,
  // pattern_table_lower, pattern_table_upper);
  for (int i = 0; i < 8; i++) {
    value = ((((uint16_t)pattern_table_upper << i) & BIT7) >> 6) | ((((uint16_t)pattern_table_lower << i) & BIT7) >> 7);
    switch (value) {
    case 0:
      color = 0x000000FF;
      break;
    case 1:
      color = 0x00FF00FF;
      break;
    case 2:
      color = 0xFF00FFFF;
      break;
    case 3:
      color = 0xFFFFFF0A;
      break;
    }
    pixels[offset + i] = color;
  }
  offset += 8;
  // nsdl_update_texture(ppu->nsdl, pixels);
}

/*
 * SDL FUNCTIONS
 */

SDL_Texture *nsdl_create_texture(struct nsdl_manager *manager) {
  SDL_Texture *texture = SDL_CreateTexture(manager->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
                                           WINDOW_WIDTH, WINDOW_HEIGHT);
  if (texture == NULL) {
    return NULL;
  }
  return texture;
}

void nsdl_update_texture(struct nsdl_manager *manager, uint32_t *rgba_arr) {
  int pitch = 4 * WINDOW_WIDTH;
  uint32_t *pixels;
  int cur_window_height = WINDOW_WIDTH;
  SDL_GetWindowSize(manager->window, NULL, &cur_window_height);
  int multiplier = cur_window_height / WINDOW_WIDTH;
  multiplier = (multiplier < 1) ? 1 : multiplier;
  // SDL_Rect drect = {0, 0, 128, 128};
  SDL_Rect drect = {.x = 0, .y = 0, .w = WINDOW_WIDTH * multiplier, .h = WINDOW_HEIGHT * multiplier};
  SDL_LockTexture(manager->texture, NULL, (void **)&pixels, &pitch);
  for (int i = 0; i < WINDOW_AREA; i++) {
    pixels[i] = rgba_arr[i];
  }
  SDL_UnlockTexture(manager->texture);
  SDL_RenderClear(manager->renderer);
  SDL_RenderCopy(manager->renderer, manager->texture, NULL, &drect);
  SDL_RenderPresent(manager->renderer);
}

// This starts the window,creates the manager, and creates
// the renderer to use.
struct nsdl_manager *start_window() {

  // Initialize the SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) == -1) {
    printf("Could not init SDL: %s.\n", SDL_GetError());
    return NULL;
  }
  // allocate the manager
  struct nsdl_manager *manager = malloc(sizeof(struct nsdl_manager));

  // create the window and check for errors
  manager->window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                                     WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

  if (manager->window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create the window: %s\n", SDL_GetError());
    return NULL;
  }

  // create renderer and check for errors
  manager->renderer = SDL_CreateRenderer(manager->window, -1, 0);
  if (manager->renderer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create the renderer: %s\n", SDL_GetError());
    return NULL;
  }
  return manager;
}

// Destroys the created window by SDL

bool destroy_window() {
  SDL_Quit();
  return true;
}
