#include "cpu.h"
#include <stdint.h>

uint16_t fetchAddrMode(addressing_mode_t addr_mode, cpu_t *cpu,
                       uint8_t *memory);
void LDA(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory);
void ADC(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory);

int run(cpu_t *cpu, uint8_t *memory) {
  uint8_t opcode = memory[cpu->PC];
  switch (opcode) {

  // LDA
  case 0xA9:
    LDA(IMMEDIATE, cpu, memory);
    break;
  case 0xA5:
    LDA(ZEROPAGE, cpu, memory);
    break;
  case 0xB5:
    LDA(ZEROPAGEX, cpu, memory);
    break;
  case 0xAD:
    LDA(ABSOLUTE, cpu, memory);
    break;
  case 0xBD:
    LDA(ABSOLUTEX, cpu, memory);
    break;
  case 0xB9:
    LDA(ABSOLUTEY, cpu, memory);
    break;
  case 0xA1:
    LDA(INDIRECTX, cpu, memory);
    break;
  case 0xB1:
    LDA(INDIRECTY, cpu, memory);
    break;
  default:
    printf("Invalid Opcode\n");
    break;
  // ADC Add with Carry
  case 0x69:
    ADC(IMMEDIATE, cpu, memory);
    break;
  case 0x65:
    ADC(ZEROPAGE, cpu, memory);
    break;
  case 0x75:
    ADC(ZEROPAGEX, cpu, memory);
    break;
  case 0x6D:
    ADC(ABSOLUTE, cpu, memory);
    break;
  case 0x7D:
    ADC(ABSOLUTEX, cpu, memory);
    break;
  case 0x79:
    ADC(ABSOLUTEY, cpu, memory);
    break;
  case 0x61:
    ADC(INDIRECTX, cpu, memory);
    break;
  case 0x71:
    ADC(INDIRECTY, cpu, memory);
    break;
  }
  cpu->PC++;
  return 0;
}

void LDA(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  cpu->AC = memory[memAddr];
  if (cpu->AC == 0) {
    cpu->SR.Zero = true;
  }
  if (cpu->AC & BIT7) {
    cpu->SR.Negative = true;
  }
}

void ADC(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint16_t added =
      (uint16_t)memory[memAddr] + (uint16_t)cpu->AC + (uint16_t)cpu->SR.Carry;
  cpu->AC = added;
  if (added > 255) {
    cpu->SR.Carry = true;
  }
  if (cpu->AC == 0) {
    cpu->SR.Zero = true;
  }
  if (cpu->AC & BIT7) {
    cpu->SR.Negative = true;
  }
}

// gets the correct memory address to be used by the corresponding opcode
// returns a 16 bit integer containing the propper memory address

uint16_t fetchAddrMode(addressing_mode_t addr_mode, cpu_t *cpu,
                       uint8_t *memory) {
  uint16_t param = 0;
  switch (addr_mode) {
  case IMPLIED: {
    return 0;
    break;
  }
  case ACCUMULATOR: {
    break;
  }
  case IMMEDIATE: {
    param = ++cpu->PC;
    break;
  }
  case ZEROPAGE: {
    uint8_t mem_access = memory[cpu->PC + 1];
    cpu->PC += 1;
    param = (uint8_t)mem_access;
    break;
  }
  case ZEROPAGEX: {
    uint8_t memAccess = memory[cpu->PC + 1];
    cpu->PC += 1;
    param = (uint8_t)(memAccess + cpu->X);
    break;
  }
  case ZEROPAGEY: {
    uint8_t memAccess = memory[cpu->PC + 1];
    cpu->PC += 1;
    printf("memaccess: %x\n", cpu->PC);
    param = (uint8_t)(memAccess + cpu->Y);
    break;
  }
  case ABSOLUTE: {
    uint16_t memAccess =
        memory[cpu->PC + 1] + (((uint16_t)memory[cpu->PC + 2]) << 8);
    param = memAccess;
    cpu->PC += 2;
    break;
  }
  case ABSOLUTEX: {
    uint16_t memAccess =
        memory[cpu->PC + 1] + (((uint16_t)memory[cpu->PC + 2]) << 8);
    param = memAccess + cpu->X;
    cpu->PC += 2;
    break;
  }
  case ABSOLUTEY: {
    uint16_t memAccess =
        memory[cpu->PC + 1] + (((uint16_t)memory[cpu->PC + 2]) << 8);
    param = memAccess + cpu->Y;
    cpu->PC += 2;
    break;
  }
  case ABSOLUTEINDIRECT: {
    uint16_t memAccess =
        memory[cpu->PC + 1] + (((uint16_t)memory[cpu->PC + 2]) << 8);
    param = (((uint16_t)memory[memAccess + 1]) << 8) + memory[memAccess];
    cpu->PC += 2;
    break;
  }
  case INDIRECTX: {
    uint8_t memAccess = memory[cpu->PC + 1];
    cpu->PC += 1;
    param = (((uint16_t)memory[memAccess + cpu->X+ 1]) << 8) +
            memory[memAccess + cpu->X];
    break;
  }
  case INDIRECTY: {
    uint8_t memAccess = memory[cpu->PC + 1];
    cpu->PC += 1;
    param =
        (((uint16_t)memory[memAccess + 1]) << 8) + (uint16_t)memory[memAccess] + (uint16_t) cpu->Y;
    break;
  }
  case RELATIVE: {
    break;
  }
  default:
    printf("Invalid Addressing mode\n");
    break;
  }
  return param;
}
