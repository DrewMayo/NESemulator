#include "cpu.h"
#define RAMSIZE 65536
typedef struct {
  cpu_t cpu;
  uint8_t memory[RAMSIZE];
} emulator_t;
