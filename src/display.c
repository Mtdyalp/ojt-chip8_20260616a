#include "../include/display.h"
#include <stdio.h>
#include <string.h>

static void draw_text(DisplayState *state,
                      const char *text,
                      int x,
                      int y)
{
    SDL_Color color = {255, 255, 255, 255};//设置颜色

    //把字符串转换成surface(是生成出来的原始像素图片)
    SDL_Surface *surface =
        TTF_RenderUTF8_Blended(state->font, text, color);

    if (!surface) {
        printf("文字生成失败: %s\n", TTF_GetError());
        return;
    }

    //把 Surface 转换成 Texture(是把这张图片转换成渲染器能够画到窗口上的格式)
    SDL_Texture *texture =
        SDL_CreateTextureFromSurface(state->renderer, surface);

    if (!texture) {
        printf("文字纹理创建失败: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    //设置文字显示位置和大小
    SDL_Rect dst = {
        x,
        y,
        surface->w,
        surface->h
    };

    SDL_RenderCopy(state->renderer, texture, NULL, &dst);//将文字纹理复制到渲染器

    SDL_DestroyTexture(texture);// 释放 Texture
    SDL_FreeSurface(surface);//释放 Surface
}

static void draw_debug_panel(
    DisplayState *state,
    chip8_t *cpu,
    const char *debug_status,
    const uint16_t *breakpoints,
    int breakpoint_count)
{
    SDL_Rect panel = {
        GAME_W,
        0,
        DEBUG_PANEL_W,
        DEBUG_WINDOW_H
    };

    SDL_SetRenderDrawColor(state->renderer, 30, 30, 30, 255);//深灰色
    SDL_RenderFillRect(state->renderer, &panel);

    SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(state->renderer, GAME_W, 0, GAME_W, DEBUG_WINDOW_H);//分割线 从坐标(GAME_W,0)到(GAME_W,DEBUG_WINDOW_H)分割

    int x = GAME_W + 12;
    int y = 8;
    char line[128];

    /* 调试状态 */
    if (debug_status &&
        strcmp(debug_status, "PAUSED") == 0) {
        snprintf(line, sizeof(line),
                 "[PAUSED] Step: N Resume: Space");
    } else {
        snprintf(line, sizeof(line),
                 "[%s]",
                 debug_status ? debug_status : "DEBUG");
    }

    draw_text(state, line, x, y);
    y += 20;

    /* 寄存器 */
    draw_text(state, "== Registers ==", x, y);
    y += 18;

    for (int row = 0; row < 4; row++) {// 每行显示 4 个寄存器
        int first = row * 4;// 每行的第一个寄存器索引

        snprintf(line, sizeof(line),
                 "V%X:%02X V%X:%02X V%X:%02X V%X:%02X",
                 first,     cpu->V[first],
                 first + 1, cpu->V[first + 1],
                 first + 2, cpu->V[first + 2],
                 first + 3, cpu->V[first + 3]);

        draw_text(state, line, x, y);
        y += 17;
    }

    snprintf(line, sizeof(line),
             "I:%04X PC:%04X SP:%02X",
             cpu->I, cpu->PC, cpu->SP);

    draw_text(state, line, x, y);
    y += 18;

    snprintf(line, sizeof(line),
             "DT:%02X ST:%02X",
             cpu->delay_timer,
             cpu->sound_timer);

    draw_text(state, line, x, y);
    y += 22;

    /* 反汇编 */
    draw_text(state, "== Disassembly ==", x, y);
    y += 18;

    for (int offset = -2; offset <= 2; offset++) {// 显示当前指令前后各两条指令
        int address = (int)cpu->PC + offset * 2;// 每条指令占 2 个字节

        // 检查地址是否在有效范围内，防止越界       
        if (address < 0 ||
            address >= CHIP8_MEM_SIZE - 1) {
            continue;
        }

        /* 从内存读取两个字节，组成一条指令 */
        uint16_t op =
            ((uint16_t)cpu->mem[address] << 8) |
            cpu->mem[address + 1];

        char instruction[64];

        /* opcode 转换成助记符 */
        chip8_disassemble(op,
                          instruction,
                          sizeof(instruction));

        /* 组合地址、opcode 和助记符 */
        snprintf(line, sizeof(line),
                 "%c 0x%04X: %04X  %s",
                 address == cpu->PC ? '>' : ' ',
                 address,
                 op,
                 instruction);

        /* 显示到调试面板 */
        draw_text(state, line, x, y);
        y += 16;
    }

    //栈
    draw_text(state, "== Stack ==", x, y);
    y += 16;

    if (cpu->SP == 0) {
    draw_text(state, "(empty)", x, y);
    y += 16;
    }

    for (int i = (int)cpu->SP - 1; i >= 0; i--) {
    snprintf(line, sizeof(line),
             "[%d] 0x%04X%s",
             i,
             cpu->stack[i],
             i == cpu->SP - 1 ? " <- top" : "");

    draw_text(state, line, x, y);
    y += 16;
    }

   //断点管理
   draw_text(state, "== Breakpoints ==", x, y);
   y += 16;

   for (int i = 0; i < breakpoint_count; i++) {
   snprintf(line, sizeof(line),
            "* 0x%04X", breakpoints[i]);

   draw_text(state, line, x, y);
   y += 16;
   }
  
#if 0   
    //memory_address
   if (memory_watch_enabled) {
       snprintf(line, sizeof(line),
             "== Memory 0x%04X ==",
             memory_address);

        draw_text(state, line, x, y);
        y += 16;

        for (int i = 0; i < 8; i++) {
        int address = memory_address + i;

        if (address >= CHIP8_MEM_SIZE) {
            break;
        }

        snprintf(line, sizeof(line),"0x%04X: %02X",address,cpu->mem[address]);

        draw_text(state, line, x, y);
        y += 16;
       }
   }
#endif

#if 0
    snprintf(line, sizeof(line),"Speed: %d cycles/frame",cycles_per_frame);
    draw_text(state, line, x, y);
#endif

}

int display_init(DisplayState *state, int debug_mode)
{
    state->window = NULL;
    state->renderer = NULL;
    state->font = NULL;
    state->debug_mode = debug_mode;
     
    //初始化 SDL 视频系统
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init 失败: %s\n", SDL_GetError());
        return -1;
    }

    int window_width = debug_mode
                     ? DEBUG_WINDOW_W
                     : NORMAL_WINDOW_W;

    int window_height = debug_mode
                  ? DEBUG_WINDOW_H
                  : NORMAL_WINDOW_H;
    //创建窗口
    state->window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width, /* 窗口宽度 */
        window_height,/* 窗口高度 */
        SDL_WINDOW_SHOWN);//窗口日志

    if (!state->window) {
        printf("SDL_CreateWindow 失败: %s\n", SDL_GetError());
        display_cleanup(state);
        return -1;
    }

    //创建渲染器
    state->renderer = SDL_CreateRenderer(
        state->window,
        -1,
        SDL_RENDERER_SOFTWARE);

    if (!state->renderer) {
        printf("SDL_CreateRenderer 失败: %s\n", SDL_GetError());
        display_cleanup(state);
        return -1;
    }

    //判断是否需要字体
    if (debug_mode) {
        //初始化 SDL2_ttf
        if (TTF_Init() < 0) {
            printf("TTF_Init 失败: %s\n", TTF_GetError());
            display_cleanup(state);
            return -1;
        }
       //加载字体
        state->font = TTF_OpenFont("assets/font.ttf", 16);

        if (!state->font) {
            printf("字体加载失败: %s\n", TTF_GetError());
            display_cleanup(state);
            return -1;
        }
    }

    return 0;
}

