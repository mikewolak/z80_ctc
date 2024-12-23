# Makefile for Z80 CTC Library

# Compiler
CC = gcc

# Base compiler flags
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -Iinclude

# Debug flags
DEBUG_FLAGS = -DCTC_DEBUG

# Directories
SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib
EXAMPLES_DIR = examples

# Library names
LIB_NAME = libz80ctc.a
DEBUG_LIB_NAME = libz80ctc_debug.a

# Source files
SRCS = $(SRC_DIR)/z80_ctc.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEBUG_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/debug_%.o,$(SRCS))

# Example sources
EXAMPLE_SRCS = $(wildcard $(EXAMPLES_DIR)/*.c)
EXAMPLE_BINS = $(patsubst $(EXAMPLES_DIR)/%.c,$(EXAMPLES_DIR)/%,$(EXAMPLE_SRCS))
DEBUG_EXAMPLE_BINS = $(patsubst $(EXAMPLES_DIR)/%.c,$(EXAMPLES_DIR)/%_debug,$(EXAMPLE_SRCS))

# Default target
all: $(LIB_DIR)/$(LIB_NAME) examples

# Debug target
debug: $(LIB_DIR)/$(DEBUG_LIB_NAME) debug_examples

# Create library
$(LIB_DIR)/$(LIB_NAME): $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(AR) rcs $@ $^

# Create debug library
$(LIB_DIR)/$(DEBUG_LIB_NAME): $(DEBUG_OBJS)
	@mkdir -p $(LIB_DIR)
	$(AR) rcs $@ $^

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile debug source files
$(OBJ_DIR)/debug_%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

# Build examples
examples: $(EXAMPLE_BINS)

# Build debug examples
debug_examples: $(DEBUG_EXAMPLE_BINS)

# Rule to build each example
$(EXAMPLES_DIR)/%: $(EXAMPLES_DIR)/%.c $(LIB_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -lz80ctc

# Rule to build each debug example
$(EXAMPLES_DIR)/%_debug: $(EXAMPLES_DIR)/%.c $(LIB_DIR)/$(DEBUG_LIB_NAME)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $< -o $@ -L$(LIB_DIR) -lz80ctc_debug

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(EXAMPLE_BINS) $(DEBUG_EXAMPLE_BINS)

# Phony targets
.PHONY: all clean examples debug debug_examples
