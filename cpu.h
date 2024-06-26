#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct status_reg {
  bool Negative;
  bool Overflow;
  bool Break;
  bool Decimal;
  bool Interrupt;
  bool Zero;
  bool Carry;
};

struct cpu_6502 {
  uint8_t AC;           // Accumulator Register
  uint8_t X;            // X register
  uint8_t Y;            // Y register
  uint8_t SP;           // stack pointer from $0100 to $01FF
  struct status_reg SR; // status register
  uint16_t PC;          // Program Counter
  int cycles;
  uint8_t memory[65536];
};

enum addr_mode_states {
  IMPLIED,
  ACCUMULATOR,
  IMMEDIATE,
  ZEROPAGE,
  ZEROPAGEX,
  ZEROPAGEY,
  ABSOLUTE,
  ABSOLUTEX,
  ABSOLUTEY,
  INDIRECT,
  INDIRECTX,
  INDIRECTY,
  RELATIVE,
};

struct instruction {
  char name[4];
  enum addr_mode_states addr_mode;
  uint8_t cycles;
  uint8_t (*fp_instruction)(const enum addr_mode_states, struct cpu_6502 *);
};

void cpu_run(struct cpu_6502 *cpu);
uint8_t cpu_combine_SR(const struct status_reg SR);
void cpu_expand_SR(struct cpu_6502 *cpu, const uint8_t SR);

// Status reguster bit 7 to 0
//
// N .... Negative
// V .... Overflow
// - .... Ignored
// B .... Break
// D .... Decimal
// I .... Interrupt
// Z .... zero
// C .... Carry

// NV-BDIZC
//
//
//  The stack is from 0x0100 to 0x01FF
//
//  Important Vectors
//  0xFFFA 0xFFFB ... NMI (Non-maskable Interrupt)
//  0xFFFC 0xFFFD ... RES (Reset)
//  0xFFFE 0xFFFF ... IRQ (Interupt Request)
//
#endif
