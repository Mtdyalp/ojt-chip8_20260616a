#include "../include/chip8.h"
#include "../include/opcodes.h"
#include "../include/display.h"
#include "../include/input.h"
#include "../include/audio.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef enum {
    DEBUG_RUNNING,//0
    DEBUG_PAUSED,
    DEBUG_STEPPING
} debug_state_t;
#define MAX_BREAKPOINTS 32

/*
 * 函数名称    : Chip8_StrToU32
 * 功能        : 将字符串转换成 uint32_t 类型数据
 * 输入参数    : szSrcStr  字符串地址，例如 "512"、"0x200"、"0xFFF"
 * 输出参数    : ulDstNum  转换后的 uint32_t 数据
 * 返回值      : 0 成功
 *               1 失败
 * 说明        : base 使用 0，支持十进制、十六进制、八进制自动识别
 */
int Chip8_StrToU32(const char *szSrcStr,
                          uint32_t *ulDstNum)
{
    char *szEndPtr = NULL;
    unsigned long ulTmp;

    if (szSrcStr == NULL || ulDstNum == NULL) {
        return 1;
    }

    if (szSrcStr[0] == '\0' ||
        szSrcStr[0] == '-') {
        *ulDstNum = 0;
        return 1;
    }

    /* 调用 strtoul() 前必须清除 errno */
    errno = 0;

    ulTmp = strtoul(szSrcStr, &szEndPtr, 0);

    if (errno == ERANGE ||
        szEndPtr == szSrcStr ||
        *szEndPtr != '\0' ||
        ulTmp > UINT32_MAX) {

        *ulDstNum = 0;
        return 1;
    }

    *ulDstNum = (uint32_t)ulTmp;
    return 0;
}


static void print_usage(const char *prog)
{
    printf("Usage:\n");
    printf("  %s                 run display/timer test mode\n", prog);
    printf("  %s <rom>           run CHIP-8 ROM\n", prog);
    printf("  %s -d <rom>        run ROM with debug output\n", prog);
    printf("  %s --debug <rom>   run ROM with debug output\n", prog);
}

static void debug_print_state(const chip8_t *cpu)
{
    uint16_t op = (cpu->mem[cpu->PC] << 8) | cpu->mem[cpu->PC + 1];
#if 1
    printf("[DBG] PC=0x%04X OP=0x%04X I=0x%04X DT=%u ST=%u VF=%u\n",
           cpu->PC,
           op,
           cpu->I,
           cpu->delay_timer,
           cpu->sound_timer,
           cpu->V[0xF]);
#endif           
}
static const char *debug_state_name(debug_state_t state)
{
    switch (state) {
    case DEBUG_RUNNING:
        return "RUNNING";

    case DEBUG_PAUSED:
        return "PAUSED";

    case DEBUG_STEPPING:
        return "STEPPING";

    default:
        return "UNKNOWN";
    }
}
#if 1
static int has_breakpoint(uint16_t pc, uint16_t *breakpoints, int count)
{
    for (int i = 0; i < count; i++) {
        if (breakpoints[i] == pc) {
            return 1;/* 有断点，暂停，返回1 */
        }
    }

    return 0;/* 无断点，继续执行，返回0 */
}
#endif    
/* ============================================================
   显示测试：画一个点
   ============================================================ */
static void run_test_pattern(DisplayState *display, chip8_t *cpu)
{
    cpu->screen[15 * CHIP8_SCREEN_W + 31] = 1;
    cpu->dirty = 1;
    display_render(display, cpu, NULL, NULL, 0);

    cpu->delay_timer = 60;
    cpu->sound_timer = 60;

    printf("\n===== 显示/定时器测试 =====\n");
    printf("屏幕中间有一个白色方块\n");
    printf("DT/ST 从 60 开始递减，约 1 秒到 0\n");
    printf("sound_timer > 0 时蜂鸣，ESC 退出\n");

    int running = 1;
    SDL_Event event;
    int last_dt = -1;
    int last_st = -1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                } else {
                    int key = input_sdl_to_chip8(event.key.keysym.sym);
                    if (key >= 0) {
                        printf("[KEY] %s -> CHIP-8 0x%X\n",
                               SDL_GetKeyName(event.key.keysym.sym), key);
                        cpu->key_down[key] = 1;
                    }
                }
            } else if (event.type == SDL_KEYUP) {
                int key = input_sdl_to_chip8(event.key.keysym.sym);
                if (key >= 0) {
                    cpu->key_down[key] = 0;
                }
            }
        }

        if (cpu->delay_timer > 0) cpu->delay_timer--;
        if (cpu->sound_timer > 0) cpu->sound_timer--;

        audio_update(cpu->sound_timer);

        if (cpu->delay_timer != last_dt || cpu->sound_timer != last_st) {
#if 0            
            printf("[TIMER] DT=%d ST=%d\n",
                   cpu->delay_timer, cpu->sound_timer);
#endif                   
            last_dt = cpu->delay_timer;
            last_st = cpu->sound_timer;
        }

        SDL_Delay(16);
    }
}

