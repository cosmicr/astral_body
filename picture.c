#include <conio.h>
#include <stdio.h>

#include "asm.h"
#include "picture.h"
#include "pixel.h"
#include "resource.h"
#include "screen.h"
#include "timer.h"

bool vis_enabled = false, pri_enabled = false;
uint8_t vis_colour = 0, pri_colour = 0, patCode, patNum;

void (*pic_action[11])(uint16_t* offset) = {
    action_F0_setPicColour, action_F1_disablePicColour, action_F2_setPriColour,
    action_F3_disablePriColour, action_F4_yCorner, action_F5_xCorner, action_F6_absoluteLine,
    action_F7_relativeDraw, action_F8_fill, action_F9_setPattern, action_FA_plotBrush};

/*
    Gets the next picture action from the picture data
*/
// uint8_t get_next_pic_action(uint16_t data_offset)
// {
//     return read_heap(data_offset);
// }
#define get_next_pic_action(data_offset) read_heap(data_offset)

/*
    Draws the current picture starting at memory bank 1
*/
void draw_pic(void)
{
    static bool in_progress = true, waitForEach = false;
    uint16_t offset = get_last_entry(picdir_head)->offset;
    static uint8_t action;

    // asm_clear_vis_color(15); // Clear the visual page
    // asm_clear_pri_color(4); // Clear the priority page

    asm("sei");

    action = get_next_pic_action(offset);

    do {
        action = get_next_pic_action(offset++);

        if (action > 0xFA) {
            in_progress = false;
            break;
        }

        // TODO: this is faster/smaller to do these inline
        // if (action == 0xF0) {
        //     vis_colour = get_next_pic_action(offset++);
        //     vis_enabled = true;
        // }

        // if (action == 0xF1) {
        //     vis_enabled = false;
        // }

        // if (action == 0xF2) {
        //     pri_colour = get_next_pic_action(offset++);
        //     pri_enabled = true;
        // }

        // if (action == 0xF3) {
        //     pri_enabled = false;
        // }

        pic_action[action & 0x0F](&offset);

        if (waitForEach) {
            while (!kbhit())
                ;
            cgetc();
        }
    } while (in_progress);

    asm("cli");
}

void show_pic(void)
{
    static uint16_t i;
    asm_clear_screen();

    VERA.control = 1;                  // select data port 1
    VERA.address = 0x2000 + (8 * 160); // low bytes of 0x12000 (plus status bar)
    VERA.address_hi = 0x10 | 0x01;     // auto increment and high byte of 0x12000

    VERA.control = 0;
    VERA.address = 0;
    VERA.address_hi = 0x10;

    for (i = 0; i < 0x6900; i++) { // 160*168
        VERA.data1 = VERA.data0;
    }
}

/*
    Sets the picture colour (drawing action 0xF0)
*/
void action_F0_setPicColour(uint16_t* data_offset)
{
    vis_colour = get_next_pic_action((*data_offset)++);
    vis_enabled = true;
}

/*
    Disables the picture colour (drawing action 0xF1)
*/
void action_F1_disablePicColour(uint16_t* data_offset)
{
    (void)data_offset;
    vis_enabled = false;
}

/*
    Sets the priority colour (drawing action 0xF2)
*/
void action_F2_setPriColour(uint16_t* data_offset)
{
    pri_colour = get_next_pic_action((*data_offset)++);
    pri_enabled = true;
}

/*
    Disables the priority colour (drawing action 0xF3)
*/
void action_F3_disablePriColour(uint16_t* data_offset)
{
    (void)data_offset;
    pri_enabled = false;
}

/*
    Draws a yCorner  (drawing action 0xF4)
*/
void action_F4_yCorner(uint16_t* data_offset)
{
    // Read the initial x and y coordinates
    uint8_t x1 = get_next_pic_action((*data_offset)++);
    uint8_t y1 = get_next_pic_action((*data_offset)++);
    static uint8_t x2, y2;

    // pset(x1, y1);
    if (vis_enabled)
        asm_plot_vis_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, vis_colour);
    if (pri_enabled)
        asm_plot_pri_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, pri_colour);

    while (true) {
        y2 = get_next_pic_action((*data_offset)++);
        if (y2 >= 0xF0)
            break;
        asm_drawline(x1, y1, x1, y2);
        y1 = y2;

        x2 = get_next_pic_action((*data_offset)++);
        if (x2 >= 0xF0)
            break;
        asm_drawline(x1, y1, x2, y1);
        x1 = x2;
    }

    // Adjust the offset back by one
    --(*data_offset);
}

