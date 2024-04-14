#ifndef PICTURE_H
#define PICTURE_H
/*
    AGI Picture loading, drawing and action routines
*/

#include <stdint.h>
#include <stdbool.h>

extern bool vis_enabled, pri_enabled; // these are also used by pixel.c
extern uint8_t vis_colour, pri_colour, patCode, patNum; // these are also used by pixel.c

void draw_pic(void);
uint8_t get_next_pic_action(uint16_t data_offset);

void action_F0_setPicColour(uint16_t* data_offset);
void action_F1_disablePicColour(uint16_t* data_offset);
void action_F2_setPriColour(uint16_t* data_offset);
void action_F3_disablePriColour(uint16_t* data_offset);
void action_F4_yCorner(uint16_t* data_offset);
void action_F5_xCorner(uint16_t* data_offset);
void action_F6_absoluteLine(uint16_t* data_offset);
void action_F7_relativeDraw(uint16_t* data_offset);
void action_F8_fill(uint16_t* data_offset);
void action_F9_setPattern(uint16_t* data_offset);
void action_FA_plotBrush(uint16_t* data_offset);

// macro for plotPattern and action_FA_plotBrush from showpic.c
#define plotPatternPoint()                                        \
    if (patCode & 0x20) {                                         \
        if ((splatterMap[bitPos >> 3] >> (7 - (bitPos & 7))) & 1) \
            pset(x1, y1);                                         \
        bitPos++;                                                 \
        if (bitPos == 0xff)                                       \
            bitPos = 0;                                           \
    } else                                                        \
        pset(x1, y1)

void plotPattern(uint8_t x, uint8_t y);
void action_FA_plotBrush(uint16_t* data_offset);

#endif // PICTURE_H