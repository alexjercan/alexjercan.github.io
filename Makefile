CC = clang
CFLAGS = -Wall -Wextra -g -I./build
BUILD_DIR = build
SRC_DIR = how_to
LIB_HEADER = aids.h argparse.h

all: $(BUILD_DIR)/main

render:
	./render.sh

$(BUILD_DIR)/main.o: main.c $(LIB_HEADER) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(BUILD_DIR)/main: $(BUILD_DIR)/main.o
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
