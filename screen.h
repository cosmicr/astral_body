#ifndef SCREEN_H
#define SCREEN_H
/*
    Screen, palette and text related functions
*/
#define STATUSBAR_OFFSET 8

#define SPRITE_BITMAP_DATA 0x13000

extern uint32_t sprite_attr_offset;

void init_screen();

void setup_ega_palette();

void show_priority();

void show_visual();

void create_black_bitmap();
void create_black_sprite(uint16_t x, uint8_t y, uint8_t width, uint8_t height);
void create_black_mask(uint16_t x, uint8_t y, uint16_t width, uint8_t height);
void create_black_mask_recursive(uint16_t x, uint8_t y, uint16_t width, uint8_t height);
void clear_sprite_masks();

#endif // SCREEN_H