#include "test.h"
#include <stdbool.h>
bool test_LDA_one_immediate(emulator_t emu) {
  emu.memory[0x8000] = 0xA9;
  emu.memory[0x8001] = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x01);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_LDA_immediate PASSED!\n");
  return true;
}

bool test_LDA_zero_immediate(emulator_t emu) {
  emu.memory[0x8000] = 0xA9;
  emu.memory[0x8001] = 0x00;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x00);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == true);
  assert(emu.cpu.PC == 0x8002);
  printf("test_LDA_zero_immediate PASSED!\n");
  return true;
}
bool test_LDA_neg_absoulte(emulator_t emu) {
  emu.memory[0x8000] = 0xAD;
  emu.memory[0x8001] = 0x18;
  emu.memory[0x8002] = 0x81;
  emu.memory[0x8118] = 0xFF;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0xFF);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8003);
  printf("test_LDA_neg_absolute PASSED!\n");
  return true;
}
bool test_LDA_absoultex(emulator_t emu) {
  emu.memory[0x8000] = 0xBD;
  emu.memory[0x8001] = 0x18;
  emu.memory[0x8002] = 0x81;
  emu.memory[0x8119] = 0x0F;
  emu.cpu.X = 0x1;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x0F);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8003);
  printf("test_LDA_absolutex PASSED!\n");
  return true;
}
bool test_LDA_absoultey(emulator_t emu) {
  emu.memory[0x8000] = 0xB9;
  emu.memory[0x8001] = 0x12;
  emu.memory[0x8002] = 0x81;
  emu.memory[0x8114] = 0x0F;
  emu.cpu.Y = 0x2;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x0F);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8003);
  printf("test_LDA_absolutey PASSED!\n");
  return true;
}
bool test_LDA_zero_page(emulator_t emu) {
  emu.memory[0x8000] = 0xA5;
  emu.memory[0x8001] = 0x12;
  emu.memory[0x0012] = 0x8F;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x8F);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_LDA_zero_page PASSED!\n");
  return true;
}
bool test_LDA_zero_pagex(emulator_t emu) {
  emu.memory[0x8000] = 0xB5;
  emu.memory[0x8001] = 0x12;
  emu.memory[0x0011] = 0x8F;
  emu.cpu.X = 0xFF;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x8F);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_LDA_zero_pagex PASSED!\n");
  return true;
}

bool test_LDA_indirectx(emulator_t emu) {
  emu.memory[0x8000] = 0xA1;
  emu.memory[0x8001] = 0x12;
  emu.memory[0x0013] = 0xFF;
  emu.memory[0x0014] = 0xFF;
  emu.memory[0xFFFF] = 0x1F;
  emu.cpu.X = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x1F);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_LDA_indirectx PASSED!\n");
  return true;
}

bool test_LDA_indirecty(emulator_t emu) {
  emu.memory[0x8000] = 0xB1;
  emu.memory[0x8001] = 0x12;
  emu.memory[0x0012] = 0xFF;
  emu.memory[0x0013] = 0xFF;
  emu.memory[0x0000] = 0x1F;
  emu.cpu.Y = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x1F);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_LDA_indirecty PASSED!\n");
  return true;
}

bool test_ADC_immediate(emulator_t emu) {
  emu.memory[0x8000] = 0x69;
  emu.memory[0x8001] = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x01);
  assert(emu.cpu.SR.Carry == false);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.SR.Overflow == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_ADC_immediate PASSED!\n");
  return true;
}
bool test_ADC_overflow_zero_page(emulator_t emu) {
  emu.memory[0x8000] = 0x65;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x0001] = 0xFF;
  emu.cpu.AC = 0x80;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x7F);
  assert(emu.cpu.SR.Carry == true);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.SR.Overflow == true);
  assert(emu.cpu.PC == 0x8002);
  printf("test_ADC_overflow_zero_page PASSED!\n");
  return true;
}
bool test_ADC_overflow_zero_page2(emulator_t emu) {
  emu.memory[0x8000] = 0x65;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x0001] = 0x01;
  emu.cpu.AC = 0x7F;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x80);
  assert(emu.cpu.SR.Carry == false);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.SR.Overflow == true);
  assert(emu.cpu.PC == 0x8002);
  printf("test_ADC_overflow_zero_page2 PASSED!\n");
  return true;
}

bool test_ADC_overflow_zero_pagex(emulator_t emu) {
  emu.memory[0x8000] = 0x75;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x0002] = 0x01;
  emu.cpu.AC = 0x7F;
  emu.cpu.X = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x80);
  assert(emu.cpu.SR.Carry == false);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.SR.Overflow == true);
  assert(emu.cpu.PC == 0x8002);
  printf("test_ADC_overflow_zero_pagex PASSED!\n");
  return true;
}

bool test_ADC_absolute(emulator_t emu) {
  emu.memory[0x8000] = 0x6D;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x8002] = 0x03;
  emu.memory[0x0301] = 0x01;
  emu.cpu.AC = 0x8F;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x90);
  assert(emu.cpu.SR.Carry == false);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.SR.Overflow == false);
  assert(emu.cpu.PC == 0x8003);
  printf("test_ADC_absolute PASSED!\n");
  return true;
}

