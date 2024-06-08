#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  bool Negative;
  bool Overflow;
  bool Break;
  bool Decimal;
  bool Interrupt;
  bool Zero;
  bool Carry;
} status_t;

typedef struct {
  uint8_t AC;  // Accumulator Register
  uint8_t X;   // X register
  uint8_t Y;   // Y register
  uint8_t SP;  // stack pointer from $0100 to $01FF
  status_t SR; // status register
  uint16_t PC; // Program Counter
} cpu_t;

typedef enum {
  IMPLIED,
  ACCUMULATOR,
  IMMEDIATE,
  ZEROPAGE,
  ZEROPAGEX,
  ZEROPAGEY,
  ABSOLUTE,
  ABSOLUTEX,
  ABSOLUTEY,
  ABSOLUTEINDIRECT,
  INDIRECT,
  INDIRECTX,
  INDIRECTY,
  RELATIVE,
} addressing_mode_t;

typedef enum {
  BIT0 = 0b00000001,
  BIT1 = 0b00000010,
  BIT2 = 0b00000100,
  BIT3 = 0b00001000,
  BIT4 = 0b00010000,
  BIT5 = 0b00100000,
  BIT6 = 0b01000000,
  BIT7 = 0b10000000,
} bitmasks8_t;

int run(cpu_t *cpu, uint8_t *memory);

typedef struct {
  char name[4];
  uint8_t opcode;
  addressing_mode_t addr_mode;
  uint8_t cycles;
} instruction_t;
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