/*
    Draws an a xCorner  (drawing action 0xF5)
*/
void action_F5_xCorner(uint16_t* data_offset)
{
    // Read the initial x and y coordinates
    uint8_t x1 = get_next_pic_action((*data_offset)++);
    uint8_t y1 = get_next_pic_action((*data_offset)++);
    static uint8_t x2, y2;

    // printf("\nDrawing action_F5_xCorner");
    // pset(x1, y1);
    if (vis_enabled)
        asm_plot_vis_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, vis_colour);
    if (pri_enabled)
        asm_plot_pri_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, pri_colour);

    while (true) {
        x2 = get_next_pic_action((*data_offset)++);
        if (x2 >= 0xF0)
            break;
        asm_drawline(x1, y1, x2, y1);
        x1 = x2;
        y2 = get_next_pic_action((*data_offset)++);
        if (y2 >= 0xF0)
            break;
        asm_drawline(x1, y1, x1, y2);
        y1 = y2;
    }

    // Adjust the offset back by one
    --(*data_offset);
}

/*
    Draws long lines to actual locations (cf. relative) (drawing action 0xF6)
*/
void action_F6_absoluteLine(uint16_t* data_offset)
{
    // Read the initial x and y coordinates
    uint8_t x1 = get_next_pic_action((*data_offset)++);
    uint8_t y1 = get_next_pic_action((*data_offset)++);
    static uint8_t x2, y2;

    // printf("\nDrawing action_F6_absoluteLine");

    // pset(x1, y1);
    if (vis_enabled)
        asm_plot_vis_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, vis_colour);
    if (pri_enabled)
        asm_plot_pri_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, pri_colour);

    while (true) {
        x2 = get_next_pic_action((*data_offset)++);
        if (x2 >= 0xF0)
            break;
        y2 = get_next_pic_action((*data_offset)++);
        if (y2 >= 0xF0)
            break;
        asm_drawline(x1, y1, x2, y2);
        x1 = x2;
        y1 = y2;
    }

    // Adjust the offset back by one
    --(*data_offset);
}

/*
    Draws short lines relative to last position.  (drawing action 0xF7)
*/
void action_F7_relativeDraw(uint16_t* data_offset)
{
    // Read the initial x and y coordinates
    uint8_t x1 = get_next_pic_action((*data_offset)++);
    uint8_t y1 = get_next_pic_action((*data_offset)++);
    static uint8_t disp;
    static int8_t dx, dy;

    // pset(x1, y1); // 0x33, 0x82
    if (vis_enabled)
        asm_plot_vis_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, vis_colour);
    if (pri_enabled)
        asm_plot_pri_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, pri_colour);

    // printf("\nDrawing action_F7_relativeDraw at %d, %d", x1, y1);

    while (true) {
        int newx, newy;
        disp = get_next_pic_action(*data_offset);

        if (disp >= 0xF0)
            break;

        dx = (disp & 0xF0) >> 4;
        dy = disp & 0x0F;

        dx = (dx & 0x07) * (1 - 2 * ((dx & 0x08) >> 3));
        dy = (dy & 0x07) * (1 - 2 * ((dy & 0x08) >> 3));

        newx = x1 + dx;
        newy = y1 + dy;

        asm_drawline(x1, y1, newx, newy);

        x1 = newx;
        y1 = newy;

        (*data_offset)++;
    }
}

/*
Agi flood fill.  (drawing action 0xF8)
*/
void action_F8_fill(uint16_t* data_offset)
{
    uint8_t x1, y1;

    while (true) {
        x1 = get_next_pic_action((*data_offset));
        if (x1 >= 0xF0)
            return;
        ++(*data_offset);

        y1 = get_next_pic_action((*data_offset));
        if (y1 >= 0xF0)
            return;
        ++(*data_offset);
        asm_flood_fill(x1, y1);
    }
}

void action_F9_setPattern(uint16_t* data_offset)
{
    patCode = get_next_pic_action((*data_offset)++);
}

const int8_t circles[][15] = {/* agi circle bitmaps */
                              {0x80},
                              {0xfc},
                              {0x5f, 0xf4},
                              {0x66, 0xff, 0xf6, 0x60},
                              {0x23, 0xbf, 0xff, 0xff, 0xee, 0x20},
                              {0x31, 0xe7, 0x9e, 0xff, 0xff, 0xde, 0x79, 0xe3, 0x00},
                              {0x38, 0xf9, 0xf3, 0xef, 0xff, 0xff, 0xff, 0xfe, 0xf9, 0xf3, 0xe3, 0x80},
                              {0x18, 0x3c, 0x7e, 0x7e, 0x7e, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x7e,
                               0x7e, 0x3c, 0x18}};