bool test_ADC_absolutex(emulator_t emu) {
  emu.memory[0x8000] = 0x7D;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x8002] = 0x03;
  emu.memory[0x0302] = 0x01;
  emu.cpu.AC = 0x8F;
  emu.cpu.X = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x90);
  assert(emu.cpu.SR.Carry == false);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.SR.Overflow == false);
  assert(emu.cpu.PC == 0x8003);
  printf("test_ADC_absolutex PASSED!\n");
  return true;
}

bool test_ADC_absolutey(emulator_t emu) {
  emu.memory[0x8000] = 0x79;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x8002] = 0x03;
  emu.memory[0x0400] = 0xFF;
  emu.cpu.AC = 0x8F;
  emu.cpu.Y = 0xFF;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x8E);
  assert(emu.cpu.SR.Carry == true);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.SR.Overflow == false);
  assert(emu.cpu.PC == 0x8003);
  printf("test_ADC_absolutey PASSED!\n");
  return true;
}

bool test_ADC_indirectx(emulator_t emu) {
  emu.memory[0x8000] = 0x61;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x0002] = 0xFF;
  emu.memory[0x0003] = 0xDF;
  emu.memory[0xDFFF] = 0x04;
  emu.cpu.AC = 0x8F;
  emu.cpu.X = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x93);
  assert(emu.cpu.SR.Carry == false);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.SR.Overflow == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_ADC_indirectx PASSED!\n");
  return true;
}

bool test_ADC_indirecty(emulator_t emu) {
  emu.memory[0x8000] = 0x71;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x0001] = 0xFF;
  emu.memory[0x0002] = 0xDF;
  emu.memory[0xE000] = 0x04;
  emu.cpu.AC = 0x8F;
  emu.cpu.Y = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x93);
  assert(emu.cpu.SR.Carry == false);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.SR.Overflow == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_ADC_indirecty PASSED!\n");
  return true;
}
bool test_AND_immediate(emulator_t emu) {
  emu.memory[0x8000] = 0x29;
  emu.memory[0x8001] = 0x01;
  emu.cpu.AC = 0x8F;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x01);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_AND_immediate PASSED!\n");
  return true;
}
bool test_AND_zero_page(emulator_t emu) {
  emu.memory[0x8000] = 0x25;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x0001] = 0xF0;
  emu.cpu.AC = 0x8F;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x80);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_AND_zero_page PASSED!\n");
  return true;
}
bool test_AND_zero_pagex(emulator_t emu) {
  emu.memory[0x8000] = 0x35;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x0002] = 0xF0;
  emu.cpu.AC = 0x8F;
  emu.cpu.X = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x80);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_AND_zero_pagex PASSED!\n");
  return true;
}
bool test_AND_absolute(emulator_t emu) {
  emu.memory[0x8000] = 0x2D;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x8002] = 0x81;
  emu.memory[0x8101] = 0x00;
  emu.cpu.AC = 0x8F;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x00);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == true);
  assert(emu.cpu.PC == 0x8003);
  printf("test_AND_absolute PASSED!\n");
  return true;
}

bool test_AND_absolutex(emulator_t emu) {
  emu.memory[0x8000] = 0x3D;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x8002] = 0x81;
  emu.memory[0x8102] = 0x02;
  emu.cpu.AC = 0x8F;
  emu.cpu.X = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x02);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8003);
  printf("test_AND_absolutex PASSED!\n");
  return true;
}

bool test_AND_absolutey(emulator_t emu) {
  emu.memory[0x8000] = 0x39;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x8002] = 0x81;
  emu.memory[0x8102] = 0x42;
  emu.cpu.AC = 0x8F;
  emu.cpu.Y = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x02);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8003);
  printf("test_AND_absolutey PASSED!\n");
  return true;
}

bool test_AND_indirectx(emulator_t emu) {
  emu.memory[0x8000] = 0x21;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x0002] = 0x81;
  emu.memory[0x0003] = 0x82;
  emu.memory[0x8281] = 0x42;
  emu.cpu.AC = 0x42;
  emu.cpu.X = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x42);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_AND_indirectx PASSED!\n");
  return true;
}
bool test_AND_indirecty(emulator_t emu) {
  emu.memory[0x8000] = 0x31;
  emu.memory[0x8001] = 0x01;
  emu.memory[0x0001] = 0x81;
  emu.memory[0x0002] = 0x82;
  emu.memory[0x8282] = 0x42;
  emu.cpu.AC = 0x42;
  emu.cpu.Y = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x42);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8002);
  printf("test_AND_indirecty PASSED!\n");
  return true;
}
bool test_ASL_accumulator(emulator_t emu) {
  emu.memory[0x8000] = 0x0A;
  emu.cpu.AC = 0x42;
  run(&emu.cpu, emu.memory);
  assert(emu.cpu.AC == 0x84);
  assert(emu.cpu.SR.Carry == false);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == false);
  assert(emu.cpu.PC == 0x8001);
  printf("test_ASL_accumulator PASSED!\n");
  return true;
}
bool test_ASL_zero_page(emulator_t emu) {
  emu.memory[0x8000] = 0x06;
  emu.memory[0x8001] = 0x0A;
  emu.memory[0x000A] = 0xFF;
  run(&emu.cpu, emu.memory);
  assert(emu.memory[0x000A] == 0xFE); assert(emu.cpu.SR.Carry == true);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == true);
  assert(emu.cpu.PC == 0x8002);
  printf("test_ASL_zero_page PASSED!\n");
  return true;
}

