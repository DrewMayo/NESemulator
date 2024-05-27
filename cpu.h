#include <stdint.h>
#define CPURAMSIZE 65536
typedef struct {
  uint8_t AC;  // Accumulator Register
  uint8_t X;   // X register
  uint8_t Y;   // Y register
  uint8_t SP;  // stack pointer from $0100 to $01FF
  uint8_t SR;  // Status Register
  uint16_t PC; // Program Counter
  uint16_t ram[CPURAMSIZE]; // The systems ram
} cpu6502;

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
