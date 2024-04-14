#include "cx16api.h"
#include "bank.h"

/*
    Get the next character in the keyboard queue without removing it,
    and place the length of the queue in the variable queue_length.
 */
uint8_t kbdbuf_peek(uint8_t* queue_length)
{
    static uint8_t byte;

    // JSR to $FEBD (kbd_get_char)
    __asm__("jsr $febd");

    // Store the X register (queue length) in the length variable
    __asm__("stx %v", byte);
    *queue_length = byte;

    // if length is 0, return 0
    if (byte == 0) {
        return 0;
    }

    // Copy the A register (next char) to 'next_char' variable
    __asm__("lda %v", byte);

    return byte; // return the next char
}

/*
    Initialize the state of the PSG. Silence all voices.
    Reset the attenuation levels to 0.
    Set "playstring" defaults including O4, T120, S1, and L4.
    Set all PSG voices to the pulse waveform at 50% duty with panning set to both L+R
*/
void psg_init(void)
{
    uint8_t OLD_BANK = ROM_BANK;
    ROM_BANK = 0x0A;

    __asm__("jsr $C04B");

    ROM_BANK = OLD_BANK;
}

/*
    Set the frequency of a PSG voice without changing any other attributes of the voice
*/
void psg_setfreq(uint16_t freq, uint8_t voice)
{
    /*
        .A = voice (register is already set)
        .X .Y = 16 bit frequency in VERA PSG format
    */
    static uint8_t A, X, Y, OLD_BANK;
    uint32_t frequency_word;

    A = voice;

    frequency_word = (freq * 131072U) / 48828U;
    X = frequency_word & 0xFF;
    Y = frequency_word >> 8;

    OLD_BANK = ROM_BANK;
    ROM_BANK = 0x0A;

    __asm__("lda %v", A);
    __asm__("ldx %v", X);
    __asm__("ldy %v", Y);

    __asm__("jsr $C057");

    ROM_BANK = OLD_BANK;
}

/*
    Set the volume for the voice.
    The volume that's written to the VERA has attenuation applied.
    Valid volumes range from $00 to $3F inclusive
*/
void psg_setvol(uint8_t vol, uint8_t voice)
{
    static uint8_t A,X, OLD_BANK;

    A = voice;
    X = vol;

    OLD_BANK = ROM_BANK;
    ROM_BANK = 0x0A;

    __asm__("lda %v", A);
    __asm__("ldx %v", X);
    __asm__("jsr $C05D");

    ROM_BANK = OLD_BANK;
}

/*
    Turn on a PSG voice at full volume (factoring in attenuation) and set its frequency
*/
void psg_playfreq(uint16_t freq, uint8_t voice) {
    static uint8_t A, X, Y, OLD_BANK;
    uint32_t new_freq;

    A = voice;

    new_freq = (freq * 131072U) / 48828U;

    X = new_freq & 0xFF;
    Y = new_freq >> 8;

    OLD_BANK = ROM_BANK;
    ROM_BANK = 0x0A;

    __asm__("lda %v", A);
    __asm__("ldx %v", X);
    __asm__("ldy %v", Y);

    __asm__("jsr $C04E");

    ROM_BANK = OLD_BANK;
}

/*
    Write a value to one of the VERA PSG registers
*/
void psg_write(uint8_t reg_offset, uint8_t val)
{
    static uint8_t A, X, OLD_BANK;

    A = val;
    X = reg_offset;

    OLD_BANK = ROM_BANK;
    ROM_BANK = 0x0A;

    __asm__("lda %v", A);
    __asm__("ldx %v", X);

    __asm__("jsr $C060");

    ROM_BANK = OLD_BANK;
}