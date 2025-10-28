#include "config.h"

#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "chip8.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Too little arguments!\n");
    printf("Hint: ./chip8 'filepath'\n");
  } else if (argc > 2) {
    printf("Too many arguments!\n");
    printf("Only one 'filepath' is supported.\n");
  } else
    (chip8_load_rom(argv[1]));

  chip8_init();

  SetTraceLogLevel(LOG_WARNING);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "chip_emulator");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    chip8_update_timers();
    BeginDrawing();
    for (int i = chip8_schedule_cycles(); i > 0; i--)
      chip8_cycle();
    EndDrawing();
  }
  return 0;
}
