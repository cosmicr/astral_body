#include <conio.h>
#include <cx16.h>
#include <stdio.h>
#include <stdlib.h>

#include "asm.h"
#include "pixel.h"
#include "screen.h"
#include "timer.h"

/*
    Draws a pixel in each screen depending on whether drawing in that
    screen is enabled or not.
*/
void pset(uint16_t x, uint8_t y)
{
    // Used in pen plotting
    if (vis_enabled)
        asm_plot_vis_pixel((x << 1), y + STATUSBAR_OFFSET, vis_colour);
    if (pri_enabled)
        asm_plot_pri_pixel((x << 1), y + STATUSBAR_OFFSET, pri_colour);
}

/*
    Fills current scanline and pushes adjacent scanlines onto the stack.
*/
void scan_and_fill(uint8_t x, uint8_t y)
{
    static uint8_t lx, rx;

    // Inline can_fill logic at the start to avoid unnecessary function calls
    if (asm_can_fill(x, y) == false) {
        return;
    }

    lx = x;
    rx = x;

    // Inline can_fill logic for left expansion
    while (lx != 0) {
        if (asm_can_fill(lx - 1, y) == false) {
            break;
        }
        --lx;
    }

    // Inline can_fill logic for right expansion
    while (rx != 159) {
        if (asm_can_fill(rx + 1, y) == false) {
            break;
        }
        ++rx;
    }

    // pset_hline(lx, rx, y);
    if (vis_enabled)
        asm_plot_vis_hline_fast((lx << 1), (rx << 1) + 2, y + STATUSBAR_OFFSET, vis_colour);
    if (pri_enabled)
        asm_plot_pri_hline_fast((lx << 1), (rx << 1) + 2, y + STATUSBAR_OFFSET, pri_colour);

    // if (y != 167) {
    //     push(lx, rx, y + 1, 1); // push below
    // }
    // if (y != 0) {
    //     push(lx, rx, y - 1, -1); // push above
    // }
    push(lx, rx, y + 1); // push below
    push(lx, rx, y - 1); // push above
}

/*
    Function to check if a pixel can be filled.
*/
bool can_fill(uint8_t x, uint8_t y)
{
    if (vis_colour == 15) {
        return false;
    }

    if (!pri_enabled && (asm_get_vis_pixel(x, y) != 15)) {
        return false;
    }

    if (pri_enabled && !vis_enabled && (asm_get_pri_pixel(x, y) != 4)) {
        return false;
    }

    if (pri_enabled && (asm_get_vis_pixel(x, y) != 15)) {
        return false;
    }

    return true;
}

/*
    Span filling flood fill algorithm.
*/
// void flood_fill(uint8_t x, uint8_t y)
// {
//     static uint8_t lx, rx, y1, nx;

//     if (!asm_can_fill(x, y)) {
//         return;
//     }

//     fill_stack_pointer = 0;
//     scan_and_fill(x, y);

//     while (pop(&lx, &rx, &y1)) {
//         for (nx = lx; nx <= rx; ++nx) {
//             if (asm_can_fill(nx, y1)) {
//                 scan_and_fill(nx, y1);
//                 while (nx <= rx && asm_can_fill(nx, y1)) {
//                     ++nx;
//                 }
//             }
//         }
//     }
// }