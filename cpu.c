#include "cpu.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)

uint8_t combine_SR(status_t SR);
uint8_t pageCrossed(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory,
                    uint16_t memAddr);
uint16_t fetchAddrMode(addressing_mode_t addr_mode, cpu_t *cpu,
                       uint8_t *memory);
instruction_t *create_opcodes();
// ADC - ADD with Carry
uint8_t ADC(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint16_t mem = (uint16_t)memory[memAddr];
  uint16_t AC = (uint16_t)cpu->AC;
  uint16_t carry = (uint16_t)cpu->SR.Carry;
  uint16_t added = mem + AC + carry;
  uint8_t cycles = 0;
  cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
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
  return cycles;
}
// AND - Logical And
uint8_t AND(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t cycles = 0;
  cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
  cpu->AC &= memory[memAddr];
  if (cpu->AC == 0) {
    cpu->SR.Zero = true;
  }
  if (cpu->AC & BIT7) {
    cpu->SR.Negative = true;
  }
  return cycles;
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

uint8_t BCC(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t cycles = 0;
  if (cpu->SR.Carry == false) {
    // plus one cycle if page crosssed
    cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

uint8_t BCS(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t cycles = 0;
  if (cpu->SR.Carry == true) {
    // plus one cycle if page crosssed
    cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}
uint8_t BEQ(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t cycles = 0;
  if (cpu->SR.Zero == true) {
    // plus one cycle if page crosssed
    cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

// Bit Test - Accumulator is Anded with the memAddr and then the zero flag is
// set
void BIT(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  cpu->SR.Zero = cpu->AC & memory[memAddr];
  cpu->SR.Overflow = cpu->AC & memory[memAddr] & BIT6;
  cpu->SR.Negative = cpu->AC & memory[memAddr] & BIT7;
}

uint8_t BMI(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t cycles = 0;
  if (cpu->SR.Negative == true) {
    // plus one cycle if page crosssed
    cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

uint8_t BNE(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t cycles = 0;
  if (cpu->SR.Zero == false) {
    // plus one cycle if page crosssed
    cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

uint8_t BPL(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t cycles = 0;
  if (cpu->SR.Negative == false) {
    // plus one cycle if page crosssed
    cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}
// THIS ONE COULD BE FUCKED
// CHECK IT LATER
void BRK(cpu_t *cpu, uint8_t *memory) {
  cpu->SR.Break = true;
  memory[cpu->SP] = cpu->PC;
  cpu->SP--;
  memory[cpu->SP] = combine_SR(cpu->SR);
  cpu->SP--;
  cpu->PC = ((uint16_t)memory[0xFFFE] << 8) + (uint16_t)memory[0xFFFF];
}

uint8_t BVC(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t cycles = 0;
  if (cpu->SR.Overflow == false) {
    // plus one cycle if page crosssed
    cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

uint8_t BVS(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t cycles = 0;
  if (cpu->SR.Overflow == true) {
    // plus one cycle if page crosssed
    cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

void CLC(cpu_t *cpu) { cpu->SR.Carry = false; }
void CLD(cpu_t *cpu) { cpu->SR.Decimal = false; }
void CLI(cpu_t *cpu) { cpu->SR.Interrupt = false; }
void CLV(cpu_t *cpu) { cpu->SR.Overflow = false; }

uint8_t CMP(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  if (cpu->AC >= memory[memAddr]) {
    cpu->SR.Carry = true;
  }
  if (cpu->AC == memory[memAddr]) {
    cpu->SR.Zero = true;
  }
  if (cpu->AC & BIT7) {
    cpu->SR.Negative = true;
  }
  uint8_t cycles = 0;
  cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
  return cycles;
}

uint8_t CMX(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  if (cpu->X >= memory[memAddr]) {
    cpu->SR.Carry = true;
  }
  if (cpu->X == memory[memAddr]) {
    cpu->SR.Zero = true;
  }
  if (cpu->X & BIT7) {
    cpu->SR.Negative = true;
  }
  uint8_t cycles = 0;
  cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
  return cycles;
}

uint8_t CMY(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  if (cpu->Y >= memory[memAddr]) {
    cpu->SR.Carry = true;
  }
  if (cpu->Y == memory[memAddr]) {
    cpu->SR.Zero = true;
  }
  if (cpu->X & BIT7) {
    cpu->SR.Negative = true;
  }
  uint8_t cycles = 0;
  cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
  return cycles;
}
// LDA - load Accumulator
uint8_t LDA(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory) {
  uint16_t memAddr = fetchAddrMode(addr_mode, cpu, memory);
  uint8_t cycles = 0;
  cycles += pageCrossed(addr_mode, cpu, memory, memAddr);
  cpu->AC = memory[memAddr];
  if (cpu->AC == 0) {
    cpu->SR.Zero = true;
  }
  if (cpu->AC & BIT7) {
    cpu->SR.Negative = true;
  }
  return cycles;
}

// gets the correct memory address to be used by the corresponding opcode
// returns a 16 bit integer containing the propper memory address
int run(cpu_t *cpu, uint8_t *memory) {
  static instruction_t *instr;
  if (unlikely(instr == NULL))
    instr = create_opcodes();
  uint8_t opcode = memory[cpu->PC];
  char *name = instr[opcode].name;
  addressing_mode_t addr_mode = instr[opcode].addr_mode;
  uint8_t cycles = instr[opcode].cycles;
  if (!strcmp(name, "LDA"))
    cycles += LDA(addr_mode, cpu, memory);
  else if (!strcmp(name, "ADC"))
    cycles += ADC(addr_mode, cpu, memory);
  else if (!strcmp(name, "AND"))
    cycles += AND(addr_mode, cpu, memory);
  else if (!strcmp(name, "ASL"))
    ASL(addr_mode, cpu, memory);
  else if (!strcmp(name, "BCC"))
    cycles += BCC(addr_mode, cpu, memory);
  else if (!strcmp(name, "BCS"))
    cycles += BCS(addr_mode, cpu, memory);
  else if (!strcmp(name, "BEQ"))
    cycles += BEQ(addr_mode, cpu, memory);
  else if (!strcmp(name, "BIT"))
    BIT(addr_mode, cpu, memory);
  else if (!strcmp(name, "BMI"))
    cycles += BMI(addr_mode, cpu, memory);
  else if (!strcmp(name, "BNE"))
    cycles += BNE(addr_mode, cpu, memory);
  else if (!strcmp(name, "BPL"))
    cycles += BPL(addr_mode, cpu, memory);
  else if (!strcmp(name, "BRK"))
    BRK(cpu, memory);
  else if (!strcmp(name, "BVC"))
    cycles += BVC(addr_mode, cpu, memory);
  else if (!strcmp(name, "BVS"))
    cycles += BVS(addr_mode, cpu, memory);
  else if (!strcmp(name, "CLC"))
    CLC(cpu);
  else if (!strcmp(name, "CLD"))
    CLD(cpu);
  else if (!strcmp(name, "CLI"))
    CLI(cpu);
  else if (!strcmp(name, "CLV"))
    CLV(cpu);
  else if (!strcmp(name, "CMP"))
    cycles += CMP(addr_mode, cpu, memory);
  else if (!strcmp(name, "CMX"))
    cycles += CMX(addr_mode, cpu, memory);
  else if (!strcmp(name, "CMY"))
    cycles += CMY(addr_mode, cpu, memory);
  cpu->PC++;
  return cycles;
}

bool hasPageCrossed(uint16_t addr1, uint16_t addr2) {
  return (addr1 & 0xFF00) != (addr2 & 0xFF00);
}

uint8_t pageCrossed(addressing_mode_t addr_mode, cpu_t *cpu, uint8_t *memory,
                    uint16_t memAddr) {
  switch (addr_mode) {
  case ABSOLUTEX:
    return 1 & hasPageCrossed(memAddr - cpu->X, memAddr);
    break;
  case (ABSOLUTEY || INDIRECTY):
    return 1 & hasPageCrossed(memAddr - cpu->Y, memAddr);
    break;
  case RELATIVE:
    return 1 & hasPageCrossed(memAddr, memAddr - memory[cpu->PC]);
    break;
  default:
    return 0;
    break;
  }
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
    param = cpu->PC + memory[cpu->PC + 1];
    cpu->PC += 1;
    break;
  }
  default:
    printf("Invalid Addressing mode: %d\n", addr_mode);
    break;
  }
  return param;
}

uint8_t combine_SR(status_t SR) {
  uint8_t combine = 0;
  combine += SR.Negative << 7;
  combine += SR.Overflow << 6;
  combine += 0 & BIT5;
  combine += SR.Break << 4;
  combine += SR.Interrupt << 3;
  combine += SR.Zero << 2;
  combine += SR.Carry;
  return combine;
}

instruction_t *create_opcodes() {
  static instruction_t instr[256];
  // ADC
  instr[0x69] = (instruction_t){"ADC", 0x69, IMMEDIATE, 2};
  instr[0x65] = (instruction_t){"ADC", 0x65, ZEROPAGE, 3};
  instr[0x75] = (instruction_t){"ADC", 0x75, ZEROPAGEX, 4};
  instr[0x6D] = (instruction_t){"ADC", 0x6D, ABSOLUTE, 4};
  instr[0x7D] = (instruction_t){"ADC", 0x7D, ABSOLUTEX, 4};
  instr[0x79] = (instruction_t){"ADC", 0x79, ABSOLUTEY, 4};
  instr[0x61] = (instruction_t){"ADC", 0x61, INDIRECTX, 6};
  instr[0x71] = (instruction_t){"ADC", 0x71, INDIRECTY, 5};
  // AND
  instr[0x29] = (instruction_t){"AND", 0x29, IMMEDIATE, 2};
  instr[0x25] = (instruction_t){"AND", 0x29, ZEROPAGE, 3};
  instr[0x35] = (instruction_t){"AND", 0x29, ZEROPAGEX, 4};
  instr[0x2D] = (instruction_t){"AND", 0x29, ABSOLUTE, 4};
  instr[0x3D] = (instruction_t){"AND", 0x29, ABSOLUTEX, 4};
  instr[0x39] = (instruction_t){"AND", 0x29, ABSOLUTEY, 4};
  instr[0x21] = (instruction_t){"AND", 0x29, INDIRECTX, 6};
  instr[0x31] = (instruction_t){"AND", 0x29, INDIRECTY, 5};
  // ASL
  instr[0x0A] = (instruction_t){"ASL", 0x0A, ACCUMULATOR, 2};
  instr[0x06] = (instruction_t){"ASL", 0x06, ZEROPAGE, 5};
  instr[0x16] = (instruction_t){"ASL", 0x16, ZEROPAGEX, 6};
  instr[0x0E] = (instruction_t){"ASL", 0x0E, ABSOLUTE, 6};
  instr[0x1E] = (instruction_t){"ASL", 0x1E, ABSOLUTEX, 7};
  // BCC
  instr[0x90] = (instruction_t){"BCC", 0x90, RELATIVE, 2};
  // BCS
  instr[0xB0] = (instruction_t){"BCS", 0xB0, RELATIVE, 2};
  // BEQ
  instr[0xF0] = (instruction_t){"BEQ", 0xF0, RELATIVE, 2};
  // BIT
  instr[0x24] = (instruction_t){"BIT", 0x24, ZEROPAGE, 3};
  instr[0x2C] = (instruction_t){"BIT", 0x2C, ZEROPAGEX, 4};
  // BMI
  instr[0x30] = (instruction_t){"BMI", 0x30, RELATIVE, 2};
  // BNE
  instr[0xD0] = (instruction_t){"BNE", 0xD0, RELATIVE, 2};
  // BPL
  instr[0x10] = (instruction_t){"BPL", 0x10, RELATIVE, 2};
  // BRK
  instr[0x00] = (instruction_t){"BRK", 0x00, IMPLIED, 7};
  // BVC
  instr[0x50] = (instruction_t){"BVC", 0x50, RELATIVE, 2};
  // CLC
  instr[0x70] = (instruction_t){"CLC", 0x70, IMPLIED, 2};
  // CLD
  instr[0xD8] = (instruction_t){"CLD", 0xD8, IMPLIED, 2};
  // CLI
  instr[0x58] = (instruction_t){"CLI", 0x58, IMPLIED, 2};
  // CLV
  instr[0xB8] = (instruction_t){"CLV", 0xB8, IMPLIED, 2};
  // CMP
  instr[0xC9] = (instruction_t){"CMP", 0xC9, IMMEDIATE, 2};
  instr[0xC5] = (instruction_t){"CMP", 0xC5, ZEROPAGE, 3};
  instr[0xD5] = (instruction_t){"CMP", 0xD5, ZEROPAGEX, 4};
  instr[0xCD] = (instruction_t){"CMP", 0xCD, ABSOLUTE, 4};
  instr[0xDD] = (instruction_t){"CMP", 0xDD, ABSOLUTEX, 4};
  instr[0xD9] = (instruction_t){"CMP", 0xD9, ABSOLUTEY, 4};
  instr[0xC1] = (instruction_t){"CMP", 0xC1, INDIRECTX, 6};
  instr[0xD1] = (instruction_t){"CMP", 0xD1, INDIRECTY, 5};
  // CPX
  instr[0xE0] = (instruction_t){"CPX", 0xE0, IMMEDIATE, 2};
  instr[0xE4] = (instruction_t){"CPX", 0xE4, ZEROPAGE, 3};
  instr[0xEC] = (instruction_t){"CPX", 0xEC, ABSOLUTE, 4};
  // CPY
  instr[0xC0] = (instruction_t){"CPY", 0xC0, IMMEDIATE, 2};
  instr[0xC4] = (instruction_t){"CPY", 0xC0, ZEROPAGE, 3};
  instr[0xCC] = (instruction_t){"CPY", 0xC0, ABSOLUTE, 4};
  // DEC
  instr[0xC6] = (instruction_t){"DEC", 0xC6, ZEROPAGE, 5};
  instr[0xD6] = (instruction_t){"DEC", 0xD6, ZEROPAGEX, 6};
  instr[0xCE] = (instruction_t){"DEC", 0xCE, ABSOLUTE, 6};
  instr[0xDE] = (instruction_t){"DEC", 0xDE, ABSOLUTEX, 7};
  // DEX
  instr[0xCA] = (instruction_t){"DEX", 0xCA, IMPLIED, 2};
  // DEY
  instr[0x88] = (instruction_t){"DEY", 0x88, IMPLIED, 2};
  // EOR
  instr[0x49] = (instruction_t){"EOR", 0x49, IMMEDIATE, 2};
  instr[0x45] = (instruction_t){"EOR", 0x45, ZEROPAGE, 3};
  instr[0x55] = (instruction_t){"EOR", 0x55, ZEROPAGEX, 4};
  instr[0x4D] = (instruction_t){"EOR", 0x4D, ABSOLUTE, 4};
  instr[0x5D] = (instruction_t){"EOR", 0x5D, ABSOLUTEX, 4};
  instr[0x59] = (instruction_t){"EOR", 0x59, ABSOLUTEY, 4};
  instr[0x41] = (instruction_t){"EOR", 0x59, ABSOLUTEY, 6};
  instr[0x51] = (instruction_t){"EOR", 0x59, ABSOLUTEY, 5};
  // INC
  instr[0xE6] = (instruction_t){"INC", 0xE6, ZEROPAGE, 5};
  instr[0xF6] = (instruction_t){"INC", 0xF6, ZEROPAGEX, 6};
  instr[0xEE] = (instruction_t){"INC", 0xEE, ABSOLUTE, 6};
  instr[0xFE] = (instruction_t){"INC", 0xFE, ABSOLUTEX, 7};
  // INX
  instr[0xE8] = (instruction_t){"INX", 0xE8, IMPLIED, 2};
  // INY
  instr[0xC8] = (instruction_t){"INY", 0xC8, IMPLIED, 2};
  // JMP
  instr[0x4C] = (instruction_t){"JMP", 0x4C, ABSOLUTE, 3};
  instr[0x6C] = (instruction_t){"JMP", 0x6C, INDIRECT, 5};
  // JSR
  instr[0x20] = (instruction_t){"JSR", 0x20, ABSOLUTE, 6};
  // LDA
  instr[0xA9] = (instruction_t){"LDA", 0xA9, IMMEDIATE, 2};
  instr[0xA5] = (instruction_t){"LDA", 0xA5, ZEROPAGE, 3};
  instr[0xB5] = (instruction_t){"LDA", 0xB5, ZEROPAGEX, 4};
  instr[0xAD] = (instruction_t){"LDA", 0xAD, ABSOLUTE, 4};
  instr[0xBD] = (instruction_t){"LDA", 0xBD, ABSOLUTEX, 4};
  instr[0xB9] = (instruction_t){"LDA", 0xB9, ABSOLUTEY, 4};
  instr[0xA1] = (instruction_t){"LDA", 0xA1, INDIRECTX, 6};
  instr[0xB1] = (instruction_t){"LDA", 0xB1, INDIRECTY, 5};
  // LDX
  instr[0xA2] = (instruction_t){"LDX", 0xA2, IMMEDIATE, 2};
  instr[0xA6] = (instruction_t){"LDX", 0xA6, ZEROPAGE, 3};
  instr[0xB6] = (instruction_t){"LDX", 0xB6, ZEROPAGEY, 4};
  instr[0xAE] = (instruction_t){"LDX", 0xAE, ABSOLUTE, 4};
  instr[0xBE] = (instruction_t){"LDX", 0xBE, ABSOLUTEY, 4};
  // LDY
  instr[0xA0] = (instruction_t){"LDY", 0xA0, IMMEDIATE, 2};
  instr[0xA4] = (instruction_t){"LDY", 0xA4, ZEROPAGE, 3};
  instr[0xB4] = (instruction_t){"LDY", 0xB4, ZEROPAGEX, 4};
  instr[0xAC] = (instruction_t){"LDY", 0xAC, ABSOLUTE, 4};
  instr[0xBC] = (instruction_t){"LDY", 0xBC, ABSOLUTEX, 4};
  // LSR
  instr[0x4A] = (instruction_t){"LSR", 0x4A, ACCUMULATOR, 2};
  instr[0x46] = (instruction_t){"LSR", 0x46, ZEROPAGE, 5};
  instr[0x56] = (instruction_t){"LSR", 0x56, ZEROPAGEX, 6};
  instr[0x4E] = (instruction_t){"LSR", 0x4E, ABSOLUTE, 6};
  instr[0x5E] = (instruction_t){"LSR", 0x5E, ABSOLUTEX, 7};
  // NOP
  instr[0xEA] = (instruction_t){"NOP", 0xEA, IMPLIED, 2};
  // ORA
  instr[0x09] = (instruction_t){"ORA", 0x09, IMMEDIATE, 2};
  instr[0x05] = (instruction_t){"ORA", 0x09, IMMEDIATE, 3};
  instr[0x15] = (instruction_t){"ORA", 0x09, IMMEDIATE, 4};
  instr[0x0D] = (instruction_t){"ORA", 0x09, IMMEDIATE, 4};
  instr[0x1D] = (instruction_t){"ORA", 0x09, IMMEDIATE, 4};
  instr[0x19] = (instruction_t){"ORA", 0x09, IMMEDIATE, 4};
  instr[0x01] = (instruction_t){"ORA", 0x09, IMMEDIATE, 6};
  instr[0x11] = (instruction_t){"ORA", 0x09, IMMEDIATE, 5};
  // PHA
  instr[0x48] = (instruction_t){"PHA", 0x48, IMPLIED, 3};
  // PHP
  instr[0x08] = (instruction_t){"PHP", 0x08, IMPLIED, 3};
  // PLA
  instr[0x68] = (instruction_t){"PLA", 0x68, IMPLIED, 4};
  // PLP
  instr[0x28] = (instruction_t){"PLP", 0x28, IMPLIED, 4};
  // ROL
  instr[0x2A] = (instruction_t){"ROL", 0x2A, ACCUMULATOR, 2};
  instr[0x26] = (instruction_t){"ROL", 0x26, ZEROPAGE, 5};
  instr[0x36] = (instruction_t){"ROL", 0x36, ZEROPAGEX, 6};
  instr[0x2E] = (instruction_t){"ROL", 0x2E, ABSOLUTE, 6};
  instr[0x3E] = (instruction_t){"ROL", 0x3E, ABSOLUTEX, 7};
  // ROR
  instr[0x6A] = (instruction_t){"ROR", 0x6A, ACCUMULATOR, 2};
  instr[0x66] = (instruction_t){"ROR", 0x66, ZEROPAGE, 5};
  instr[0x76] = (instruction_t){"ROR", 0x76, ZEROPAGEX, 6};
  instr[0x6E] = (instruction_t){"ROR", 0x6E, ABSOLUTE, 6};
  instr[0x7E] = (instruction_t){"ROR", 0x7E, ABSOLUTEX, 7};
  // RTI
  instr[0x40] = (instruction_t){"RTI", 0x40, IMPLIED, 6};
  // RTS
  instr[0x60] = (instruction_t){"RTS", 0x60, IMPLIED, 6};
  // SBC
  instr[0xE9] = (instruction_t){"SBC", 0xE9, IMMEDIATE, 2};
  instr[0xE5] = (instruction_t){"SBC", 0xE5, ZEROPAGE, 3};
  instr[0xF5] = (instruction_t){"SBC", 0xF5, ZEROPAGEX, 4};
  instr[0xED] = (instruction_t){"SBC", 0xED, ABSOLUTE, 4};
  instr[0xFD] = (instruction_t){"SBC", 0xFD, ABSOLUTEX, 4};
  instr[0xF9] = (instruction_t){"SBC", 0xF9, ABSOLUTEY, 4};
  instr[0xE1] = (instruction_t){"SBC", 0xE1, INDIRECTX, 6};
  instr[0xF1] = (instruction_t){"SBC", 0xF1, INDIRECTY, 5};
  // SEC
  instr[0x38] = (instruction_t){"SEC", 0x38, IMPLIED, 2};
  // SED
  instr[0xF8] = (instruction_t){"SED", 0xF8, IMPLIED, 2};
  // SEI
  instr[0x78] = (instruction_t){"SEI", 0x78, IMPLIED, 2};
  // STA
  instr[0x85] = (instruction_t){"STA", 0x85, ZEROPAGE, 3};
  instr[0x95] = (instruction_t){"STA", 0x95, ZEROPAGEX, 4};
  instr[0x8D] = (instruction_t){"STA", 0x8D, ABSOLUTE, 4};
  instr[0x9D] = (instruction_t){"STA", 0x9D, ABSOLUTEX, 5};
  instr[0x99] = (instruction_t){"STA", 0x99, ABSOLUTEY, 5};
  instr[0x81] = (instruction_t){"STA", 0x81, INDIRECTX, 6};
  instr[0x91] = (instruction_t){"STA", 0x91, INDIRECTY, 6};
  // STX
  instr[0x86] = (instruction_t){"STX", 0x86, ZEROPAGE, 3};
  instr[0x96] = (instruction_t){"STX", 0x96, ZEROPAGEY, 4};
  instr[0x8E] = (instruction_t){"STX", 0x8E, ABSOLUTE, 4};
  // STY
  instr[0x84] = (instruction_t){"STY", 0x84, ZEROPAGE, 3};
  instr[0x94] = (instruction_t){"STY", 0x94, ZEROPAGEX, 4};
  instr[0x8C] = (instruction_t){"STY", 0x8C, ABSOLUTE, 4};
  // TAX
  instr[0xAA] = (instruction_t){"TAX", 0xAA, IMPLIED, 2};
  // TAY
  instr[0xA8] = (instruction_t){"TAY", 0xA8, IMPLIED, 2};
  // TSX
  instr[0xBA] = (instruction_t){"TSX", 0xB8, IMPLIED, 2};
  // TXA
  instr[0x8A] = (instruction_t){"TXA", 0x8A, IMPLIED, 2};
  // TXS
  instr[0x9A] = (instruction_t){"TXS", 0x9A, IMPLIED, 2};
  // TYA
  instr[0x98] = (instruction_t){"TYA", 0x98, IMPLIED, 2};
  return instr;
}
