#ifndef TEST_H
#define TEST_H
#include "emulator.h"
#include "cpu.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
int run_unit_tests();
void testCpuPart(const cpu_t cpu, const uint8_t *memory, const instruction_t instr);

#endif
