.SILENT: all run clean

CC = gcc

INCLUDE_DIR = /usr/local/include
LIB_DIR = /usr/local/lib

CFLAGS = -I$(INCLUDE_DIR)
LDFLAGS = -L$(LIB_DIR) -lraylib -lm -lpthread -ldl -lrt -lGL -lX11

SRC = ./src/*.c
OUT = chip8

ROM ?= IBM.ch8

all:
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LDFLAGS)

run: all
	./$(OUT) ./roms/$(ROM)

clean:
	rm -f $(OUT)

test:
	$(CC) test.c -o test $(CFLAGS) $(LDFLAGS)
