#include "test.h"
#include "cpu.h"
#include <stdbool.h>
#include <stdint.h>

void testCpuPart(const struct cpu_6502 cpu, const uint8_t *memory, const struct instruction instr) {
  printf("%04X  ", cpu.PC & 0xFFFF);
  printf("%02X ", memory[cpu.PC] & 0xFF);
  switch (instr.addr_mode) {
  case IMPLIED:
  case ACCUMULATOR:
    printf("     ");
    break;
  case IMMEDIATE:
  case ZEROPAGE:
  case ZEROPAGEX:
  case INDIRECTX:
  case INDIRECTY:
  case ZEROPAGEY:
  case RELATIVE:
    printf("%02X   ", memory[cpu.PC + 1] & 0xFF);
    break;
  case ABSOLUTE:
  case ABSOLUTEX:
  case ABSOLUTEY:
  case INDIRECT:
    printf("%02X %02X", memory[cpu.PC + 1] & 0xFF, memory[cpu.PC + 2] & 0xFF);
    break;
  }
  printf("  ");
  printf("%s ", instr.name);
  printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d\n", cpu.AC, cpu.X, cpu.Y, cpu_combine_SR(cpu.SR), cpu.SP, cpu.cycles);
}
