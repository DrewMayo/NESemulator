#ifndef EMU_H
#define EMU_H

#include "cartridge.h"
#include "cpu.h"
#define RAMSIZE 65536
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

typedef struct {
  struct cpu_6502 cpu;
  struct cartridge cart;
  uint8_t memory[RAMSIZE];
} emulator_t;
#endif
