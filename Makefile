CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Iinclude
LDLIBS := -luser32 -lgdi32
BUILD_DIR := build
BIN := $(BUILD_DIR)/gbemu

SRCS := $(wildcard src/*.c) $(wildcard src/cpu/*.c)
OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all check clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compiles every source file to an object without linking, for use before
# main.c / the frontend exist yet.
check: $(OBJS)

clean:
	rm -rf $(BUILD_DIR)
