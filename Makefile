CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include -std=c99

test_cpu: test/test_cpu.c src/chip8.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f test_cpu

.PHONY: test_cpu clean