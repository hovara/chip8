#include "config.h"

#include "chip8.h"
#include <raylib.h>
#include <string.h>

Chip8 chip8;

static const unsigned char fonts[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
void chip8_load_fonts() { memcpy(&chip8.memory[0x50], fonts, sizeof(fonts)); }

// this is where i am
// how to read from memory[I] each bit into display as 1 or 0
// how
static uint8_t display[DISPLAY_HEIGHT][DISPLAY_WIDTH];

void chip8_init() {
  chip8.PC = 0x200;
  chip8_load_fonts();
}

static inline uint16_t _OP(uint8_t B0, uint8_t B1) {
  return ((uint16_t)(B0 & 0x0F) << 8) | (uint16_t)B1;
}
static inline uint8_t _instr(uint16_t OP) { return (uint8_t)(OP >> 12); }
static inline uint8_t _X(uint16_t OP) { return (uint8_t)(OP >> 8) & 0x0F; }
static inline uint8_t _Y(uint16_t OP) { return (uint8_t)(OP >> 4) & 0x0F; }
static inline uint8_t _N(uint16_t OP) { return (uint8_t)OP & 0x0F; }
static inline uint8_t _NN(uint16_t OP) { return (uint8_t)OP; }
static inline uint16_t _NNN(uint16_t OP) { return OP & 0x0FFF; }

void chip8_cycle() {
  // fetch
  uint16_t OP = _OP(chip8.memory[chip8.PC], chip8.memory[chip8.PC + 1]);
  chip8.PC += 2;

  // decode
  uint8_t instr = _instr(OP);
  uint8_t X = _X(OP);
  uint8_t Y = _Y(OP);
  uint8_t N = _N(OP);
  uint8_t NN = _NN(OP);
  uint16_t NNN = _NNN(OP);

  // execute
  switch (instr) {
  case 0x0:
    if (N ^ 0xE) {
      ClearBackground(BLACK);
    }
    break;
  case 0x1:
    chip8.PC = NNN;
    break;
  case 0x2:
    break;
  case 0x3:
    break;
  case 0x4:
    break;
  case 0x5:
    break;
  case 0x6:
    chip8.V[X] = NN;
    break;
  case 0x7:
    chip8.V[X] += NN;
    break;
  case 0x8:
    break;
  case 0x9:
    break;
  case 0xA:
    chip8.I = NNN;
    break;
  case 0xB:
    break;
  case 0xC:
    break;
  case 0xD:
    for (size_t i = 0; i < N; i++) {
    }
    break;
  case 0xE:
    break;
  case 0xF:
    break;
  default:
    break;
  }

  // execute
  // wait:
}
