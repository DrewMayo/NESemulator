#include "cpu.h"
#include "bitmask.h"
#include "bus.h"
#include "test.h"
#include <stdint.h>

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)

uint8_t cpu_combine_SR(struct status_reg SR);
void cpu_expand_SR(struct cpu_6502 *cpu, uint8_t SR);
uint8_t page_crossed(const enum addr_mode_states addr_mode, const struct cpu_6502 *cpu, uint16_t mem_addr);
uint16_t fetch_addr_mode(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu);
uint8_t NMI(struct cpu_6502 *cpu);
uint8_t IRQ(struct cpu_6502 *cpu);
void cpu_reset(struct cpu_6502 *cpu);
struct instruction *create_opcodes();

// This is the main tick of the CPU

uint8_t cpu_run(struct cpu_6502 *const cpu) {
  uint8_t cycles = 0;
  cycles += IRQ(cpu);
  if (cpu->interrupt_state == INONMASKABLE) {
    cycles += NMI(cpu);
    printf("NMI\n");
    cpu->interrupt_state = NOINTERRUPT;
    return cycles;
  }
  // check for non-maskable interrupt coming from the PPU
  const uint8_t opcode = bus_read(cpu->bus, cpu->PC, CPUMEM);
  cycles += cpu->instr[opcode].cycles;
  // run the output
  // testCpuPart(*cpu, cpu->instr[opcode]);
  // assert(cpu->instr[opcode].fp_instruction != NULL);
  if (cpu->instr[opcode].fp_instruction == NULL) {
    return 0;
  }
  cycles += cpu->instr[opcode].fp_instruction(cpu->instr[opcode].addr_mode, cpu);
  cpu->PC++;
  cpu->cycles += cycles;

  // handle interrupts in phase 2
  return cycles;
}

struct cpu_6502 *cpu_build() {
  struct cpu_6502 *cpu = (struct cpu_6502 *)malloc(sizeof(struct cpu_6502));
  cpu->instr = create_opcodes();
  for (int i = 0; i < 0xFFFF; i++) {
    cpu->memory[i] = 0;
  }
  return cpu;
}

void cpu_reset(struct cpu_6502 *cpu) {
  cpu->AC = 0;
  cpu->X = 0;
  cpu->Y = 0;
  cpu->SP = 0xFD;
  cpu_expand_SR(cpu, 0x24);
  cpu->PC = (((uint16_t)(bus_read(cpu->bus, 0xFFFD, CPUMEM)) << 8) & 0xFF00) +
            ((uint16_t)(bus_read(cpu->bus, 0xFFFC, CPUMEM)));
  cpu->cycles = 7;
}

uint8_t NMI(struct cpu_6502 *cpu) {
  cpu->SR.Break = 0;
  bus_write(cpu->bus, cpu->SP + 0x0100, (cpu->PC & 0xFF00) >> 8, CPUMEM);
  cpu->SP--;
  bus_write(cpu->bus, cpu->SP + 0x0100, cpu->PC & 0x00FF, CPUMEM);
  cpu->SP--;
  bus_write(cpu->bus, cpu->SP + 0x0100, cpu_combine_SR(cpu->SR), CPUMEM);
  cpu->SP--;
  cpu->PC = (((uint16_t)(bus_read(cpu->bus, 0xFFFB, CPUMEM)) << 8) & 0xFF00) +
            ((uint16_t)(bus_read(cpu->bus, 0xFFFA, CPUMEM)));
  return 7;
}

uint8_t IRQ(struct cpu_6502 *cpu) {
  if (cpu->SR.Interrupt == 0) {
    cpu->SR.Break = 0;
    cpu->SR.Interrupt = 1;
    bus_write(cpu->bus, cpu->SP + 0x0100, (cpu->PC & 0xFF00) >> 8, CPUMEM);
    cpu->SP--;
    bus_write(cpu->bus, cpu->SP + 0x0100, cpu->PC & 0x00FF, CPUMEM);
    cpu->SP--;
    bus_write(cpu->bus, cpu->SP + 0x0100, cpu_combine_SR(cpu->SR), CPUMEM);
    cpu->SP--;
    cpu->PC = (((uint16_t)(bus_read(cpu->bus, 0xFFFF, CPUMEM)) << 8) & 0xFF00) +
              ((uint16_t)(bus_read(cpu->bus, 0xFFFE, CPUMEM)));
    return 7;
  }
  return 0;
}
// ADC - ADD with Carry
uint8_t ADC(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  const uint16_t mem = (uint16_t)bus_read(cpu->bus, mem_addr, CPUMEM);
  const uint16_t AC = (uint16_t)cpu->AC;
  const uint16_t carry = (uint16_t)cpu->SR.Carry;
  const uint16_t added = mem + AC + carry;
  const uint8_t cycles = page_crossed(addr_mode, cpu, mem_addr);
  cpu->AC = (uint8_t)added;
  cpu->SR.Carry = (added > 255);
  // formula for determining overflow
  // (result ^ M) & (result ^ mem) & BIT7
  cpu->SR.Overflow = (AC ^ added) & (added ^ mem) & BIT7;
  cpu->SR.Zero = (cpu->AC == 0);
  cpu->SR.Negative = (cpu->AC & BIT7);
  return cycles;
}
// AND - Logical And
uint8_t AND(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  const uint8_t cycles = page_crossed(addr_mode, cpu, mem_addr);
  cpu->AC &= bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Zero = (cpu->AC == 0);
  cpu->SR.Negative = cpu->AC & BIT7;
  return cycles;
}
// ASL - Arithmetic Shift Left
uint8_t ASL(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t value = bus_read(cpu->bus, mem_addr, CPUMEM);
  uint8_t *ptr = (addr_mode == ACCUMULATOR) ? &cpu->AC : &value;
  cpu->SR.Carry = *ptr & BIT7;
  *ptr *= 2;
  cpu->SR.Zero = *ptr == 0;
  cpu->SR.Negative = *ptr & BIT7;
  if (addr_mode != ACCUMULATOR) {
    bus_write(cpu->bus, mem_addr, value, CPUMEM);
  }
  return 0;
}

