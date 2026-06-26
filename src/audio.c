#include "../include/audio.h"
#include <stdio.h>

static SDL_AudioDeviceID audio_dev = 0;
//static int beep_active = 0;

/* 音频回调：生成方波 */
static void audio_callback(void *userdata, Uint8 *stream, int len)
{
    (void)userdata;

    Sint16 *buf = (Sint16 *)stream;
    int samples = len / 2;

    static int tick = 0;

    for (int i = 0; i < samples; i++) {
        int half = 44100 / 440 / 2;// 采样率 / 频率 / 2

        buf[i] = (tick < half) ? 20000 : -20000;// 生成方波

        tick++;// 计数器
        if (tick >= half * 2) {// 计数器归零
            tick = 0;
        }
    }
}

void audio_init(void)
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        printf("音频子系统初始化失败: %s\n", SDL_GetError());
        return;
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);// 清零   
    want.freq = 44100;// 采样率
    want.format = AUDIO_S16SYS;// 16位有符号整数
    want.channels = 1;// 单声道
    want.samples = 1024;// 缓冲区大小
    want.callback = audio_callback;// 回调函数

    audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);// 打开音频设备
    if (audio_dev == 0) {
        printf("音频初始化失败: %s\n", SDL_GetError());
    }
}

void audio_update(uint8_t sound_timer)
{
    if (!audio_dev) {
        return;
    }

    if (sound_timer > 0) {
        SDL_PauseAudioDevice(audio_dev, 0);// 开启音频播放
    } else {
        SDL_PauseAudioDevice(audio_dev, 1);// 停止音频播放
    }
}

void audio_cleanup(void)
{
    if (audio_dev) {
        SDL_CloseAudioDevice(audio_dev);// 关闭音频设备
    }
}