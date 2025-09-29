# Makefile for Unix Shell Job Scheduler
# Usage:
#   make        - Build all programs
#   make clean  - Remove compiled programs
#   make test   - Run the shell

CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
TARGETS = shell test_program

.PHONY: all clean test help

all: $(TARGETS)
	@echo "Build complete!"
	@echo "Run './shell' to start the shell"
	@echo "Run 'make test' to start the shell automatically"

shell: shell.c
	$(CC) $(CFLAGS) -o shell shell.c
	@echo "Compiled shell"

test_program: test_program.c
	$(CC) $(CFLAGS) -o test_program test_program.c
	@echo "Compiled test_program"

clean:
	rm -f $(TARGETS)
	@echo "Cleaned up compiled files"

test: shell test_program
	@echo "Starting shell..."
	./shell

help:
	@echo "Unix Shell Job Scheduler - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make          - Build all programs"
	@echo "  make clean    - Remove compiled programs"
	@echo "  make test     - Build and run the shell"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Quick start:"
	@echo "  1. make"
	@echo "  2. ./shell"
	@echo "  3. Try: ./test_program &"
