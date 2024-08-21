#pragma once
#ifndef TEST_H
#define TEST_H
#include "cpu.h"
#include "ppu.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
int run_unit_tests();
void testCpuPart(const struct cpu_6502 cpu, const struct instruction instr);
void test_ppu(const struct ppu_2C02 ppu);
#endif