int main(int argc, char *argv[])
{
    int debug_mode = 0;
    const char *rom_path = NULL;
    int iParameter;
    debug_state_t debug_state = DEBUG_RUNNING;    
    uint16_t breakpoints[MAX_BREAKPOINTS];
    int breakpoint_count = 0;
    int skip_breakpoint_once = 0;
    const char *status = NULL;
    uint32_t addr = 0;
  //int cycles_per_frame = 1;

    char *szShortOptions = "dhb:";
//    char *szShortOptions = "dhb:m:";
    struct option stLongOptions[] = {
        {"debug" , 0,  NULL,   'd'},
        {"break" , 1,  NULL,   'b'},
      //{"memory", 1,  NULL,   'm'},
        {"help"  , 0,  NULL,   'h'},
        {0       , 0,  0   ,    0 }
    };

    while ((iParameter = getopt_long(argc, argv, szShortOptions, stLongOptions, NULL)) != -1) {
        switch (iParameter) {
        case 'd':
            debug_mode = 1;
            break;
        case 'b': {          
            if (Chip8_StrToU32(optarg, &addr) != 0) {
                printf("invalid addr: %s\n", optarg);
                return 1;
            }

            /* 一条指令需要读取 address 和 address + 1 */
            if (addr > 0x0FFE) {
                printf("Breakpoint out of range: %s\n", optarg);
                return 1;
           }

            if ((addr & 1) != 0) {
                 printf("Breakpoint must be even: %s\n", optarg);
                return 1;
            }

            if (breakpoint_count >= MAX_BREAKPOINTS) {
                printf("Too many breakpoints\n");
                return 1;
           }

           breakpoints[breakpoint_count++] = (uint16_t)addr;      
           debug_mode = 1;
           break;
        }    
#if 0
        case 'm':
            if (Chip8_StrToU32(optarg,&memory_address) != 0 ||
                memory_address >= CHIP8_MEM_SIZE) {
                printf("Invalid memory address: %s\n",optarg);
                return 1;
            }

            memory_watch_enabled = 1;
            debug_mode = 1;
            break;
#endif
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    if (optind < argc) {
        rom_path = argv[optind];// 获取 ROM 文件路径
    }
    /*CPU 初始化*/
    chip8_t cpu;
    chip8_init(&cpu);

    /* 初始化显示 */
    DisplayState display;
    if (display_init(&display, debug_mode) != 0) {
        return 1;
    }

    /* 初始化音频 */
    audio_init();

    if (rom_path == NULL) {
#if 1        
        printf("===== 显示测试模式 =====\n");
        printf("提示: 运行 ROM 请使用 ./chip8 <rom文件>\n");
        run_test_pattern(&display, &cpu);
        display_cleanup(&display);
        printf("测试结束\n");
#endif        
        return 0;
    }

#if 1
    /* ===== 正常模式 ===== */
    if (!chip8_load(&cpu, rom_path)) {
        printf("加载 ROM 失败\n");
        display_cleanup(&display);
        return 1;
    }

    printf("===== CHIP-8 模拟器 =====\n");
    printf("ROM: %s\n", rom_path);
    printf("PC = 0x%04X\n", cpu.PC);

    /* 调试模式下打印初始状态 */
    if (debug_mode) {
        debug_state = DEBUG_PAUSED;//调试状态设为 PAUSED
        printf("[DBG] paused at start\n");
        debug_print_state(&cpu);
    }
    printf("按 ESC 或关闭窗口退出\n");
 //   printf("键盘映射: Q=左, E=右, 1/2/3/4=按键\n");

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;//关闭窗口
            if (event.type == SDL_KEYDOWN) {//按键按下
                if (event.key.keysym.sym == SDLK_ESCAPE) running = 0;//按 ESC 退出

                // 调试模式下的按键s'pace/n
                if (debug_mode) {
                    if (event.key.keysym.sym == SDLK_SPACE) {// 空格键切换运行/暂停
                        if (debug_state == DEBUG_RUNNING) {// 之前是运行，按空格就暂停
                            debug_state = DEBUG_PAUSED;
                            printf("[DBG] paused\n");
                            debug_print_state(&cpu);
                        } else {//之前是暂停，按空格就继续运行
#if 1                             /* 因为当前是暂停状态，程序检查当前 PC 是否就是断点。有断点，返回1； 无断点，返回0*/
                                  skip_breakpoint_once = has_breakpoint(cpu.PC, breakpoints, breakpoint_count);
#endif
                                  debug_state = DEBUG_RUNNING;
                                  printf("[DBG] running\n");
                                                   
                        }
                    } else if (event.key.keysym.sym == SDLK_n) {// 'n' 单步执行
                               if (debug_state == DEBUG_PAUSED) {
                                   debug_state = DEBUG_STEPPING;
                            }
                    }
#if 0               
                   // 调试模式下的按键+/-
                   if (event.key.keysym.sym == SDLK_EQUALS ||
                       event.key.keysym.sym == SDLK_KP_PLUS) {

                       if (cycles_per_frame < 20) {
                           cycles_per_frame++;
                        }

                       printf("[DBG] speed: %d cycles/frame\n",cycles_per_frame);
                    }

                   if (event.key.keysym.sym == SDLK_MINUS ||
                      event.key.keysym.sym == SDLK_KP_MINUS) {

                      if (cycles_per_frame > 1) {
                          cycles_per_frame--;
                      }

                      printf("[DBG] speed: %d cycles/frame\n",cycles_per_frame);
                  }     
#endif                    
                }

            }

           // 处理 CHIP-8 键盘事件
            input_handle_event(&event, &cpu);
        }
        
        /* 如果 CPU 正在等按键，就先不要执行下一条 CHIP-8 指令*/
        if (!debug_mode) {
            for (int i = 0; i < 10; i++) {
                 chip8_emulate_cycle(&cpu);
            }
        } else {
                   if (debug_state == DEBUG_RUNNING) {
#if 0                    
                       chip8_emulate_cycle(&cpu);
#else                       
                       //从断点恢复的第一次则跳过检查
                   //for (int i = 0;i < cycles_per_frame;i++) 
                       if (skip_breakpoint_once == 0 && has_breakpoint(cpu.PC, breakpoints, breakpoint_count)) {

                           debug_state = DEBUG_PAUSED;

                           printf("[DBG] breakpoint hit at 0x%04X\n", cpu.PC);
                           debug_print_state(&cpu);
                       } else {
                           skip_breakpoint_once = 0;
                           chip8_emulate_cycle(&cpu);
                       }
#endif                           
                    }else if (debug_state == DEBUG_STEPPING) {
                        /* N 键只执行一条指令 */
                       printf("[DBG] before step\n");
                       debug_print_state(&cpu);

                       chip8_emulate_cycle(&cpu);

                       /* 执行一条后重新暂停 */
                       debug_state = DEBUG_PAUSED;

                       printf("[DBG] after step\n");
                       debug_print_state(&cpu);
                    }
        }

        // 更新定时器和音频
        if (!debug_mode || debug_state == DEBUG_RUNNING) {
            if (cpu.delay_timer > 0) cpu.delay_timer--;
            if (cpu.sound_timer > 0) cpu.sound_timer--;
        }
        /* 更新蜂鸣状态（根据 sound_timer） */
        audio_update(cpu.sound_timer);

    
        if (debug_mode) {
            status = debug_state_name(debug_state);
        }
        /* 画完了 更新窗口 */
        display_render(&display, &cpu, status, breakpoints, breakpoint_count);
#if 0        
        display_render(&display, &cpu, status, breakpoints, breakpoint_count, 
                       memory_watch_enabled, (uint16_t)memory_address);  
#endif                       
        cpu.dirty = 0;// 已经刷新完，不需要重复刷新

        /* 60Hz 速度控制 1s 60次*/
        SDL_Delay(16);
    }
#endif
    /* 清理 */
    display_cleanup(&display);//关闭窗口
    audio_cleanup();//关闭音频
    printf("模拟器已退出\n");
    return 0;
}
