#include "cpu.h"
#include <stdbool.h>
#include <stdint.h>

uint16_t fetchAddrMode(addressing_mode_t addr_mode, cpu_t *cpu,
                       uint8_t *memory);

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
// ADC - ADD with Carry
void ADC(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint16_t mem = (uint16_t)memory[memAddr];
  uint16_t AC = (uint16_t)cpu->AC;
  uint16_t carry = (uint16_t)cpu->SR.Carry;
  uint16_t added = mem + AC + carry;
  cpu->AC = (uint8_t)added;
  if (added > 255) {
    cpu->SR.Carry = true;
  }
  // formula for determining overflow
  // (result ^ M) & (result ^ mem) & BIT7
  if ((AC ^ added) & (added ^ mem) & BIT7) {
    cpu->SR.Overflow = true;
  }

  if (cpu->AC == 0) {
    cpu->SR.Zero = true;
  }
  if (cpu->AC & BIT7) {
    cpu->SR.Negative = true;
  }
}
// AND - Logical And
void AND(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  cpu->AC &= memory[memAddr];
  if (cpu->AC == 0) {
    cpu->SR.Zero = true;
  }
  if (cpu->AC & BIT7) {
    cpu->SR.Negative = true;
  }
}

void ASL(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t *ptr;
  if (addr_mode == ACCUMULATOR) {
    ptr = &cpu->AC;
  } else {
    ptr = &memory[memAddr];
  }
  cpu->SR.Carry = *ptr & BIT7;
  *ptr *= 2;
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = *ptr & BIT7;
}

// gets the correct memory address to be used by the corresponding opcode
// returns a 16 bit integer containing the propper memory address
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
  // AND - And AC and M->[addr_mode]
  case 0x29:
    AND(IMMEDIATE, cpu, memory);
    break;
  case 0x25:
    AND(ZEROPAGE, cpu, memory);
    break;
  case 0x35:
    AND(ZEROPAGEX, cpu, memory);
    break;
  case 0x2D:
    AND(ABSOLUTE, cpu, memory);
    break;
  case 0x3D:
    AND(ABSOLUTEX, cpu, memory);
    break;
  case 0x39:
    AND(ABSOLUTEY, cpu, memory);
    break;
  case 0x21:
    AND(INDIRECTX, cpu, memory);
    break;
  case 0x31:
    AND(INDIRECTY, cpu, memory);
    break;
  case 0x0A:
    ASL(ACCUMULATOR, cpu, memory);
    break;
  case 0x06:
    ASL(ZEROPAGE, cpu, memory);
    break;
  case 0x16:
    ASL(ZEROPAGEX, cpu, memory);
    break;
  case 0x0E:
    ASL(ABSOLUTE, cpu, memory);
    break;
  case 0x1E:
    ASL(ABSOLUTEX, cpu, memory);
    break;
  }

  cpu->PC++;
  return 0;
}

uint16_t fetchAddrMode(addressing_mode_t addr_mode, cpu_t *cpu,
                       uint8_t *memory) {
  uint16_t param = 0;
  switch (addr_mode) {
  case IMPLIED: {
    return 0;
    break;
  }
  case ACCUMULATOR: {
    return 0;
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
    // printf("memaccess: %x\n", cpu->PC);
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
    param = (((uint16_t)memory[memAccess + cpu->X + 1]) << 8) +
            memory[memAccess + cpu->X];
    break;
  }
  case INDIRECTY: {
    uint8_t memAccess = memory[cpu->PC + 1];
    cpu->PC += 1;
    param = (((uint16_t)memory[memAccess + 1]) << 8) +
            (uint16_t)memory[memAccess] + (uint16_t)cpu->Y;
    break;
  }
  case RELATIVE: {
    break;
  }
  default:
    printf("Invalid Addressing mode: %d\n", addr_mode);
    break;
  }
  return param;
}
