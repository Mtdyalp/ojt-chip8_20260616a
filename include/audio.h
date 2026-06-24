#ifndef AUDIO_H
#define AUDIO_H

#include <SDL2/SDL.h>
#include <stdint.h>

void audio_init(void);
void audio_update(uint8_t sound_timer);
void audio_cleanup(void);

#endif