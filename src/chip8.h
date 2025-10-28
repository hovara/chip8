#ifndef CHIP8_H
#define CHIP8_H

#include "stack.h"
#include <stdint.h>

#define MEM_SIZE 4096
typedef struct Chip8 {
  unsigned char memory[MEM_SIZE];
  uint16_t PC;
  uint16_t I;
  Stack stack;
  uint8_t delay_timer;
  uint8_t sound_timer;
  uint8_t V[16];
} Chip8;

extern Chip8 chip8;

void chip8_load_fonts();
void chip8_load_rom(const char *file_path);
void chip8_init();
void chip8_update_timers();
int chip8_schedule_cycles();
void chip8_cycle();

#endif
