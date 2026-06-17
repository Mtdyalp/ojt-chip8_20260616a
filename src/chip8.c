#include "../include/chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------
   字库：16 个字符，每个 5 字节
   从 0x000 开始放
   ------------------------------------------------------------ */

static const uint8_t font_data[CHIP8_FONT_BYTES] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,     /* 0 */
    0x20, 0x60, 0x20, 0x20, 0x70,     /* 1 */
    0xF0, 0x10, 0xF0, 0x80, 0xF0,     /* 2 */
    0xF0, 0x10, 0xF0, 0x10, 0xF0,     /* 3 */
    0x90, 0x90, 0xF0, 0x10, 0x10,     /* 4 */
    0xF0, 0x80, 0xF0, 0x10, 0xF0,     /* 5 */
    0xF0, 0x80, 0xF0, 0x90, 0xF0,     /* 6 */
    0xF0, 0x10, 0x20, 0x40, 0x40,     /* 7 */
    0xF0, 0x90, 0xF0, 0x90, 0xF0,     /* 8 */
    0xF0, 0x90, 0xF0, 0x10, 0xF0,     /* 9 */
    0xF0, 0x90, 0xF0, 0x90, 0x90,     /* A */
    0xE0, 0x90, 0xE0, 0x90, 0xE0,     /* B */
    0xF0, 0x80, 0x80, 0x80, 0xF0,     /* C */
    0xE0, 0x90, 0x90, 0x90, 0xE0,     /* D */
    0xF0, 0x80, 0xF0, 0x80, 0xF0,     /* E */
    0xF0, 0x80, 0xF0, 0x80, 0x80      /* F */
};

/* ------------------------------------------------------------
   初始化：清零 + 灌字库
   ------------------------------------------------------------ */

void chip8_init(chip8_t *cpu)
{
    memset(cpu, 0, sizeof(chip8_t));

    /* 字库放到内存开头，CHIP-8 程序从这里读字体 */
    memcpy(cpu->mem, font_data, CHIP8_FONT_BYTES);

    cpu->PC = CHIP8_PROG_START;
    cpu->running = 1;
    cpu->dirty = 1;
}

/* ------------------------------------------------------------
   加载 ROM：读到 0x200 位置
   ------------------------------------------------------------ */

int chip8_load_rom(chip8_t *cpu, const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "chip8: 打不开 %s\n", path);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (sz > CHIP8_MEM_SIZE - CHIP8_PROG_START) {
        fprintf(stderr, "chip8: ROM 太大 (%ld 字节)\n", sz);
        fclose(fp);
        return 0;
    }

    size_t n = fread(cpu->mem + CHIP8_PROG_START, 1, sz, fp);
    fclose(fp);

    if (n != (size_t)sz) {
        fprintf(stderr, "chip8: 读取不完整 (%zu / %ld)\n", n, sz);
        return 0;
    }

    cpu->PC = CHIP8_PROG_START;
    cpu->running = 1;

    printf("chip8: 加载 %s 成功 (%ld 字节)\n", path, sz);
    return 1;
}