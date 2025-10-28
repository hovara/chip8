#include "config.h"

#include "chip8.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static bool is_chip8_key_down(uint8_t key) {
  switch (key & 0xF) {
  case 0x1:
    return IsKeyDown('1');
  case 0x2:
    return IsKeyDown('2');
  case 0x3:
    return IsKeyDown('3');
  case 0xC:
    return IsKeyDown('4');
  case 0x4:
    return IsKeyDown('Q');
  case 0x5:
    return IsKeyDown('W');
  case 0x6:
    return IsKeyDown('E');
  case 0xD:
    return IsKeyDown('R');
  case 0x7:
    return IsKeyDown('A');
  case 0x8:
    return IsKeyDown('S');
  case 0x9:
    return IsKeyDown('D');
  case 0xE:
    return IsKeyDown('F');
  case 0xA:
    return IsKeyDown('Z');
  case 0x0:
    return IsKeyDown('X');
  case 0xB:
    return IsKeyDown('C');
  case 0xF:
    return IsKeyDown('V');
  default:
    return false;
  }
}

static uint8_t get_key_pressed(void) {
  // COSMAC VIP keypad
  // 1 2 3 C
  // 4 5 6 D
  // 7 8 9 E
  // A 0 B F
  if (IsKeyDown('1'))
    return 0x1;
  if (IsKeyDown('2'))
    return 0x2;
  if (IsKeyDown('3'))
    return 0x3;
  if (IsKeyDown('4'))
    return 0xC;
  if (IsKeyDown('Q'))
    return 0x4;
  if (IsKeyDown('W'))
    return 0x5;
  if (IsKeyDown('E'))
    return 0x6;
  if (IsKeyDown('R'))
    return 0xD;
  if (IsKeyDown('A'))
    return 0x7;
  if (IsKeyDown('S'))
    return 0x8;
  if (IsKeyDown('D'))
    return 0x9;
  if (IsKeyDown('F'))
    return 0xE;
  if (IsKeyDown('Z'))
    return 0xA;
  if (IsKeyDown('X'))
    return 0x0;
  if (IsKeyDown('C'))
    return 0xB;
  if (IsKeyDown('V'))
    return 0xF;

  return 0xFF; // none pressed
}

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
static void clear_display() {
  for (int y = 0; y < DISPLAY_HEIGHT; y++)
    for (int x = 0; x < DISPLAY_WIDTH; x++)
      display[y][x] = 0;
}
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

void chip8_load_rom(const char *file_path) {
  FILE *f = fopen(file_path, "rb");
  if (!f) {
    printf("fopen() error: Failed to open file\n");
    return;
  }
  if (fseek(f, 0, SEEK_END) != 0) {
    printf("fseek() error: Failed to move position indicator\n");
    fclose(f);
    return;
  }
  long fsize = ftell(f);
  if (fsize < 0) {
    printf("ftell() error: Failed to read value of position indicator\n");
    fclose(f);
    return;
  }
  rewind(f);
  if (fsize > (MEM_SIZE - 0x200)) {
    printf("ROM too large! MEM_SIZE= %d [bytes], ROM_SIZE=%ld [bytes]\n",
           MEM_SIZE, fsize);
    fclose(f);
    return;
  }
  size_t read = fread(&chip8.memory[0x200], 1, fsize, f);
  fclose(f);
  if (read != fsize) {
    printf("fread() error: Read %zu instead of %ld from %s\n", read, fsize,
           file_path);
    return;
  }
  printf("ROM loaded successfully.\n");
}

void chip8_init() {

  srand(time(NULL));
  chip8.PC = 0x200;
  chip8_load_fonts();
}

