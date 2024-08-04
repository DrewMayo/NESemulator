#pragma once
#ifndef CPU_H
#define CPU_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct Bus;

struct status_reg {
  bool Negative;
  bool Overflow;
  bool Break;
  bool Decimal;
  bool Interrupt;
  bool Zero;
  bool Carry;
};

enum intrpt_states {
  NOINTERRUPT,
  INONMASKABLE,
  RESET,
  IREQUEST,
};

struct cpu_6502 {
  struct status_reg SR; // status register
  uint8_t AC;           // Accumulator Register
  uint8_t X;            // X register
  uint8_t Y;            // Y register
  uint8_t SP;           // stack pointer from $0100 to $01FF
  uint16_t PC;          // Program Counter
  int cycles;
  uint8_t memory[65536];
  struct instruction *instr;
  enum intrpt_states interrupt_state;
  struct Bus *bus;
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

struct cpu_6502 *cpu_build();
uint8_t cpu_run(struct cpu_6502 *const cpu);
void cpu_reset(struct cpu_6502 *cpu);
uint8_t cpu_combine_SR(struct status_reg SR);
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
//  0xFFFB 0xFFFA ... NMI (Non-maskable Interrupt)
//  0xFFFD 0xFFFC ... RES (Reset)
//  0xFFFF 0xFFFE ... IRQ (Interupt Request)
//
#endif
