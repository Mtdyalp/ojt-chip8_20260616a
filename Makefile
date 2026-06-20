CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include -std=c99
DEBUG_FLAGS = -Wall -Wextra -g -I./include -std=c99 -DDEBUG
LDFLAGS = -lm

TARGET = chip8
TEST_TARGET = test_cpu

SRCS = src/main.c src/chip8.c src/opcodes.c
OBJS = $(SRCS:.c=.o)

# ===== make 只编译主程序 =====
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ===== make test 编译测试程序（不运行） =====
$(TEST_TARGET): test/test_cpu.c src/chip8.c src/opcodes.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(TEST_TARGET)

debug: CFLAGS = $(DEBUG_FLAGS)
debug: clean $(TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) *.o src/*.o

.PHONY: all debug test clean