/*
 * Guidelines for 65c02 Programming with cc65 for Commander X16
 * -------------------------------------------------------------
 * - Adhere to C89 standards; avoid complete C99 features.
 * - Use uint8_t and int8_t for 8-bit architecture efficiency.
 * - Minimize memory usage; the 65c02 has limited RAM.
 * - Avoid complex floating-point arithmetic; prefer fixed-point.
 * - Leverage cc65 extensions: void data, custom keywords.
 * - Utilize inline assembly for performance-critical sections.
 * - Be mindful of the Commander X16's specific hardware resources.
 * - Declarations must be at the beginning of blocks.
 * - Efficient coding practices are crucial due to limited resources.
 * - Reduce stack usage; avoid deep recursion and large stacks.
 * - ASM variables must be static. Use %b for immediate byte values or %v for addresses.
 * - Only one __asm__ call per line.
 */
#include <cbm.h>
#include <conio.h>
#include <ctype.h>
#include <cx16.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
features:
    - logic evaluation
    - sound
    - views - how should views work? use sprites? or bitmaps? make layer 1 bitmap layer for priority overlays on views (eg a rock)?
     - if layer 1 is a bitmap, then text needs to be redone
     - if using sprites, a "copy" of background needs to be overlayed on sprite as another sprite (better option?)
    - save/load
    - joystick
    - game loader/identifier
    - menu system

fixes:
    - update all asm functions to work with 320x200 sized memory areas (will save about 6k per screen)
    - set a permanent vram memory location for priority screen (wasted space)
    - DEBUG flag for printf statements
    - rename all pic* variables to vis*
    - rename functions to distinguish between full or half pixels
    - preload and cache pictures? maybe just neighbouring pictures?
    - version 3 support
    - error reporting functions
*/

// Program data gets up to ~38k of main memory (current size is 20k approx.)

#include "asm.h"
#include "cx16api.h"
#include "dprint.h"
#include "interp.h"
#include "picture.h"
#include "pixel.h"
#include "resource.h"
#include "screen.h"
#include "sound.h"
#include "text.h"
#include "timer.h"

static bool quit = false; // todo: put somewhere else

int main()
{
    uint16_t i;
    Sound* sound;
    volatile uint8_t* gif_recorder = (uint8_t*)0x9fb5;

    dprintf("\nReading files...");

    init_resources();
    printf("\nLoading please wait...");

    interp_init();

    // Initialize the screen
    init_screen();

    // Initialize the PSG
    psg_init();

    load_resource(picdir, 1);
    // //load_file("testpics/test.pic");

    start_stopwatch();
    draw_pic();
    end_stopwatch();
    show_pic();

    // load_resource(snddir, 65);
    // sound = create_sound(get_last_entry(snddir_head)->offset);

    // while (!kbhit()) {
    //     asm_wait_for_refresh(); // todo: this is done in the interpreter loop normally
    //     play_sound(sound);
    // }

    // Turn off all voices by setting volumes to 0
    psg_setvol(0, 0);
    psg_setvol(0, 1);
    psg_setvol(0, 2);

    display_text(0, 0, "1234567890. The quick brown fox jumps over the lazy dog.");

    asm_clear_overlay();
    // display_text(0,0,"Some more text.");

    // while(!kbhit());
    // cgetc();

    *gif_recorder = 0;

    asm_clear_overlay();

    while (!quit) {
        interp_run();
    }

    return 0;
}