uint8_t BCC(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t cycles = 0;
  if (cpu->SR.Carry == false) {
    // plus one cycle if page crosssed
    cycles += page_crossed(addr_mode, cpu, mem_addr);
    cpu->PC = mem_addr;
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

uint8_t BCS(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t cycles = 0;
  if (cpu->SR.Carry == true) {
    // plus one cycle if page crosssed
    cycles += page_crossed(addr_mode, cpu, mem_addr);
    cpu->PC = mem_addr;
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}
uint8_t BEQ(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t cycles = 0;
  if (cpu->SR.Zero == true) {
    // plus one cycle if page crosssed
    cycles += page_crossed(addr_mode, cpu, mem_addr);
    cpu->PC = mem_addr;
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

// Bit Test - Accumulator is Anded with the mem_addr and then the zero flag is
// set
uint8_t BIT(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  cpu->SR.Zero = (cpu->AC & bus_read(cpu->bus, mem_addr, CPUMEM)) == 0;
  cpu->SR.Overflow = bus_read(cpu->bus, mem_addr, CPUMEM) & BIT6;
  cpu->SR.Negative = bus_read(cpu->bus, mem_addr, CPUMEM) & BIT7;
  return 0;
}

uint8_t BMI(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t cycles = 0;
  if (cpu->SR.Negative == true) {
    // plus one cycle if page crosssed
    cycles += page_crossed(addr_mode, cpu, mem_addr);
    cpu->PC = mem_addr;
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

uint8_t BNE(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t cycles = 0;
  if (cpu->SR.Zero == false) {
    // plus one cycle if page crosssed
    cycles += page_crossed(addr_mode, cpu, mem_addr);
    cpu->PC = mem_addr;
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

uint8_t BPL(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t cycles = 0;
  if (cpu->SR.Negative == false) {
    // plus one cycle if page crosssed
    cycles += page_crossed(addr_mode, cpu, mem_addr);
    cpu->PC = mem_addr;
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

uint8_t BRK(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  bus_write(cpu->bus, cpu->SP + 0x0100, (cpu->PC & 0xFF00) >> 8, CPUMEM);
  cpu->SP--;
  bus_write(cpu->bus, cpu->SP + 0x0100, cpu->PC & 0x00FF, CPUMEM);
  cpu->SP--;
  bus_write(cpu->bus, cpu->SP + 0x0100, cpu_combine_SR(cpu->SR), CPUMEM);
  cpu->SP--;
  cpu->PC = (((uint16_t)(bus_read(cpu->bus, 0xFFFE, CPUMEM)) << 8) & 0xFF00) +
            ((uint16_t)(bus_read(cpu->bus, 0xFFFF, CPUMEM)));
  cpu->SR.Break = 0;
  cpu->SR.Interrupt = 1;
  return 0;
}

uint8_t BVC(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t cycles = 0;
  if (cpu->SR.Overflow == false) {
    // plus one cycle if page crosssed
    cycles += page_crossed(addr_mode, cpu, mem_addr);
    cpu->PC = mem_addr;
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

uint8_t BVS(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t cycles = 0;
  if (cpu->SR.Overflow == true) {
    // plus one cycle if page crosssed
    cycles += page_crossed(addr_mode, cpu, mem_addr);
    cpu->PC = mem_addr;
    // on sucess add one cycles
    cycles += 1;
  }
  return cycles;
}

uint8_t CLC(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SR.Carry = false;
  return 0;
}

uint8_t CLD(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SR.Decimal = false;
  return 0;
}
uint8_t CLI(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SR.Interrupt = false;
  return 0;
}
uint8_t CLV(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SR.Overflow = false;
  return 0;
}

uint8_t CMP(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  const uint8_t cycles = page_crossed(addr_mode, cpu, mem_addr);
  cpu->SR.Carry = cpu->AC >= bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Zero = cpu->AC == bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Negative = (cpu->AC - bus_read(cpu->bus, mem_addr, CPUMEM)) & BIT7;
  return cycles;
}

uint8_t CPX(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  const uint8_t cycles = page_crossed(addr_mode, cpu, mem_addr);
  cpu->SR.Carry = cpu->X >= bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Zero = cpu->X == bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Negative = (cpu->X - bus_read(cpu->bus, mem_addr, CPUMEM)) & BIT7;
  return cycles;
}

uint8_t CPY(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  const uint8_t cycles = page_crossed(addr_mode, cpu, mem_addr);
  cpu->SR.Carry = cpu->Y >= bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Zero = cpu->Y == bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Negative = (cpu->Y - bus_read(cpu->bus, mem_addr, CPUMEM)) & BIT7;
  return cycles;
}

uint8_t DEC(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  bus_write(cpu->bus, mem_addr, bus_read(cpu->bus, mem_addr, CPUMEM) - 1, CPUMEM);

  cpu->SR.Zero = bus_read(cpu->bus, mem_addr, CPUMEM) == 0;
  cpu->SR.Negative = bus_read(cpu->bus, mem_addr, CPUMEM) & BIT7;
  return 0;
}

uint8_t DEX(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->X--;
  cpu->SR.Zero = cpu->X == 0;
  cpu->SR.Negative = cpu->X & BIT7;
  return 0;
}

uint8_t DEY(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->Y--;
  cpu->SR.Zero = cpu->Y == 0;
  cpu->SR.Negative = cpu->Y & BIT7;
  return 0;
}

uint8_t EOR(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  const uint8_t cycles = page_crossed(addr_mode, cpu, mem_addr);
  cpu->AC ^= bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = cpu->AC & BIT7;
  return cycles;
}

uint8_t INC(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t mem_value = bus_read(cpu->bus, mem_addr, CPUMEM);
  mem_value += 1;
  bus_write(cpu->bus, mem_addr, mem_value, CPUMEM);
  cpu->SR.Zero = mem_value == 0;
  cpu->SR.Negative = mem_value & BIT7;
  return 0;
}

uint8_t INX(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->X++;
  cpu->SR.Zero = cpu->X == 0;
  cpu->SR.Negative = cpu->X & BIT7;
  return 0;
}

uint8_t INY(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->Y++;
  cpu->SR.Zero = cpu->Y == 0;
  cpu->SR.Negative = cpu->Y & BIT7;
  return 0;
}

uint8_t JMP(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  // subtract one for the later +1 in fn run
  cpu->PC = fetch_addr_mode(addr_mode, cpu) - 1;
  return 0;
}

uint8_t JSR(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  bus_write(cpu->bus, cpu->SP + 0x0100, cpu->PC >> 8, CPUMEM);
  cpu->SP--;
  bus_write(cpu->bus, cpu->SP + 0x0100, cpu->PC & 0xFF, CPUMEM);
  cpu->SP--;
  cpu->PC = mem_addr;
  cpu->PC--;
  return 0;
}

// LDA - load Accumulator
uint8_t LDA(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  const uint8_t cycles = page_crossed(addr_mode, cpu, mem_addr);
  cpu->AC = bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = cpu->AC & BIT7;
  return cycles;
}

uint8_t LDX(enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  const uint8_t cycles = page_crossed(addr_mode, cpu, mem_addr);
  cpu->X = bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Zero = cpu->X == 0;
  cpu->SR.Negative = cpu->X & BIT7;
  return cycles;
}

uint8_t LDY(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  const uint8_t cycles = page_crossed(addr_mode, cpu, mem_addr);
  cpu->Y = bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Zero = cpu->Y == 0;
  cpu->SR.Negative = cpu->Y & BIT7;
  return cycles;
}

uint8_t LSR(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t value = bus_read(cpu->bus, mem_addr, CPUMEM);
  uint8_t *ptr = (addr_mode == ACCUMULATOR) ? &cpu->AC : &value;
  cpu->SR.Carry = *ptr & BIT0;
  *ptr /= 2;
  cpu->SR.Zero = *ptr == 0;
  cpu->SR.Negative = *ptr & BIT7;
  if (addr_mode != ACCUMULATOR) {
    bus_write(cpu->bus, mem_addr, value, CPUMEM);
  }
  return 0;
}
uint8_t NOP(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  if (addr_mode == ABSOLUTEX) {
    return page_crossed(addr_mode, cpu, mem_addr);
  }
  return 0;
}

uint8_t ORA(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  cpu->AC = cpu->AC | bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = cpu->AC & BIT7;
  uint8_t cycles = 0;
  cycles += page_crossed(addr_mode, cpu, mem_addr);
  return cycles;
}

uint8_t PHA(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  bus_write(cpu->bus, 0x0100 + cpu->SP, cpu->AC, CPUMEM);
  cpu->SP--;
  return 0;
}

uint8_t PHP(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SR.Break = true;
  bus_write(cpu->bus, 0x0100 + cpu->SP, cpu_combine_SR(cpu->SR), CPUMEM);
  cpu->SR.Break = false;
  cpu->SP--;
  return 0;
}

uint8_t PLA(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SP++;
  cpu->AC = bus_read(cpu->bus, 0x0100 + cpu->SP, CPUMEM);
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = cpu->AC & BIT7;
  return 0;
}

uint8_t PLP(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SP++;
  // NOTICE: THIS CHANGES THE SR
  cpu_expand_SR(cpu, bus_read(cpu->bus, 0x0100 + cpu->SP, CPUMEM));
  cpu->SR.Break = false;
  return 0;
}

uint8_t ROL(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t mem_value = bus_read(cpu->bus, mem_addr, CPUMEM);
  uint8_t *const ptr = (addr_mode == ACCUMULATOR) ? &cpu->AC : &mem_value;
  const uint8_t old_bit_7 = *ptr & BIT7;
  *ptr = (*ptr << 1) + cpu->SR.Carry;
  cpu->SR.Carry = old_bit_7;
  cpu->SR.Zero = *ptr == 0;
  cpu->SR.Negative = *ptr & BIT7;
  if (addr_mode != ACCUMULATOR) {
    bus_write(cpu->bus, mem_addr, *ptr, CPUMEM);
  }
  return 0;
}

uint8_t ROR(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t mem_value = bus_read(cpu->bus, mem_addr, CPUMEM);
  uint8_t *const ptr = (addr_mode == ACCUMULATOR) ? &cpu->AC : &mem_value;
  const uint8_t old_bit_7 = *ptr & BIT0;
  *ptr = (*ptr >> 1) + (cpu->SR.Carry << 7);
  cpu->SR.Carry = old_bit_7 != 0;
  cpu->SR.Zero = *ptr == 0;
  cpu->SR.Negative = *ptr & BIT7;
  if (addr_mode != ACCUMULATOR) {
    bus_write(cpu->bus, mem_addr, *ptr, CPUMEM);
  }
  return 0;
}

uint8_t RTI(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SP++;
  cpu_expand_SR(cpu, bus_read(cpu->bus, 0x0100 + cpu->SP, CPUMEM));
  cpu->PC = ((uint16_t)bus_read(cpu->bus, 0x0100 + cpu->SP + 2, CPUMEM) << 8) +
            bus_read(cpu->bus, 0x0100 + cpu->SP + 1, CPUMEM) - 1;
  cpu->SP += 2;
  return 0;
}

uint8_t RTS(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->PC = ((uint16_t)bus_read(cpu->bus, 0x0100 + cpu->SP + 2, CPUMEM) << 8) +
            bus_read(cpu->bus, 0x0100 + cpu->SP + 1, CPUMEM);
  cpu->SP += 2;
  return 0;
}

uint8_t SBC(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  const uint16_t mem = (uint16_t)bus_read(cpu->bus, mem_addr, CPUMEM);
  const uint16_t AC = (uint16_t)cpu->AC;
  const uint16_t carry = (uint16_t)cpu->SR.Carry;
  const uint16_t added = AC - mem - (1 - carry);
  const uint8_t cycles = page_crossed(addr_mode, cpu, mem_addr);
  cpu->AC = (uint8_t)added;
  cpu->SR.Carry = !(added & 0xFF00);
  // formula for determining overflow
  // (result ^ M) & (result ^ mem) & BIT7
  cpu->SR.Overflow = (AC ^ added) & (added ^ ~mem) & BIT7;
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = (cpu->AC & BIT7);
  return cycles;
}

uint8_t SEC(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SR.Carry = true;
  return 0;
}

uint8_t SED(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SR.Decimal = true;
  return 0;
}

uint8_t SEI(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SR.Interrupt = true;
  return 0;
}

uint8_t STA(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  bus_write(cpu->bus, mem_addr, cpu->AC, CPUMEM);
  return 0;
}

uint8_t STX(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  bus_write(cpu->bus, mem_addr, cpu->X, CPUMEM);
  return 0;
}

uint8_t STY(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  bus_write(cpu->bus, mem_addr, cpu->Y, CPUMEM);
  return 0;
}

uint8_t TAX(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->X = cpu->AC;
  cpu->SR.Zero = cpu->X == 0;
  cpu->SR.Negative = cpu->X & BIT7;
  return 0;
}

uint8_t TAY(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->Y = cpu->AC;
  cpu->SR.Zero = cpu->Y == 0;
  cpu->SR.Negative = cpu->Y & BIT7;
  return 0;
}

uint8_t TSX(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->X = cpu->SP;
  cpu->SR.Zero = cpu->X == 0;
  cpu->SR.Negative = cpu->X & BIT7;
  return 0;
}

uint8_t TXA(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->AC = cpu->X;
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = cpu->AC & BIT7;
  return 0;
}

uint8_t TXS(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->SP = cpu->X;

  return 0;
}

uint8_t TYA(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  (void)addr_mode;
  cpu->AC = cpu->Y;
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = cpu->AC & BIT7;
  return 0;
}

// UNOFFICIAL OPCODES

uint8_t LAX(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  cpu->AC = bus_read(cpu->bus, mem_addr, CPUMEM);
  cpu->X = cpu->AC;
  cpu->SR.Zero = cpu->X == 0;
  cpu->SR.Negative = cpu->X & BIT7;
  return page_crossed(addr_mode, cpu, mem_addr);
}

uint8_t SAX(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  bus_write(cpu->bus, mem_addr, cpu->AC & cpu->X, CPUMEM);
  return 0;
}

uint8_t DCP(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t mem_value = bus_read(cpu->bus, mem_addr, CPUMEM);
  mem_value--;
  bus_write(cpu->bus, mem_addr, mem_value, CPUMEM);
  cpu->SR.Carry = cpu->AC >= mem_value;
  cpu->SR.Zero = cpu->AC == mem_value;
  cpu->SR.Negative = (cpu->AC - mem_value) & BIT7;
  return 0;
}

uint8_t ISB(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint16_t mem_value = bus_read(cpu->bus, mem_addr, CPUMEM);
  mem_value++;
  bus_write(cpu->bus, mem_addr, mem_value, CPUMEM);
  const uint16_t mem = (uint16_t)bus_read(cpu->bus, mem_addr, CPUMEM);
  const uint16_t AC = (uint16_t)cpu->AC;
  const uint16_t carry = (uint16_t)cpu->SR.Carry;
  const uint16_t added = AC - mem - (1 - carry);
  cpu->AC = (uint8_t)added;
  cpu->SR.Carry = !(added & 0xFF00);
  // formula for determining overflow
  // (result ^ M) & (result ^ mem) & BIT7
  cpu->SR.Overflow = (AC ^ added) & (added ^ ~mem) & BIT7;
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = (cpu->AC & BIT7);
  return 0;
}

uint8_t RLA(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t mem_value = bus_read(cpu->bus, mem_addr, CPUMEM);
  uint8_t *const ptr = (addr_mode == ACCUMULATOR) ? &cpu->AC : &mem_value;
  const bool old_carry_bit = *ptr & BIT7;
  *ptr = (*ptr << 1) + cpu->SR.Carry;
  cpu->AC = cpu->AC & mem_value;
  cpu->SR.Carry = old_carry_bit;
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = (cpu->AC & BIT7);
  if (addr_mode != ACCUMULATOR) {
    bus_write(cpu->bus, mem_addr, mem_value, CPUMEM);
  }
  return 0;
}

uint8_t RRA(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t mem_value = bus_read(cpu->bus, mem_addr, CPUMEM);
  uint8_t *const ptr = (addr_mode == ACCUMULATOR) ? &cpu->AC : &mem_value;
  const uint8_t old_bit_7 = *ptr & BIT0;
  *ptr = (*ptr >> 1) + (cpu->SR.Carry << 7);
  cpu->SR.Carry = old_bit_7 != 0;
  const uint16_t mem = (uint16_t)mem_value;
  const uint16_t AC = (uint16_t)cpu->AC;
  const uint16_t carry = (uint16_t)cpu->SR.Carry;
  const uint16_t added = mem + AC + carry;
  cpu->AC = (uint8_t)added;
  cpu->SR.Carry = (added > 255);
  // formula for determining overflow
  // (result ^ M) & (result ^ mem) & BIT7
  cpu->SR.Overflow = (AC ^ added) & (added ^ mem) & BIT7;
  cpu->SR.Zero = (cpu->AC == 0);
  cpu->SR.Negative = (cpu->AC & BIT7);
  if (addr_mode != ACCUMULATOR) {
    bus_write(cpu->bus, mem_addr, mem_value, CPUMEM);
  }
  return 0;
}

uint8_t SLO(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t mem_value = bus_read(cpu->bus, mem_addr, CPUMEM);
  uint8_t *const ptr = (addr_mode == ACCUMULATOR) ? &cpu->AC : &mem_value;
  cpu->SR.Carry = *ptr & BIT7;
  *ptr *= 2;
  cpu->AC = cpu->AC | mem_value;
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = (cpu->AC & BIT7);
  if (addr_mode != ACCUMULATOR) {
    bus_write(cpu->bus, mem_addr, *ptr, CPUMEM);
  }
  return 0;
}

uint8_t SRE(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
  const uint16_t mem_addr = fetch_addr_mode(addr_mode, cpu);
  uint8_t mem_value = bus_read(cpu->bus, mem_addr, CPUMEM);
  uint8_t *const ptr = (addr_mode == ACCUMULATOR) ? &cpu->AC : &mem_value;
  cpu->SR.Carry = *ptr & BIT0;
  *ptr >>= 1;
  cpu->AC = cpu->AC ^ mem_value;
  cpu->SR.Zero = cpu->AC == 0;
  cpu->SR.Negative = (cpu->AC & BIT7);
  if (addr_mode != ACCUMULATOR) {
    bus_write(cpu->bus, mem_addr, mem_value, CPUMEM);
  }
  return 0;
}

// gets the correct cpu->memory address to be used by the corresponding opcode
// returns a 16 bit integer containing the propper cpu->memory address

bool is_page_crossed(const uint16_t addr1, const uint16_t addr2) { return (addr1 & 0xFF00) != (addr2 & 0xFF00); }

uint8_t page_crossed(const enum addr_mode_states addr_mode, const struct cpu_6502 *cpu, uint16_t mem_addr) {
  switch (addr_mode) {
  case ABSOLUTEX:
    return is_page_crossed(mem_addr - cpu->X, mem_addr);
    break;
  case ABSOLUTEY:
  case INDIRECTY: {
    return is_page_crossed(mem_addr - cpu->Y, mem_addr);
    break;
  }
  case RELATIVE: {
    // from the next PC
    mem_addr += 1;
    if (bus_read(cpu->bus, cpu->PC, CPUMEM) < 128) {
      return is_page_crossed(mem_addr, mem_addr - (uint16_t)bus_read(cpu->bus, cpu->PC, CPUMEM));
    } else {
      return is_page_crossed(mem_addr, mem_addr + (uint8_t)(~(bus_read(cpu->bus, cpu->PC, CPUMEM)) + 1));
    }
    break;
  }
  default: {
    return 0;
    break;
  }
  }
}

uint16_t fetch_addr_mode(const enum addr_mode_states addr_mode, struct cpu_6502 *cpu) {
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
    const uint8_t mem_access = bus_read(cpu->bus, cpu->PC + 1, CPUMEM);
    cpu->PC += 1;
    param = (uint8_t)mem_access;
    break;
  }
  case ZEROPAGEX: {
    const uint8_t mem_access = bus_read(cpu->bus, cpu->PC + 1, CPUMEM);
    cpu->PC += 1;
    param = (uint8_t)(mem_access + cpu->X);
    break;
  }
  case ZEROPAGEY: {
    const uint8_t mem_access = bus_read(cpu->bus, cpu->PC + 1, CPUMEM);
    cpu->PC += 1;
    // printf("memaccess: %x\n", cpu->PC);
    param = (uint8_t)(mem_access + cpu->Y);
    break;
  }
  case ABSOLUTE: {
    const uint16_t mem_access =
        bus_read(cpu->bus, cpu->PC + 1, CPUMEM) + ((uint16_t)bus_read(cpu->bus, cpu->PC + 2, CPUMEM) << 8);
    param = mem_access;
    cpu->PC += 2;
    break;
  }
  case ABSOLUTEX: {
    const uint16_t mem_access =
        bus_read(cpu->bus, cpu->PC + 1, CPUMEM) + ((uint16_t)bus_read(cpu->bus, cpu->PC + 2, CPUMEM) << 8);
    param = mem_access + cpu->X;
    cpu->PC += 2;
    break;
  }
  case ABSOLUTEY: {
    const uint16_t mem_access =
        bus_read(cpu->bus, cpu->PC + 1, CPUMEM) + ((uint16_t)bus_read(cpu->bus, cpu->PC + 2, CPUMEM) << 8);
    param = mem_access + cpu->Y;
    cpu->PC += 2;
    break;
  }
  case INDIRECT: {
    const uint16_t mem_access =
        bus_read(cpu->bus, cpu->PC + 1, CPUMEM) + (((uint16_t)bus_read(cpu->bus, cpu->PC + 2, CPUMEM)) << 8);
    param = ((uint16_t)bus_read(cpu->bus, mem_access + 1, CPUMEM) << 8) + bus_read(cpu->bus, mem_access, CPUMEM);
    // handle the bug in the indirect vector goes to the same
    // page when crossing the page boundry
    if (((mem_access) & 0xFF) == 0xFF) {
      param = ((uint16_t)bus_read(cpu->bus, mem_access & 0xFF00, CPUMEM) << 8) + bus_read(cpu->bus, mem_access, CPUMEM);
    }
    cpu->PC += 2;
    break;
  }
  case INDIRECTX: {
    const uint8_t mem_access = bus_read(cpu->bus, cpu->PC + 1, CPUMEM);
    cpu->PC += 1;
    //&ed with 0xFF because of sign extensions
    param = ((uint16_t)bus_read(cpu->bus, (mem_access + cpu->X + 1) & 0xFF, CPUMEM)) << 8;
    param += bus_read(cpu->bus, (mem_access + cpu->X) & 0xFF, CPUMEM);
    break;
  }
  case INDIRECTY: {
    const uint8_t mem_access = bus_read(cpu->bus, cpu->PC + 1, CPUMEM);
    cpu->PC += 1;
    //&ed with 0xFF because of sign extensions
    param = ((uint16_t)bus_read(cpu->bus, (mem_access + 1) & 0xFF, CPUMEM)) << 8;
    param += bus_read(cpu->bus, mem_access & 0xFF, CPUMEM) + cpu->Y;
    break;
  }
  case RELATIVE: {
    int8_t add = bus_read(cpu->bus, cpu->PC + 1, CPUMEM);
    // relative is signed so we must check
    // whether it is greater than 127 and if so
    // we make it negative by noting it and subtracting one
    if (bus_read(cpu->bus, cpu->PC + 1, CPUMEM) > 0x7F) {
      add = -1 * ~(bus_read(cpu->bus, cpu->PC + 1, CPUMEM) - 1);
    }
    param = cpu->PC + add + 1;
    cpu->PC += 1;
    break;
  }
  default:
    printf("Invalid Addressing mode: %d\n", addr_mode);
    break;
  }
  return param;
}

uint8_t cpu_combine_SR(const struct status_reg SR) {
  uint8_t combine = 0;
  combine += SR.Negative << 7;
  combine += SR.Overflow << 6;
  combine += 0xFF & BIT5;
  combine += SR.Break << 4;
  combine += SR.Decimal << 3;
  combine += SR.Interrupt << 2;
  combine += SR.Zero << 1;
  combine += SR.Carry;
  return combine;
}

void cpu_expand_SR(struct cpu_6502 *cpu, const uint8_t SR) {
  cpu->SR.Negative = SR & BIT7;
  cpu->SR.Overflow = SR & BIT6;
  cpu->SR.Break = SR & BIT4;
  cpu->SR.Decimal = SR & BIT3;
  cpu->SR.Interrupt = SR & BIT2;
  cpu->SR.Zero = SR & BIT1;
  cpu->SR.Carry = SR & BIT0;
}

struct instruction *create_opcodes() {
  static struct instruction instr[256];
  // In thoery it should never encounter this
  for (int i = 0; i < 256; i++) {
    instr[i] = (struct instruction){"XXX", 0, 0, NULL};
  }
  // ADC
  instr[0x69] = (struct instruction){"ADC", IMMEDIATE, 2, &ADC};
  instr[0x65] = (struct instruction){"ADC", ZEROPAGE, 3, &ADC};
  instr[0x75] = (struct instruction){"ADC", ZEROPAGEX, 4, &ADC};
  instr[0x6D] = (struct instruction){"ADC", ABSOLUTE, 4, &ADC};
  instr[0x7D] = (struct instruction){"ADC", ABSOLUTEX, 4, &ADC};
  instr[0x79] = (struct instruction){"ADC", ABSOLUTEY, 4, &ADC};
  instr[0x61] = (struct instruction){"ADC", INDIRECTX, 6, &ADC};
  instr[0x71] = (struct instruction){"ADC", INDIRECTY, 5, &ADC};
  // AND
  instr[0x29] = (struct instruction){"AND", IMMEDIATE, 2, &AND};
  instr[0x25] = (struct instruction){"AND", ZEROPAGE, 3, &AND};
  instr[0x35] = (struct instruction){"AND", ZEROPAGEX, 4, &AND};
  instr[0x2D] = (struct instruction){"AND", ABSOLUTE, 4, &AND};
  instr[0x3D] = (struct instruction){"AND", ABSOLUTEX, 4, &AND};
  instr[0x39] = (struct instruction){"AND", ABSOLUTEY, 4, &AND};
  instr[0x21] = (struct instruction){"AND", INDIRECTX, 6, &AND};
  instr[0x31] = (struct instruction){"AND", INDIRECTY, 5, &AND};
  // ASL
  instr[0x0A] = (struct instruction){"ASL", ACCUMULATOR, 2, &ASL};
  instr[0x06] = (struct instruction){"ASL", ZEROPAGE, 5, &ASL};
  instr[0x16] = (struct instruction){"ASL", ZEROPAGEX, 6, &ASL};
  instr[0x0E] = (struct instruction){"ASL", ABSOLUTE, 6, &ASL};
  instr[0x1E] = (struct instruction){"ASL", ABSOLUTEX, 7, &ASL};
  // BCC
  instr[0x90] = (struct instruction){"BCC", RELATIVE, 2, &BCC};
  // BCS
  instr[0xB0] = (struct instruction){"BCS", RELATIVE, 2, &BCS};
  // BEQ
  instr[0xF0] = (struct instruction){"BEQ", RELATIVE, 2, &BEQ};
  // BIT
  instr[0x24] = (struct instruction){"BIT", ZEROPAGE, 3, &BIT};
  instr[0x2C] = (struct instruction){"BIT", ABSOLUTE, 4, &BIT};
  // BMI
  instr[0x30] = (struct instruction){"BMI", RELATIVE, 2, &BMI};
  // BNE
  instr[0xD0] = (struct instruction){"BNE", RELATIVE, 2, &BNE};
  // BPL
  instr[0x10] = (struct instruction){"BPL", RELATIVE, 2, &BPL};
  // BRK
  instr[0x00] = (struct instruction){"BRK", IMPLIED, 7, &BRK};
  // BVC
  instr[0x50] = (struct instruction){"BVC", RELATIVE, 2, &BVC};
  // BVS
  instr[0x70] = (struct instruction){"BVS", RELATIVE, 2, &BVS};
  // CLC
  instr[0x18] = (struct instruction){"CLC", IMPLIED, 2, &CLC};
  // CLD
  instr[0xD8] = (struct instruction){"CLD", IMPLIED, 2, &CLD};
  // CLI
  instr[0x58] = (struct instruction){"CLI", IMPLIED, 2, &CLI};
  // CLV
  instr[0xB8] = (struct instruction){"CLV", IMPLIED, 2, &CLV};
  // CMP
  instr[0xC9] = (struct instruction){"CMP", IMMEDIATE, 2, &CMP};
  instr[0xC5] = (struct instruction){"CMP", ZEROPAGE, 3, &CMP};
  instr[0xD5] = (struct instruction){"CMP", ZEROPAGEX, 4, &CMP};
  instr[0xCD] = (struct instruction){"CMP", ABSOLUTE, 4, &CMP};
  instr[0xDD] = (struct instruction){"CMP", ABSOLUTEX, 4, &CMP};
  instr[0xD9] = (struct instruction){"CMP", ABSOLUTEY, 4, &CMP};
  instr[0xC1] = (struct instruction){"CMP", INDIRECTX, 6, &CMP};
  instr[0xD1] = (struct instruction){"CMP", INDIRECTY, 5, &CMP};
  // CPX
  instr[0xE0] = (struct instruction){"CPX", IMMEDIATE, 2, &CPX};
  instr[0xE4] = (struct instruction){"CPX", ZEROPAGE, 3, &CPX};
  instr[0xEC] = (struct instruction){"CPX", ABSOLUTE, 4, &CPX};
  // CPY
  instr[0xC0] = (struct instruction){"CPY", IMMEDIATE, 2, &CPY};
  instr[0xC4] = (struct instruction){"CPY", ZEROPAGE, 3, &CPY};
  instr[0xCC] = (struct instruction){"CPY", ABSOLUTE, 4, &CPY};
  // DEC
  instr[0xC6] = (struct instruction){"DEC", ZEROPAGE, 5, &DEC};
  instr[0xD6] = (struct instruction){"DEC", ZEROPAGEX, 6, &DEC};
  instr[0xCE] = (struct instruction){"DEC", ABSOLUTE, 6, &DEC};
  instr[0xDE] = (struct instruction){"DEC", ABSOLUTEX, 7, &DEC};
  // DEX
  instr[0xCA] = (struct instruction){"DEX", IMPLIED, 2, &DEX};
  // DEY
  instr[0x88] = (struct instruction){"DEY", IMPLIED, 2, &DEY};
  // EOR
  instr[0x49] = (struct instruction){"EOR", IMMEDIATE, 2, &EOR};
  instr[0x45] = (struct instruction){"EOR", ZEROPAGE, 3, &EOR};
  instr[0x55] = (struct instruction){"EOR", ZEROPAGEX, 4, &EOR};
  instr[0x4D] = (struct instruction){"EOR", ABSOLUTE, 4, &EOR};
  instr[0x5D] = (struct instruction){"EOR", ABSOLUTEX, 4, &EOR};
  instr[0x59] = (struct instruction){"EOR", ABSOLUTEY, 4, &EOR};
  instr[0x41] = (struct instruction){"EOR", INDIRECTX, 6, &EOR};
  instr[0x51] = (struct instruction){"EOR", INDIRECTY, 5, &EOR};
  // INC
  instr[0xE6] = (struct instruction){"INC", ZEROPAGE, 5, &INC};
  instr[0xF6] = (struct instruction){"INC", ZEROPAGEX, 6, &INC};
  instr[0xEE] = (struct instruction){"INC", ABSOLUTE, 6, &INC};
  instr[0xFE] = (struct instruction){"INC", ABSOLUTEX, 7, &INC};
  // INX
  instr[0xE8] = (struct instruction){"INX", IMPLIED, 2, &INX};
  // INY
  instr[0xC8] = (struct instruction){"INY", IMPLIED, 2, &INY};
  // JMP
  instr[0x4C] = (struct instruction){"JMP", ABSOLUTE, 3, &JMP};
  instr[0x6C] = (struct instruction){"JMP", INDIRECT, 5, &JMP};
  // JSR
  instr[0x20] = (struct instruction){"JSR", ABSOLUTE, 6, &JSR};
  // LDA
  instr[0xA9] = (struct instruction){"LDA", IMMEDIATE, 2, &LDA};
  instr[0xA5] = (struct instruction){"LDA", ZEROPAGE, 3, &LDA};
  instr[0xB5] = (struct instruction){"LDA", ZEROPAGEX, 4, &LDA};
  instr[0xAD] = (struct instruction){"LDA", ABSOLUTE, 4, &LDA};
  instr[0xBD] = (struct instruction){"LDA", ABSOLUTEX, 4, &LDA};
  instr[0xB9] = (struct instruction){"LDA", ABSOLUTEY, 4, &LDA};
  instr[0xA1] = (struct instruction){"LDA", INDIRECTX, 6, &LDA};
  instr[0xB1] = (struct instruction){"LDA", INDIRECTY, 5, &LDA};
  // LDX
  instr[0xA2] = (struct instruction){"LDX", IMMEDIATE, 2, &LDX};
  instr[0xA6] = (struct instruction){"LDX", ZEROPAGE, 3, &LDX};
  instr[0xB6] = (struct instruction){"LDX", ZEROPAGEY, 4, &LDX};
  instr[0xAE] = (struct instruction){"LDX", ABSOLUTE, 4, &LDX};
  instr[0xBE] = (struct instruction){"LDX", ABSOLUTEY, 4, &LDX};
  // LDY
  instr[0xA0] = (struct instruction){"LDY", IMMEDIATE, 2, &LDY};
  instr[0xA4] = (struct instruction){"LDY", ZEROPAGE, 3, &LDY};
  instr[0xB4] = (struct instruction){"LDY", ZEROPAGEX, 4, &LDY};
  instr[0xAC] = (struct instruction){"LDY", ABSOLUTE, 4, &LDY};
  instr[0xBC] = (struct instruction){"LDY", ABSOLUTEX, 4, &LDY};
  // LSR
  instr[0x4A] = (struct instruction){"LSR", ACCUMULATOR, 2, &LSR};
  instr[0x46] = (struct instruction){"LSR", ZEROPAGE, 5, &LSR};
  instr[0x56] = (struct instruction){"LSR", ZEROPAGEX, 6, &LSR};
  instr[0x4E] = (struct instruction){"LSR", ABSOLUTE, 6, &LSR};
  instr[0x5E] = (struct instruction){"LSR", ABSOLUTEX, 7, &LSR};
  // NOP
  instr[0xEA] = (struct instruction){"NOP", IMPLIED, 2, &NOP};
  // ORA
  instr[0x09] = (struct instruction){"ORA", IMMEDIATE, 2, &ORA};
  instr[0x05] = (struct instruction){"ORA", ZEROPAGE, 3, &ORA};
  instr[0x15] = (struct instruction){"ORA", ZEROPAGEX, 4, &ORA};
  instr[0x0D] = (struct instruction){"ORA", ABSOLUTE, 4, &ORA};
  instr[0x1D] = (struct instruction){"ORA", ABSOLUTEX, 4, &ORA};
  instr[0x19] = (struct instruction){"ORA", ABSOLUTEY, 4, &ORA};
  instr[0x01] = (struct instruction){"ORA", INDIRECTX, 6, &ORA};
  instr[0x11] = (struct instruction){"ORA", INDIRECTY, 5, &ORA};
  // PHA
  instr[0x48] = (struct instruction){"PHA", IMPLIED, 3, &PHA};
  // PHP
  instr[0x08] = (struct instruction){"PHP", IMPLIED, 3, &PHP};
  // PLA
  instr[0x68] = (struct instruction){"PLA", IMPLIED, 4, &PLA};
  // PLP
  instr[0x28] = (struct instruction){"PLP", IMPLIED, 4, &PLP};
  // ROL
  instr[0x2A] = (struct instruction){"ROL", ACCUMULATOR, 2, &ROL};
  instr[0x26] = (struct instruction){"ROL", ZEROPAGE, 5, &ROL};
  instr[0x36] = (struct instruction){"ROL", ZEROPAGEX, 6, &ROL};
  instr[0x2E] = (struct instruction){"ROL", ABSOLUTE, 6, &ROL};
  instr[0x3E] = (struct instruction){"ROL", ABSOLUTEX, 7, &ROL};
  // ROR
  instr[0x6A] = (struct instruction){"ROR", ACCUMULATOR, 2, &ROR};
  instr[0x66] = (struct instruction){"ROR", ZEROPAGE, 5, &ROR};
  instr[0x76] = (struct instruction){"ROR", ZEROPAGEX, 6, &ROR};
  instr[0x6E] = (struct instruction){"ROR", ABSOLUTE, 6, &ROR};
  instr[0x7E] = (struct instruction){"ROR", ABSOLUTEX, 7, &ROR};
  // RTI
  instr[0x40] = (struct instruction){"RTI", IMPLIED, 6, &RTI};
  // RTS
  instr[0x60] = (struct instruction){"RTS", IMPLIED, 6, &RTS};
  // SBC
  instr[0xE9] = (struct instruction){"SBC", IMMEDIATE, 2, &SBC};
  instr[0xE5] = (struct instruction){"SBC", ZEROPAGE, 3, &SBC};
  instr[0xF5] = (struct instruction){"SBC", ZEROPAGEX, 4, &SBC};
  instr[0xED] = (struct instruction){"SBC", ABSOLUTE, 4, &SBC};
  instr[0xFD] = (struct instruction){"SBC", ABSOLUTEX, 4, &SBC};
  instr[0xF9] = (struct instruction){"SBC", ABSOLUTEY, 4, &SBC};
  instr[0xE1] = (struct instruction){"SBC", INDIRECTX, 6, &SBC};
  instr[0xF1] = (struct instruction){"SBC", INDIRECTY, 5, &SBC};
  // SEC
  instr[0x38] = (struct instruction){"SEC", IMPLIED, 2, &SEC};
  // SED
  instr[0xF8] = (struct instruction){"SED", IMPLIED, 2, &SED};
  // SEI
  instr[0x78] = (struct instruction){"SEI", IMPLIED, 2, &SEI};
  // STA
  instr[0x85] = (struct instruction){"STA", ZEROPAGE, 3, &STA};
  instr[0x95] = (struct instruction){"STA", ZEROPAGEX, 4, &STA};
  instr[0x8D] = (struct instruction){"STA", ABSOLUTE, 4, &STA};
  instr[0x9D] = (struct instruction){"STA", ABSOLUTEX, 5, &STA};
  instr[0x99] = (struct instruction){"STA", ABSOLUTEY, 5, &STA};
  instr[0x81] = (struct instruction){"STA", INDIRECTX, 6, &STA};
  instr[0x91] = (struct instruction){"STA", INDIRECTY, 6, &STA};
  // STX
  instr[0x86] = (struct instruction){"STX", ZEROPAGE, 3, &STX};
  instr[0x96] = (struct instruction){"STX", ZEROPAGEY, 4, &STX};
  instr[0x8E] = (struct instruction){"STX", ABSOLUTE, 4, &STX};
  // STY
  instr[0x84] = (struct instruction){"STY", ZEROPAGE, 3, &STY};
  instr[0x94] = (struct instruction){"STY", ZEROPAGEX, 4, &STY};
  instr[0x8C] = (struct instruction){"STY", ABSOLUTE, 4, &STY};
  // TAX
  instr[0xAA] = (struct instruction){"TAX", IMPLIED, 2, &TAX};
  // TAY
  instr[0xA8] = (struct instruction){"TAY", IMPLIED, 2, &TAY};
  // TSX
  instr[0xBA] = (struct instruction){"TSX", IMPLIED, 2, &TSX};
  // TXA
  instr[0x8A] = (struct instruction){"TXA", IMPLIED, 2, &TXA};
  // TXS
  instr[0x9A] = (struct instruction){"TXS", IMPLIED, 2, &TXS};
  // TYA
  instr[0x98] = (struct instruction){"TYA", IMPLIED, 2, &TYA};
  // UNOFFICIAL OPCODES
  // LAX
  instr[0xA7] = (struct instruction){"LAX", ZEROPAGE, 3, &LAX};
  instr[0xB7] = (struct instruction){"LAX", ZEROPAGEY, 4, &LAX};
  instr[0xA3] = (struct instruction){"LAX", INDIRECTX, 6, &LAX};
  instr[0xB3] = (struct instruction){"LAX", INDIRECTY, 5, &LAX};
  instr[0xAF] = (struct instruction){"LAX", ABSOLUTE, 4, &LAX};
  instr[0xBF] = (struct instruction){"LAX", ABSOLUTEY, 4, &LAX};
  // SAX
  instr[0x87] = (struct instruction){"SAX", ZEROPAGE, 3, &SAX};
  instr[0x97] = (struct instruction){"SAX", ZEROPAGEY, 4, &SAX};
  instr[0x83] = (struct instruction){"SAX", INDIRECTX, 6, &SAX};
  instr[0x8F] = (struct instruction){"SAX", ABSOLUTE, 4, &SAX};
  // DCP
  instr[0xC3] = (struct instruction){"DCP", INDIRECTX, 8, &DCP};
  instr[0xC7] = (struct instruction){"DCP", ZEROPAGE, 5, &DCP};
  instr[0xCF] = (struct instruction){"DCP", ABSOLUTE, 6, &DCP};
  instr[0xD3] = (struct instruction){"DCP", INDIRECTY, 8, &DCP};
  instr[0xD7] = (struct instruction){"DCP", ZEROPAGEX, 6, &DCP};
  instr[0xDB] = (struct instruction){"DCP", ABSOLUTEY, 7, &DCP};
  instr[0xDF] = (struct instruction){"DCP", ABSOLUTEX, 7, &DCP};
  // ISB
  instr[0xE3] = (struct instruction){"ISB", INDIRECTX, 8, &ISB};
  instr[0xE7] = (struct instruction){"ISB", ZEROPAGE, 5, &ISB};
  instr[0xEF] = (struct instruction){"ISB", ABSOLUTE, 6, &ISB};
  instr[0xF3] = (struct instruction){"ISB", INDIRECTY, 8, &ISB};
  instr[0xF7] = (struct instruction){"ISB", ZEROPAGEX, 6, &ISB};
  instr[0xFB] = (struct instruction){"ISB", ABSOLUTEY, 7, &ISB};
  instr[0xFF] = (struct instruction){"ISB", ABSOLUTEX, 7, &ISB};
  // RLA
  instr[0x23] = (struct instruction){"RLA", INDIRECTX, 8, &RLA};
  instr[0x27] = (struct instruction){"RLA", ZEROPAGE, 5, &RLA};
  instr[0x2F] = (struct instruction){"RLA", ABSOLUTE, 6, &RLA};
  instr[0x33] = (struct instruction){"RLA", INDIRECTY, 8, &RLA};
  instr[0x37] = (struct instruction){"RLA", ZEROPAGEX, 6, &RLA};
  instr[0x3B] = (struct instruction){"RLA", ABSOLUTEY, 7, &RLA};
  instr[0x3F] = (struct instruction){"RLA", ABSOLUTEX, 7, &RLA};
  // RRA
  instr[0x63] = (struct instruction){"RRA", INDIRECTX, 8, &RRA};
  instr[0x67] = (struct instruction){"RRA", ZEROPAGE, 5, &RRA};
  instr[0x6F] = (struct instruction){"RRA", ABSOLUTE, 6, &RRA};
  instr[0x73] = (struct instruction){"RRA", INDIRECTY, 8, &RRA};
  instr[0x77] = (struct instruction){"RRA", ZEROPAGEX, 6, &RRA};
  instr[0x7B] = (struct instruction){"RRA", ABSOLUTEY, 7, &RRA};
  instr[0x7F] = (struct instruction){"RRA", ABSOLUTEX, 7, &RRA};
  // SLO
  instr[0x03] = (struct instruction){"SLO", INDIRECTX, 8, &SLO};
  instr[0x07] = (struct instruction){"SLO", ZEROPAGE, 5, &SLO};
  instr[0x0F] = (struct instruction){"SLO", ABSOLUTE, 6, &SLO};
  instr[0x13] = (struct instruction){"SLO", INDIRECTY, 8, &SLO};
  instr[0x17] = (struct instruction){"SLO", ZEROPAGEX, 6, &SLO};
  instr[0x1B] = (struct instruction){"SLO", ABSOLUTEY, 7, &SLO};
  instr[0x1F] = (struct instruction){"SLO", ABSOLUTEX, 7, &SLO};
  // SRE
  instr[0x43] = (struct instruction){"SRE", INDIRECTX, 8, &SRE};
  instr[0x47] = (struct instruction){"SRE", ZEROPAGE, 5, &SRE};
  instr[0x4F] = (struct instruction){"SRE", ABSOLUTE, 6, &SRE};
  instr[0x53] = (struct instruction){"SRE", INDIRECTY, 8, &SRE};
  instr[0x57] = (struct instruction){"SRE", ZEROPAGEX, 6, &SRE};
  instr[0x5B] = (struct instruction){"SRE", ABSOLUTEY, 7, &SRE};
  instr[0x5F] = (struct instruction){"SRE", ABSOLUTEX, 7, &SRE};
  // NOP
  instr[0x1A] = (struct instruction){"NOP", IMPLIED, 2, &NOP};
  instr[0x3A] = (struct instruction){"NOP", IMPLIED, 2, &NOP};
  instr[0x5A] = (struct instruction){"NOP", IMPLIED, 2, &NOP};
  instr[0x7A] = (struct instruction){"NOP", IMPLIED, 2, &NOP};
  instr[0xDA] = (struct instruction){"NOP", IMPLIED, 2, &NOP};
  instr[0xFA] = (struct instruction){"NOP", IMPLIED, 2, &NOP};
  // OTHER NOPS -- EG IGN
  instr[0x0C] = (struct instruction){"NOP", ABSOLUTE, 4, &NOP};
  instr[0x1C] = (struct instruction){"NOP", ABSOLUTEX, 4, &NOP};
  instr[0x3C] = (struct instruction){"NOP", ABSOLUTEX, 4, &NOP};
  instr[0x5C] = (struct instruction){"NOP", ABSOLUTEX, 4, &NOP};
  instr[0x7C] = (struct instruction){"NOP", ABSOLUTEX, 4, &NOP};
  instr[0xDC] = (struct instruction){"NOP", ABSOLUTEX, 4, &NOP};
  instr[0xFC] = (struct instruction){"NOP", ABSOLUTEX, 4, &NOP};
  instr[0x04] = (struct instruction){"NOP", ZEROPAGE, 3, &NOP};
  instr[0x44] = (struct instruction){"NOP", ZEROPAGE, 3, &NOP};
  instr[0x64] = (struct instruction){"NOP", ZEROPAGE, 3, &NOP};
  instr[0x14] = (struct instruction){"NOP", ZEROPAGEX, 4, &NOP};
  instr[0x34] = (struct instruction){"NOP", ZEROPAGEX, 4, &NOP};
  instr[0x54] = (struct instruction){"NOP", ZEROPAGEX, 4, &NOP};
  instr[0x74] = (struct instruction){"NOP", ZEROPAGEX, 4, &NOP};
  instr[0xD4] = (struct instruction){"NOP", ZEROPAGEX, 4, &NOP};
  instr[0xF4] = (struct instruction){"NOP", ZEROPAGEX, 4, &NOP};
  // more NOPS
  instr[0x80] = (struct instruction){"NOP", IMMEDIATE, 2, &NOP};
  instr[0x82] = (struct instruction){"NOP", IMMEDIATE, 2, &NOP};
  instr[0x82] = (struct instruction){"NOP", IMMEDIATE, 2, &NOP};
  // SBC -- unofficial
  instr[0xEB] = (struct instruction){"SBC", IMMEDIATE, 2, &SBC};
  return instr;
}
