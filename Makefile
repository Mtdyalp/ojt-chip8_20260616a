CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include -std=c99

test_cpu: test/test_cpu.c src/chip8.c src/opcodes.c
	$(CC) $(CFLAGS) -o $@ $^

main: src/main.c src/chip8.c src/opcodes.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f test_cpu main

.PHONY: test_cpu main clean