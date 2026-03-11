CC := gcc

SRC_DIR := src
INC_DIR := include
BUILD_DIR := build

CFLAGS := -Wall -Wextra -Wno-unused-parameter -I$(INC_DIR)
DEBUG_FLAGS := -g

SRC := $(shell find $(SRC_DIR) -name "*.c")

OBJ := $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TARGET := shell

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all debug clean
