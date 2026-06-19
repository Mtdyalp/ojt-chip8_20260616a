CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include -std=c99
DEBUG_FLAGS = -Wall -Wextra -g -I./include -std=c99 -DDEBUG
LDFLAGS = -lm

TARGET = chip8
TEST_TARGET = test_cpu

SRCS = src/main.c src/chip8.c src/opcodes.c
OBJS = $(SRCS:.c=.o)

# ===== 一次生成两个 =====
all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS = $(DEBUG_FLAGS)
debug: clean $(TARGET)

test: test/test_cpu.c src/chip8.c src/opcodes.c
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $^ $(LDFLAGS)
	@echo "编译完成，手动运行: ./$(TEST_TARGET)"

clean:
	rm -f $(TARGET) $(TEST_TARGET) *.o src/*.o

.PHONY: all debug test clean