void display_render(
    DisplayState *state,
    chip8_t *cpu,
    const char *debug_status,
    const uint16_t *breakpoints,
    int breakpoint_count)
{
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);//黑色
    SDL_RenderClear(state->renderer);//清屏

    for (int y = 0; y < CHIP8_SCREEN_H; y++) {// 遍历 CHIP-8 屏幕的每一行
        for (int x = 0; x < CHIP8_SCREEN_W; x++) {// 遍历 CHIP-8 屏幕的每一列
            if (cpu->screen[y * CHIP8_SCREEN_W + x]) {// 如果该像素是亮的
                SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);//白色
                SDL_Rect rect = {x * SCALE, y * SCALE, SCALE, SCALE};//每个 CHIP-8 像素放大 SCALE 倍
                SDL_RenderFillRect(state->renderer, &rect);//画白色方块
            }
        }
    }

    if (state->debug_mode && state->font) {
#if 1        
        draw_debug_panel(state, cpu, debug_status, breakpoints, breakpoint_count);
#else
        draw_debug_panel(state, cpu, debug_status, breakpoints, breakpoint_count，
                         memory_watch_enabled, (uint16_t)memory_address);
#endif                
    }

    SDL_RenderPresent(state->renderer);
}

void display_cleanup(DisplayState *state)
{
    if (state->font) {
        TTF_CloseFont(state->font);
        state->font = NULL;
    }

    if (TTF_WasInit()) {
        TTF_Quit();
    }

    if (state->renderer) {
        SDL_DestroyRenderer(state->renderer);
        state->renderer = NULL;
    }

    if (state->window) {
        SDL_DestroyWindow(state->window);
        state->window = NULL;
    }

    SDL_Quit();
}