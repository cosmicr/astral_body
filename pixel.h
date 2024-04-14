#ifndef PIXEL_H
#define PIXEL_H
/*
    Pixel drawing routines (used mainly by picture.c)
*/

#include <stdbool.h>
#include <stdint.h>

#include "fillstack.h"
#include "picture.h"

// Draw the current color on vis, pri, or both
void pset(uint16_t x, uint8_t y);

void scan_and_fill(uint8_t x, uint8_t y);
bool can_fill(uint8_t x, uint8_t y);
void flood_fill(uint8_t x, uint8_t y);

#endif // PIXEL_H