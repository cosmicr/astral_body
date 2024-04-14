#include <conio.h>
#include <cx16.h>
#include <stdint.h>
#include <stdio.h>

#include "asm.h"
#include "screen.h"

uint32_t sprite_attr_offset = 0x1FE80; // start at sprite 80 for text mask

void init_screen()
{
    uint8_t i;
    asm_init_vera();
    setup_ega_palette();
    asm_clear_overlay();
    asm_clear_vis_color(15);
    asm_clear_pri_color(4);

    // set bounding lines
    asm_plot_vis_hline(0, 320, 168, 0);
    asm_plot_vis_hline(0, 320, 200, 0);
    asm_plot_pri_hline(0, 320, 168, 0);

    gotoxy(0, 22);
    bgcolor(0x00);
    textcolor(0x0f);

    create_black_bitmap(); // for masking behind text
    asm_clear_screen();
}

void setup_ega_palette()
{
    uint8_t p, q, i = 0;
    // Define the EGA palette colors
    uint8_t ega_palette[16][3] = {
        {0, 0, 0}, {0, 0, 170}, {0, 170, 0}, {0, 170, 170}, {170, 0, 0}, {170, 0, 170}, {170, 85, 0}, {170, 170, 170}, {85, 85, 85}, {85, 85, 255}, {85, 255, 85}, {85, 255, 255}, {255, 85, 85}, {255, 85, 255}, {255, 255, 85}, {255, 255, 255}};
    // uint8_t ega_palette[16][3] = {
    //     {20, 20, 30},    // Moonlit Night Sky
    //     {20, 40, 110},   // Deep Ocean
    //     {30, 180, 30},   // Fresh Grass
    //     {80, 190, 190},  // Mountain Lake
    //     {180, 70, 50},   // Autumn Leaves
    //     {150, 30, 100},  // Wild Berry
    //     {120, 70, 40},   // Tree Bark
    //     {190, 190, 180}, // Pebble Beach
    //     {100, 100, 90},  // Darker Storm Cloud
    //     {155, 205, 255}, // Midday Sky
    //     {80, 160, 80},   // Moss
    //     {180, 240, 240}, // Glacier Ice
    //     {220, 160, 130}, // Flesh Tone
    //     {255, 130, 150}, // Sunset
    //     {255, 180, 90},  // Savannah Sunset
    //     {255, 255, 255}  // Snow Peak
    // };

    do {
        p = i >> 4; // equivalent to i / 16
        q = i & 15; // equivalent to i % 16
        asm_set_palette(i, ega_palette[(p == q) ? 0 : q][0], ega_palette[(p == q) ? 0 : q][1], ega_palette[(p == q) ? 0 : q][2]);
        i++;
    } while (i != 0);
}

void show_priority()
{
    VERA.layer0.tilebase = 0x4c; // 0x4c * 2048 = 0x9800
}

void show_visual()
{
    VERA.layer0.tilebase = 0x00; // 0x00 * 2048 = 0x0000
}

/*
    Creates a 64x64 black sprite area used for masking
*/
void create_black_bitmap()
{
    int i;
    // Fill the sprite graphic memory with black
    VERA.address = SPRITE_BITMAP_DATA & 0xFFFF;
    VERA.address_hi = (SPRITE_BITMAP_DATA >> 16) & 0xFF;
    VERA.address_hi |= 0x10; // Set auto-increment
    for (i = 0; i < 4096; ++i) { // 64x64 pixels
        VERA.data0 = 0x11; // Black color index for palette 1
    }
}

void create_black_sprite(uint16_t x, uint8_t y, uint8_t width, uint8_t height)
{
    static uint8_t byte = 0;
    // return if we have more than 128 sprites
    if (sprite_attr_offset >= 0x20000)
        return;

    // Configure sprite to cover the text area
    VERA.address = sprite_attr_offset & 0xFFFF;
    VERA.address_hi = (sprite_attr_offset >> 16) & 0xFF;
    VERA.address_hi |= 0x10; // Set auto-increment

    VERA.data0 = (SPRITE_BITMAP_DATA >> 5) & 0x00FF;         // 0: Address bits 12:5
    VERA.data0 = 0x00 | ((SPRITE_BITMAP_DATA >> 13) & 0x0F); // 1: 4bpp mode, Address bits 16:13
    VERA.data0 = x;                                          // 2: X position
    VERA.data0 = x >> 8;                                     // 3: X position
    VERA.data0 = y;                                          // 4: Y position
    VERA.data0 = y >> 8;                                     // 5: Y position
    VERA.data0 = 0b00001000;                                 // 6: Z-depth between layer 0 and layer 1

    // Set sprite width and height
    // Upper 4 bits of last byte
    // 0bHHWW0000
    // where: 0 - 8px
    //        1 - 16px
    //        2 - 32px
    //        3 - 64px
    byte = (width == 64 ? (3 << 4) : width & 0xF0);
    byte |= (height == 64 ? (3 << 6) : ((height & 0xF0) << 2));
    byte |= 0x01; // Set palette 1
    VERA.data0 = byte;

    sprite_attr_offset += 8;
}

void clear_sprite_masks()
{
    static uint8_t num_sprites, i;
    num_sprites = (sprite_attr_offset - 0x1FE80) / 8;

    for (i = 0; i < num_sprites; i++) {
        uint32_t addr = 0x1FE80 + (i * 8) + 6;
        VERA.address = addr & 0xFFFF;
        VERA.address_hi = (addr >> 16) & 0xFF;
        VERA.address_hi |= 0x40; // Set auto-increment to 4
        VERA.data0 = 0x00;
    }
}

/*
    Uses multiple sprites to create a black mask
*/
void create_black_mask(uint16_t x, uint8_t y, uint16_t width, uint8_t height)
{
    // TODO: This could be done in assembly
    uint16_t w, rem_w, j;
    uint8_t i, h, rem_h;

    // Subdivide the area
    for (i = 0; i < height; i += h) {
        for (j = 0; j < width; j += w) {
            // Calculate the remaining width and height
            rem_w = width - j;
            rem_h = height - i;

            // Determine the sprite size (this relies on the width and height being multiples of 8)
            w = 64;
            while (w > rem_w)
                w >>= 1;
            h = 64;
            while (h > rem_h)
                h >>= 1;

            // Create a black sprite at the current position
            create_black_sprite(x + j, y + i, w, h);
        }
    }
}