const uint8_t splatterMap[32] = {/* splatter brush bitmaps */
                                 0x20, 0x94, 0x02, 0x24, 0x90, 0x82, 0xa4, 0xa2,
                                 0x82, 0x09, 0x0a, 0x22, 0x12, 0x10, 0x42, 0x14,
                                 0x91, 0x4a, 0x91, 0x11, 0x08, 0x12, 0x25, 0x10,
                                 0x22, 0xa8, 0x14, 0x24, 0x00, 0x50, 0x24, 0x04};

const uint8_t splatterStart[128] = {/* starting bit position */
                                    0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,
                                    0x60, 0xbd, 0x89, 0x05, 0x0a, 0xf4, 0x7d, 0x7d,
                                    0x85, 0xb0, 0x8e, 0x95, 0x1f, 0x22, 0x0d, 0xdf,
                                    0x2a, 0x78, 0xd5, 0x73, 0x1c, 0xb4, 0x40, 0xa1,
                                    0xb9, 0x3c, 0xca, 0x58, 0x92, 0x34, 0xcc, 0xce,
                                    0xd7, 0x42, 0x90, 0x0f, 0x8b, 0x7f, 0x32, 0xed,
                                    0x5c, 0x9d, 0xc8, 0x99, 0xad, 0x4e, 0x56, 0xa6,
                                    0xf7, 0x68, 0xb7, 0x25, 0x82, 0x37, 0x3a, 0x51,
                                    0x69, 0x26, 0x38, 0x52, 0x9e, 0x9a, 0x4f, 0xa7,
                                    0x43, 0x10, 0x80, 0xee, 0x3d, 0x59, 0x35, 0xcf,
                                    0x79, 0x74, 0xb5, 0xa2, 0xb1, 0x96, 0x23, 0xe0,
                                    0xbe, 0x05, 0xf5, 0x6e, 0x19, 0xc5, 0x66, 0x49,
                                    0xf0, 0xd1, 0x54, 0xa9, 0x70, 0x4b, 0xa4, 0xe2,
                                    0xe6, 0xe5, 0xab, 0xe4, 0xd2, 0xaa, 0x4c, 0xe3,
                                    0x06, 0x6f, 0xc6, 0x4a, 0xa4, 0x75, 0x97, 0xe1};

/*
Draws pixels, circles, squares, or splatter brush patterns depending on the pattern code.
*/
void plotPattern(uint8_t x, uint8_t y)
{
    static int8_t circlePos = 0;
    static uint8_t x1, y1, penSize;
    uint8_t bitPos = splatterStart[patNum];
    static uint8_t halfPenSize, adjustedPenSize;

    // printf("\nDrawing plotPattern");

    penSize = (patCode & 7);

    if (x < ((penSize << 1) + 1))
        x = ((penSize << 1) + 1);
    else if (x > 160 - ((penSize << 1) + 1))
        x = 160 - ((penSize << 1) + 1);
    if (y < penSize)
        y = penSize;
    else if (y >= 168 - penSize)
        y = 167 - penSize;

    halfPenSize = penSize >> 1;
    adjustedPenSize = halfPenSize + (penSize & 1);

    for (y1 = y - penSize; y1 <= y + penSize; y1++) {
        for (x1 = x - adjustedPenSize; x1 <= x + halfPenSize; x1++) {
            if (patCode & 0x10) { /* Square */
                plotPatternPoint();
            } else { /* Circle */
                if ((circles[patCode & 7][circlePos >> 3] >> (7 - (circlePos & 7))) & 1) {
                    plotPatternPoint();
                }
                ++circlePos;
            }
        }
    }
}

/*
Plots points and various brush patterns.
*/
void action_FA_plotBrush(uint16_t* data_offset)
{
    uint8_t x1, y1;

    // printf("\nDrawing action_FA_plotBrush");

    while (true) {
        if (patCode & 0x20) {
            if ((patNum = get_next_pic_action((*data_offset)++)) >= 0xF0)
                break;
            patNum = (patNum >> 1 & 0x7f);
        }
        if ((x1 = get_next_pic_action((*data_offset)++)) >= 0xF0)
            break;
        if ((y1 = get_next_pic_action((*data_offset)++)) >= 0xF0)
            break;
        plotPattern(x1, y1);
    }

    // Adjust the offset back by one
    (*data_offset)--;
}