bool test_ASL_zero_pagex(emulator_t emu) {
  emu.memory[0x8000] = 0x16;
  emu.memory[0x8001] = 0x0A;
  emu.memory[0x000B] = 0xFF;
  emu.cpu.X = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.memory[0x000B] == 0xFE);
  assert(emu.cpu.SR.Carry == true);
  assert(emu.cpu.SR.Negative == true);
  assert(emu.cpu.SR.Zero == true);
  assert(emu.cpu.PC == 0x8002);
  printf("test_ASL_zero_pagex PASSED!\n");
  return true;
}

bool test_ASL_absolute(emulator_t emu) {
  emu.memory[0x8000] = 0x0E;
  emu.memory[0x8001] = 0x0A;
  emu.memory[0x8002] = 0x0D;
  emu.memory[0x0D0A] = 0x04;
  run(&emu.cpu, emu.memory);
  assert(emu.memory[0x0D0A] == 0x08);
  assert(emu.cpu.SR.Carry == false);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == true);
  assert(emu.cpu.PC == 0x8003);
  printf("test_ASL_absolute PASSED!\n");
  return true;
}

bool test_ASL_absolutex(emulator_t emu) {
  emu.memory[0x8000] = 0x1E;
  emu.memory[0x8001] = 0x0A;
  emu.memory[0x8002] = 0x0D;
  emu.memory[0x0D0B] = 0x08;
  emu.cpu.X = 0x01;
  run(&emu.cpu, emu.memory);
  assert(emu.memory[0x0D0B] == 0x10);
  assert(emu.cpu.SR.Carry == false);
  assert(emu.cpu.SR.Negative == false);
  assert(emu.cpu.SR.Zero == true);
  assert(emu.cpu.PC == 0x8003);
  printf("test_ASL_absolutex PASSED!\n");
  return true;
}

int run_unit_tests() {
  emulator_t emu;
  emu.cpu.PC = 0x8000;
  emu.cpu.AC = 0;
  emu.cpu.X = 0;
  emu.cpu.Y = 0;
  emu.cpu.SP = 0;
  emu.cpu.SR.Negative = false;
  emu.cpu.SR.Overflow = false;
  emu.cpu.SR.Break = true;
  emu.cpu.SR.Decimal = false;
  emu.cpu.SR.Interrupt = false;
  emu.cpu.SR.Zero = false;
  emu.cpu.SR.Carry = false;
  test_LDA_one_immediate(emu);
  test_LDA_zero_immediate(emu);
  test_LDA_neg_absoulte(emu);
  test_LDA_absoultex(emu);
  test_LDA_absoultey(emu);
  test_LDA_zero_page(emu);
  test_LDA_zero_pagex(emu);
  test_LDA_indirectx(emu);
  test_LDA_indirecty(emu);
  printf("===========================\n");
  printf(ANSI_COLOR_GREEN "PASSED ALL LDA TESTS!\n" ANSI_COLOR_RESET);
  printf("===========================\n");
  test_ADC_immediate(emu);
  test_ADC_overflow_zero_page(emu);
  test_ADC_overflow_zero_page2(emu);
  test_ADC_overflow_zero_pagex(emu);
  test_ADC_absolute(emu);
  test_ADC_absolutex(emu);
  test_ADC_absolutey(emu);
  test_ADC_indirectx(emu);
  test_ADC_indirecty(emu);
  printf("===========================\n");
  printf(ANSI_COLOR_GREEN "PASSED ALL ADC TESTS!\n" ANSI_COLOR_RESET);
  printf("===========================\n");
  test_AND_immediate(emu);
  test_AND_zero_page(emu);
  test_AND_zero_pagex(emu);
  test_AND_absolute(emu);
  test_AND_absolutex(emu);
  test_AND_absolutey(emu);
  test_AND_indirectx(emu);
  test_AND_indirecty(emu);
  printf("===========================\n");
  printf(ANSI_COLOR_GREEN "PASSED ALL AND TESTS!\n" ANSI_COLOR_RESET);
  printf("===========================\n");
  test_ASL_accumulator(emu);
  test_ASL_zero_page(emu);
  test_ASL_zero_pagex(emu);
  test_ASL_absolute(emu);
  test_ASL_absolutex(emu);
  printf("===========================\n");
  printf(ANSI_COLOR_GREEN "PASSED ALL ASL TESTS!\n" ANSI_COLOR_RESET);
  printf("===========================\n");

  return 1;
}
