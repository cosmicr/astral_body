#ifndef CX16API_H
#define CX16API_H
/*
    API for the Commander X16 Kernel
*/
#include <stdint.h>

// Get the next character from the keyboard buffer
uint8_t kbdbuf_peek(uint8_t* queue_length);

/* Sound Routines */

// Initialise the PSG sound chip
void psg_init(void);

// Set the frequency of a voice
void psg_setfreq(uint16_t freq, uint8_t voice);

// Set the volume of a voice
void psg_setvol(uint8_t vol, uint8_t voice);

// Play a note
void psg_playfreq(uint16_t freq, uint8_t voice);

// Write to the PSG
void psg_write(uint8_t reg_offset, uint8_t val);


#endif // CX16API_H