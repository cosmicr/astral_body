#ifndef TEXT_H
#define TEXT_H
/*
    Text display functions
*/
#include <stdint.h>
#include <cx16.h>

char* process_text(const char* text);

void display_text(uint8_t x, uint8_t y, const char *str);

#endif // TEXT_H