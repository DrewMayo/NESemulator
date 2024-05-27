#include "nessdl.h"
#include "emulator.h"
#include <stdint.h>

//memory location 0x8000 is the start of the 
//rom cartrige

int readRom(emulator *emu, char *filename) {
  FILE *fp = fopen(filename, "rb");
  int counter = 0;
  while(!feof(fp)) {
    fread(&emu->cpu.ram[0x8000 + counter], sizeof(uint16_t), 1, fp);
    counter++;
    if (counter + 0x8000 > CPURAMSIZE) {
      printf("Error: ROM size is too big");
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Usage: ./nes [<romName>]\n");
    return 1;
  }
  //init emulator
  emulator *emu = malloc(sizeof(emulator));

  //read rom
  readRom(emu, argv[1]);
  //for(int i = 0; i < 65536; i++) {
  //  printf("address: %x, value: %x\n", i, emu->cpu.ram[i]);}
  
  //SDL
  nsdl_manager *manager = start_window();
  if (renderLoop(manager)) {
    destroy_window();
  }
  free(emu);
  return 0;
}


