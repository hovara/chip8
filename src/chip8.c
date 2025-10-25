// test code
#include <stdint.h>
#include <unistd.h>
// test code

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

static uint8_t display[DISPLAY_HEIGHT][DISPLAY_WIDTH];
static void display_draw(void) {
  for (int y = 0; y < DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
      if (display[y][x])
        DrawRectangle(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE,
                      WHITE);
      else
        DrawRectangle(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE,
                      BLACK);
    }
  }
}

void chip8_init() {
  // test code

  chip8.I = 0x50;
  chip8.memory[0x200] = 0xD0;
  chip8.memory[0x201] = 0x05;

  // test code

  chip8.PC = 0x200;
  chip8_load_fonts();
}

static inline uint16_t _OP(uint8_t B0, uint8_t B1) {
  return ((uint16_t)(B0) << 8) | (uint16_t)B1;
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
    if (X == 0x0) {
      switch (NN) {
      case 0xE0: // Clear screen
        ClearBackground(BLACK);
        break;

      case 0xEE: // Return from a subroutine
        stack_pop(&chip8.stack, &chip8.PC);
        break;
      }
    } // else {} OP = 0x0NNN irrelevant (Execute machine language subroutine)
    break;
  case 0x1: // Jump
    chip8.PC = NNN;
    break;
  case 0x2: // Call subroutine
    stack_push(&chip8.stack, chip8.PC);
    chip8.PC = NNN;
    break;
  case 0x3: // Conditional skip
    if (chip8.V[X] == NN)
      chip8.PC += 2;
    break;
  case 0x4: // Conditional skip
    if (chip8.V[X] != NN)
      chip8.PC += 2;
    break;
  case 0x5: // Conditional skip
    if (chip8.V[X] == chip8.V[Y])
      chip8.PC += 2;
    break;
  case 0x6: // Set
    chip8.V[X] = NN;
    break;
  case 0x7: // Add
    chip8.V[X] += NN;
    break;
  case 0x8:

    switch (N) {
    case 0x0: // Set
      chip8.V[X] = chip8.V[Y];
      break;
    case 0x1: // Binary OR
      chip8.V[X] = chip8.V[X] | chip8.V[Y];
      break;
    case 0x2: // Binary AND
      chip8.V[X] = chip8.V[X] & chip8.V[Y];
      break;
    case 0x3: // Logical XOR
      chip8.V[X] = chip8.V[X] ^ chip8.V[Y];
      break;
    case 0x4: { // Add with overflow
      uint16_t int_buff = chip8.V[X] + chip8.V[Y];
      if (int_buff > 255)
        chip8.V[0xF] = 1;
      else
        chip8.V[0xF] = 0;
      chip8.V[X] = (uint8_t)int_buff;
      break;
    }
    case 0x5: // Subtract with borrow
      if (chip8.V[X] > chip8.V[Y])
        chip8.V[0xF] = 1;
      else
        chip8.V[0xF] = 0;
      chip8.V[X] = chip8.V[X] - chip8.V[Y];
      break;
    case 0x6: // Shift right
      if (SYS_TYPE == SYS_COSMAC_VIP) {
        chip8.V[X] = chip8.V[Y];
      }
      chip8.V[0xF] = 1 & chip8.V[X];
      chip8.V[X] = chip8.V[X] >> 1;
      break;
    case 0x7: // Subtract with borrow
      if (chip8.V[Y] > chip8.V[X])
        chip8.V[0xF] = 1;
      else
        chip8.V[0xF] = 0;
      chip8.V[X] = chip8.V[Y] - chip8.V[X];
      break;
    case 0xE: // Shift left
      if (SYS_TYPE == SYS_COSMAC_VIP) {
        chip8.V[X] = chip8.V[Y];
      }
      chip8.V[0xF] = 0x80 & chip8.V[X];
      chip8.V[X] = chip8.V[X] << 1;
      break;
    }
    break;
  case 0x9: // Conditional skip
    if (chip8.V[X] != chip8.V[Y])
      chip8.PC += 2;
    break;
  case 0xA:
    chip8.I = NNN;
    break;
  case 0xB:
    break;
  case 0xC:
    break;
  case 0xD: // Display
    chip8.V[0xF] = 0;
    // wrong here
    X &= 63;
    Y &= 31;
    for (size_t i = 0; i < N; i++) {
      if (Y + i > 31) {
        break;
      }

      for (int ib = 7; ib >= 0; ib--) {
        if (X + ib > 63) {
          break;
        }

        uint8_t lsb = ((chip8.memory[chip8.I + i] >> ib) & 1);
        if (display[Y + i][X + (7 - ib)] && lsb) {
          chip8.V[0xF] = 1;
        }
        display[Y + i][X + (7 - ib)] ^= lsb;
      }
    }
    display_draw();
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
