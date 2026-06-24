#include "../include/audio.h"
#include <stdio.h>

static SDL_AudioDeviceID audio_dev = 0;
static int beep_active = 0;

/* 音频回调：生成方波 */
static void audio_callback(void *userdata, Uint8 *stream, int len)
{
    (void)userdata;
    Sint16 *buf = (Sint16 *)stream;
    int samples = len / 2;

    static int tick = 0;               /* ← 加这个 */

    for (int i = 0; i < samples; i++) {
        if (!beep_active) {
            buf[i] = 0;
        } else {
            int half = 44100 / 440 / 2;   /* 半个周期 ≈ 50 样本 */
            buf[i] = (tick < half) ? 20000 : -20000;   /* 方波：正 → 负 */
            tick++;
            if (tick >= half * 2) tick = 0;
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
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;

    audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_dev == 0) {
        printf("音频初始化失败: %s\n", SDL_GetError());
    }
}

void audio_update(uint8_t sound_timer)
{
    if (!audio_dev) {
        return;
    }

    int should_beep;

    if (sound_timer > 0) {
        should_beep = 1;
    } else {
        should_beep = 0;
    }

    if (should_beep && !beep_active) {
        beep_active = 1;
        SDL_PauseAudioDevice(audio_dev, 0);//
    } else if (!should_beep && beep_active) {
        beep_active = 0;
        SDL_PauseAudioDevice(audio_dev, 1);//暂停音频
    }
}

void audio_cleanup(void)
{
    if (audio_dev) {
        SDL_CloseAudioDevice(audio_dev);
    }
}