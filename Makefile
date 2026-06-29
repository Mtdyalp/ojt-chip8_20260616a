CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include -std=c99
DEBUG_FLAGS = -Wall -Wextra -g -I./include -std=c99 -DDEBUG
LDFLAGS = -lm

# ===== SDL2 链接参数（主程序用） =====
SDL_LDFLAGS = -lm -lSDL2 -lSDL2_ttf

TARGET = chip8
TEST_TARGET = test_cpu

# ===== 主程序源文件（新增 display.c） =====
SRCS = src/main.c src/chip8.c src/opcodes.c src/display.c src/input.c src/audio.c
OBJS = $(SRCS:.c=.o)

TEST_SRCS = test/test_cpu.c src/chip8.c src/opcodes.c

# ===== make 只编译主程序 =====
all: $(TARGET)

# ===== chip8 链接 SDL2 =====
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(SDL_LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ===== make test 编译测试程序（不依赖 SDL2） =====
$(TEST_TARGET): test/test_cpu.c src/chip8.c src/opcodes.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(TEST_TARGET)

# ===== make debug 编译调试版 =====
debug: CFLAGS = $(DEBUG_FLAGS)
debug: clean $(TARGET)

# ===== make clean 清理 =====
clean:
	rm -f $(TARGET) $(TEST_TARGET) *.o src/*.o test/*.o

.PHONY: all debug test clean