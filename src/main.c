#include "config.h"

#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

#include "chip8.h"

int main() {
  chip8_init();

  SetTraceLogLevel(LOG_WARNING);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "chip_emulator");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    chip8_cycle();
    EndDrawing();
  }
  return 0;
}
