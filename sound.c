#include "sound.h"
#include "asm.h"
#include "bank.h"
#include "cx16api.h"
#include "timer.h"

// Noise freq table
const uint16_t noise_freq[] = {2230, 1115, 557};
const uint8_t volumes[] = {63, 47, 31, 15, 0, 0 ,0, 0}; // Corresponding to attenuation values 1,2,4,8,15
/*
    1 = 2 (1 >> 1 = 0)
    2 = 4 (2 >> 1 = 1)
    4 = 8 (4 >> 1 = 2)
    8 = 16(8 >> 1 = 3)
    15 = off (15 >> 1 = 7)
*/

Sound* create_sound(uint16_t bank_offset)
{
    Sound* sound = malloc(sizeof(Sound));
    sound->bank_offset = bank_offset;

    RAM_BANK = SOUND_BANK;
    sound->voice1_offset = read_bank_data(SOUND_BANK, bank_offset) | (read_bank_data(SOUND_BANK, bank_offset + 1) << 8);
    sound->voice2_offset = read_bank_data(SOUND_BANK, bank_offset + 2) | (read_bank_data(SOUND_BANK, bank_offset + 3) << 8);
    sound->voice3_offset = read_bank_data(SOUND_BANK, bank_offset + 4) | (read_bank_data(SOUND_BANK, bank_offset + 5) << 8);
    sound->noise_offset = read_bank_data(SOUND_BANK, bank_offset + 6) | (read_bank_data(SOUND_BANK, bank_offset + 7) << 8);
    sound->voice1_duration = 0;
    sound->voice2_duration = 0;
    sound->voice3_duration = 0;
    sound->noise_duration = 0;

    // Setup Noise channel
    psg_write(12 + 3, 0xFF);

    printf("Sound: %4X %4X %4X %4X\n", sound->voice1_offset, sound->voice2_offset, sound->voice3_offset, sound->noise_offset);

    return sound;
}

void play_sound(Sound* sound)
{
    static uint32_t freq_divisor, frequency;
    static uint8_t volume, i;
    static uint8_t bytes[5];

    // printf("%u %u %u %u\n", sound->voice1_duration, sound->voice2_duration, sound->voice3_duration, sound->noise_duration);

    // Voice 1
    if (sound->voice1_duration <= 0) {
        for (i = 0; i < 5; i++) {
            bytes[i] = read_bank_data(SOUND_BANK, sound->voice1_offset + i);
        }
        sound->voice1_duration = bytes[0] | (bytes[1] << 8);
        if (sound->voice1_duration != 0xFFFF) {
            freq_divisor = ((bytes[2] & 0x3F) << 4) + (bytes[3] & 0x0F);
            frequency = (111860 / freq_divisor) + 1;
            volume = volumes[(bytes[4] & 0x0F) >> 1];

            //printf("Voice 1: %lu %u %u\n", freq_divisor, sound->voice1_duration, volume);
            psg_setfreq(frequency, 0);
            psg_setvol(volume, 0);
            sound->voice1_offset += 5;
        } else {
            psg_setvol(0, 0);
        }
    }

    // Voice 2
    if (sound->voice2_duration <= 0) {
        for (i = 0; i < 5; i++) {
            bytes[i] = read_bank_data(SOUND_BANK, sound->voice2_offset + i);
        }
        sound->voice2_duration = bytes[0] | (bytes[1] << 8);
        if (sound->voice2_duration != 0xFFFF) {
            freq_divisor = ((bytes[2] & 0x3F) << 4) + (bytes[3] & 0x0F);
            frequency = (111860 / freq_divisor) + 1;
            volume = volumes[(bytes[4] & 0x0F) >> 1];

            // printf("Voice 2: %lu %u %u\n", freq_divisor, sound->voice2_duration, volume);
            psg_setfreq(frequency, 1);
            psg_setvol(volume, 1);
            sound->voice2_offset += 5;
        } else {
            psg_setvol(0, 1);
        }
    }

    // Voice 3
    if (sound->voice3_duration <= 0) {
        for (i = 0; i < 5; i++) {
            bytes[i] = read_bank_data(SOUND_BANK, sound->voice3_offset + i);
        }
        sound->voice3_duration = bytes[0] | (bytes[1] << 8);
        if (sound->voice3_duration != 0xFFFF) {
            freq_divisor = ((bytes[2] & 0x3F) << 4) + (bytes[3] & 0x0F);
            frequency = (111860 / freq_divisor) + 1;
            volume = volumes[(bytes[4] & 0x0F) >> 1];

            // printf("Voice 3: %lu %u %u\n", freq_divisor, sound->voice3_duration, volume);
            psg_setfreq(frequency, 2);
            psg_setvol(volume, 2);
            sound->voice3_offset += 5;
        } else {
            psg_setvol(0, 2);
        }
    }

    // Noise
    if (sound->noise_duration <= 0) {
        for (i = 0; i < 5; i++) {
            bytes[i] = read_bank_data(SOUND_BANK, sound->noise_offset + i);
        }
        sound->noise_duration = bytes[0] | (bytes[1] << 8);
        if (sound->noise_duration != 0xFFFF) {
            frequency = noise_freq[bytes[2] & 0x03];
            volume = volumes[(bytes[4] & 0x0F) >> 1];
            psg_setfreq(frequency, 3);
            psg_setvol(volume, 3);
            sound->noise_offset += 5;
        } else {
            psg_setvol(0, 3);
        }
    }
    // Decrement the duration for each voice
    asm_wait_for_refresh();
    sound->voice1_duration--;
    sound->voice2_duration--;
    sound->voice3_duration--;
    sound->noise_duration--;
}

// Function to play a single note on the PSG
void play_psg_note(uint8_t voice, uint16_t frequency, uint8_t volume)
{
    // Set the frequency of the voice
    psg_setfreq(frequency, voice);

    // Set the volume (attenuation) of the voice
    psg_setvol(volume, voice);

    // Turn on the voice at the specified frequency and volume
    psg_playfreq(voice, frequency);
}
