#include "nessdl.h"
#include "test.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Usage: ./nes [<romName>]\n");
    return 1;
  }
  // init emulator
  emulator_t *emu = malloc(sizeof(emulator_t));
  run_unit_tests();

  // SDL
  nsdl_manager *manager = start_window();
  if (renderLoop(manager)) {
    destroy_window();
  }
  free(emu);
  return 0;
}
