#include "test.h"
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
  printf("test_LDA_zeroPage PASSED!\n");
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





int run_unit_tests() {
  emulator_t emu;
  emu.cpu.PC = 0x8000;
  emu.cpu.AC = 0;
  emu.cpu.X = 0;
  emu.cpu.Y = 0;
  emu.cpu.SP = 0;
  emu.cpu.SR.Negative = false;
  emu.cpu.SR.Overflow = false;
  emu.cpu.SR.Break = false;
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
  printf("PASSED ALL LDA TESTS!\n");
  return 1;
}