void chip8_update_timers() {
  const double TIMERS_UPDATE_TIME = 1.0 / 60.0;
  const double EPS = 1e-6;
  static double elapsed_time = 0;
  elapsed_time += GetFrameTime();

  while (elapsed_time + EPS >= TIMERS_UPDATE_TIME) {
    chip8.delay_timer -= chip8.delay_timer > 0;
    chip8.sound_timer -= chip8.sound_timer > 0;
    elapsed_time -= TIMERS_UPDATE_TIME;
  }
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

int chip8_schedule_cycles() {
  const double CYCLE_TIME = 1.0 / CHIP8_NO_CYCLES;
  const double EPS = 1e-6;
  static double elapsed_time = 0;

  elapsed_time += GetFrameTime();

  int n = (int)((elapsed_time + EPS) / CYCLE_TIME);
  elapsed_time -= n * CYCLE_TIME;
  return n;
}

void chip8_cycle() {
  ///////////
  // fetch //
  ///////////
  uint16_t OP = _OP(chip8.memory[chip8.PC], chip8.memory[chip8.PC + 1]);
  chip8.PC += 2;

  ////////////
  // decode //
  ////////////
  uint8_t instr = _instr(OP);
  uint8_t X = _X(OP);
  uint8_t Y = _Y(OP);
  uint8_t N = _N(OP);
  uint8_t NN = _NN(OP);
  uint16_t NNN = _NNN(OP);

  /////////////
  // execute //
  /////////////
  switch (instr) {
  case 0x0:
    if (X == 0x0) {
      switch (NN) {
      case 0xE0: // Clear screen
        ClearBackground(BLACK);
        clear_display();
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
    case 0x4: /* Add with overflow */ {
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
  case 0xA: // Set index
    chip8.I = NNN;
    break;
  case 0xB: // Jump with offset
    switch (SYS_TYPE) {
    default:
    case SYS_COSMAC_VIP:
      chip8.PC = NNN + chip8.V[0x0];
      break;
    case SYS_S_CHIP:
      chip8.PC = NNN + chip8.V[X];
      break;
    }
    break;
  case 0xC: // Random
    chip8.V[X] = rand() & NN;
    break;
  case 0xD: // Display
    chip8.V[0xF] = 0;
    uint8_t pos_x = chip8.V[X] &= 63;
    uint8_t pos_y = chip8.V[Y] &= 31;
    for (size_t i = 0; i < N; i++) {
      if (pos_y + i > 31) {
        break;
      }
      for (int ib = 7; ib >= 0; ib--) {
        if (pos_x + ib > 63) {
          break;
        }
        uint8_t lsb = ((chip8.memory[chip8.I + i] >> ib) & 1);
        if (display[pos_y + i][pos_x + (7 - ib)] && lsb) {
          chip8.V[0xF] = 1;
        }
        display[pos_y + i][pos_x + (7 - ib)] ^= lsb;
      }
    }
    display_draw();
    break;
  case 0xE:
    switch (NN) {
    case 0x9E: // Skip if key pressed
      if (is_chip8_key_down(chip8.V[X])) {
        chip8.PC += 2;
      }
      break;
    case 0xA1: // Skip if key not pressed
      if (!is_chip8_key_down(chip8.V[X])) {
        chip8.PC += 2;
      }
      break;
    }
    break;
  case 0xF:
    switch (NN) {
    case 0x07: // Set Vx to value of delay timer
      chip8.V[X] = chip8.delay_timer;
      break;
    case 0x15: // Set delay timer to Vx
      chip8.delay_timer = chip8.V[X];
      break;
    case 0x18: // Set sound timer to Vx
      chip8.sound_timer = chip8.V[X];
      break;
    case 0x1E: // Add to index
      chip8.I += chip8.V[X];
      if (SYS_TYPE == SYS_AMIGA) {
        chip8.V[0xF] = (chip8.I >> 12) & 1;
      }
      chip8.I &= 0x0FFF;
      break;
    case 0x0A: /* Get key */ {
      uint8_t key_pressed = get_key_pressed();
      if (key_pressed == 0xFF) {
        chip8.PC -= 2;
      } else {
        if (SYS_TYPE == SYS_COSMAC_VIP)
          while (!IsKeyReleased(key_pressed)) {
          }
        chip8.V[X] = key_pressed;
      }
      break;
    }
    case 0x29: // Set index register to Font character address
      chip8.I = 0x50 + (chip8.V[X] & 0x0F) * 5;
      break;
    case 0x33: // Binary-coded decimal conversion
      if (chip8.V[X] == 0)
        chip8.memory[chip8.I] = 0;
      else {
        uint8_t i = 0;
        while (chip8.V[X] > 0) {
          chip8.memory[chip8.I + i] = chip8.V[X] % 10;
          chip8.V[X] = chip8.V[X] / 10;
          i++;
        }
      }
      break;
    case 0x55: // Store memory
      for (int i = 0; i <= X; i++) {
        chip8.memory[chip8.I + i] = chip8.V[i];
      }
      if (SYS_TYPE == SYS_COSMAC_VIP)
        chip8.I += X + 1;
      break;
    case 0x65: // Load memory
      for (int i = 0; i <= X; i++) {
        chip8.V[i] = chip8.memory[chip8.I + i];
      }
      if (SYS_TYPE == SYS_COSMAC_VIP)
        chip8.I += X + 1;
      break;
    }
    break;
  default:
    break;
  }
}
