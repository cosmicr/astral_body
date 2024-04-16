#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Structure to hold the offsets for each voice's data
typedef struct {
    uint32_t offset;
    uint32_t voice1_offset;
    int16_t voice1_duration;
    uint32_t voice2_offset;
    int16_t voice2_duration;
    uint32_t voice3_offset;
    int16_t voice3_duration;
    uint32_t noise_offset;
    int16_t noise_duration;
} Sound;

// Function to init an AGI sound
Sound *create_sound(uint32_t offset);

// Function to play a sound
void play_sound(Sound *sound);

// Function to play a single note on the PSG
void play_psg_note(uint8_t voice, uint16_t frequency, uint8_t volume);


#endif // SOUND_H