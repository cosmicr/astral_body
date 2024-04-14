#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dprint.h"
#include "interp.h"
#include "logic.h"
#include "screen.h"
#include "text.h"

#define MAX_LINE_LENGTH 32
#define MAX_LINES 20
char lines[MAX_LINES][MAX_LINE_LENGTH + 1]; // Static allocation

/*
    Processes the format specifiers in the text string and returns a new string
*/
/*
    Processes the format specifiers in the text string and populates the lines array
*/
char *process_text(const char* text)
{
    static uint8_t i = 0;
    static uint8_t num_buffer = 0;
    static uint8_t line_index = 0;
    static uint8_t char_index = 0;
    static char c;
    dprintf("\nProcessing text");

    do {
        c = text[i++];
        if (c == '%') {
            char mode = text[i++];
            num_buffer = 0;
            while (isdigit(c = text[i++])) {
                num_buffer = (num_buffer * 10) + (c - '0');
            }

            switch (mode) {
            case 'v': {
                char buf[8];
                int pad = 0;
                if (c == '|') {
                    while (isdigit(c = text[i++])) {
                        pad = (pad * 10) + (c - '0');
                    }
                }
                sprintf(buf, "%0*d", pad, game.vars[num_buffer]);
                strcat(lines[line_index] + char_index, buf);
                char_index += strlen(buf);
                break;
            }
            case 's':
                strcat(lines[line_index] + char_index, game.strings[num_buffer]);
                char_index += strlen(game.strings[num_buffer]);
                break;
            case 'm':
                strcat(lines[line_index] + char_index, get_message(num_buffer));
                char_index += strlen(get_message(num_buffer));
                break;
            }
        }

        switch (c) {
        case '\\':
            continue;
        case '\n':
            lines[line_index][char_index] = '\0';
            line_index++;
            char_index = 0;
            break;
        default:
            lines[line_index][char_index++] = c;
            break;
        }
    } while (c != '\0' && line_index < MAX_LINES);

    if (char_index > 0) {
        lines[line_index][char_index] = '\0';
    }

    return lines;
}

void display_text(uint8_t x, uint8_t y, const char* str)
{
    uint8_t line_count = 0, start_x, start_y;
    uint8_t width = 0, len = 0;
    const char *word_start = str, *str_end = str + strlen(str);
    uint8_t i, line_len = 0;

    // Split the text into lines
    while (word_start < str_end && line_count < MAX_LINES) {
        const char* word_end = strchr(word_start, ' ');
        if (!word_end)
            word_end = str_end;
        len = word_end - word_start;
        if (line_len + len > MAX_LINE_LENGTH) {
            lines[line_count][line_len] = '\0';
            if (line_len > width)
                width = line_len;
            line_len = 0;
            ++line_count;
            if (line_count == MAX_LINES)
                break;
        }
        if (line_len + len <= MAX_LINE_LENGTH) {
            memcpy(lines[line_count] + line_len, word_start, len);
            line_len += len;
            if (word_end < str_end)
                lines[line_count][line_len++] = ' '; // Add space if not the end
        }
        word_start = word_end + 1;
    }
    if (line_len > 0) { // Handle last line if exists
        lines[line_count][line_len] = '\0';
        if (line_len > width)
            width = line_len;
        ++line_count;
    }

    // max text box is 32x21 (with 2 each side, and 1 top/bottom included)
    // print the text (cputc uses petscii hex value)
    // center the text on the tile grid:
    // start_x = (40 / 2 - width / 2) - 1;
    // start_y = 20 / 2 - line_count / 2;
    // using bitshifting:
    start_x = 20 - (width >> 1) - 1;
    start_y = 10 - (line_count >> 1);

    gotoxy(0, 1);

    // create a mask
    // create_black_mask((start_x * 8) + 8, start_y * 8, width * 8, line_count * 8);
    create_black_mask((start_x << 3) + 8, start_y << 3, width << 3, line_count << 3);
    bgcolor(0x0F);
    textcolor(0x00);
    for (i = 0; i < line_count; i++) {
        gotoxy(start_x, start_y + i);
        textcolor(0x04);
        cputc(0x7D); // side border
        textcolor(0x00);
        printf("%-*s", width, lines[i]);
        textcolor(0x04);
        cputcxy(start_x + width + 1, start_y + i, 0x7d); // side border
    }
    // print the border
    textcolor(0x04);
    for (i = 1; i <= width; i++) {
        gotoxy(start_x + i, start_y - 1);
        cputc(0x60); // top border
        gotoxy(start_x + i, start_y + line_count);
        cputc(0x60); // bottom border
    }

    cputcxy(start_x, start_y - 1, 0xB0);                      // top left
    cputcxy(start_x + width + 1, start_y - 1, 0xAE);          // top right
    cputcxy(start_x, start_y + line_count, 0xAD);             // bottom left
    cputcxy(start_x + width + 1, start_y + line_count, 0xBD); // bottom right

    // wait for enter key
    while (!kbhit()) {
    }
    cgetc();

    // free the memory
    free(lines);
    // clear the sprite masks
    clear_sprite_masks();
}
