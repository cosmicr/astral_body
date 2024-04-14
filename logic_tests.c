#include "logic.h"
#include "bank.h"
#include "logic_tests.h"
#include "interp.h"
#include "dprint.h"
#include <string.h>

bool test_equaln(uint16_t* offset)
{
    return (game.vars[read_bank_data(bank, (*offset)++)] == read_bank_data(bank, (*offset)++));
}

bool test_equalv(uint16_t* offset)
{
    return (game.vars[read_bank_data(bank, (*offset)++)] == game.vars[read_bank_data(bank, (*offset)++)]);
}

bool test_lessn(uint16_t* offset)
{
    uint8_t var = read_bank_data(bank, (*offset)++);
    uint8_t num = read_bank_data(bank, (*offset)++);
    return (game.vars[var] < num);
}

bool test_lessv(uint16_t* offset)
{
    uint8_t var1 = read_bank_data(bank, (*offset)++);
    uint8_t var2 = read_bank_data(bank, (*offset)++);
    return (game.vars[var1] < game.vars[var2]);
}

bool test_greatern(uint16_t* offset)
{
    uint8_t var = read_bank_data(bank, (*offset)++);
    uint8_t num = read_bank_data(bank, (*offset)++);
    dprintf_args("greatern(var %d (%d), %d)", var, game.vars[var], num);
    return (game.vars[var] > num);
}

bool test_greaterv(uint16_t* offset)
{
    uint8_t var1 = read_bank_data(bank, (*offset)++);
    uint8_t var2 = read_bank_data(bank, (*offset)++);
    return (game.vars[var1] > game.vars[var2]);
}

bool test_isset(uint16_t* offset)
{
    uint8_t flag = read_bank_data(bank, (*offset)++);
    return (game.flags[flag]);
}

bool test_issetv(uint16_t* offset)
{
    uint8_t var = read_bank_data(bank, (*offset)++);
    return (game.flags[game.vars[var]]);
}

bool test_has(uint16_t* offset)
{
    // TODO: test_has
    (*offset)++;
    dprintf("test_has not implemented\n");
    return false;
}

bool test_obj_in_room(uint16_t* offset)
{
    uint8_t obj, room;
    obj = read_bank_data(bank, (*offset)++);
    room = read_bank_data(bank, (*offset)++);
    //return (game.objects[obj].room == room);
    dprintf("test_obj_in_room not implemented\n");
    return false; // TODO: test_obj_in_room
}

bool test_posn(uint16_t* offset)
{
    uint8_t obj = read_bank_data(bank, (*offset)++);
    uint8_t x1 = read_bank_data(bank, (*offset)++);
    uint8_t y1 = read_bank_data(bank, (*offset)++);
    uint8_t x2 = read_bank_data(bank, (*offset)++);
    uint8_t y2 = read_bank_data(bank, (*offset)++);
    uint8_t obj_x = game.objects[obj].x;
    uint8_t obj_y = game.objects[obj].y;
    return (obj_x >= x1 && obj_x <= x2 && obj_y >= y1 && obj_y <= y2);
}

bool test_controller(uint16_t* offset)
{
    // TODO: test_controller
    (*offset)++;
    dprintf("test_controller not implemented\n");
    return false;
}

bool test_have_key(uint16_t* offset)
{
    (void)offset; // ignore offset
    return game.vars[19] > 0; // v19 = pressed key
}

bool test_said(uint16_t* offset)
{
    uint8_t num_words;
    num_words = read_bank_data(bank, (*offset)++);
    (*offset) += 2 * num_words;
    dprintf("test_said not implemented\n");
    return false; // TODO: test_said
}

bool test_compare_strings(uint16_t* offset)
{
    uint8_t str1 = read_bank_data(bank, (*offset)++);
    uint8_t str2 = read_bank_data(bank, (*offset)++);
    return strcmp(game.strings[str1], game.strings[str2]) == 0;
}

// tests if entire width of object is in box
bool test_obj_in_box(uint16_t* offset)
{
    uint8_t obj = read_bank_data(bank, (*offset)++);
    uint8_t x1 = read_bank_data(bank, (*offset)++);
    uint8_t y1 = read_bank_data(bank, (*offset)++);
    uint8_t x2 = read_bank_data(bank, (*offset)++);
    uint8_t y2 = read_bank_data(bank, (*offset)++);
    uint8_t obj_x = game.objects[obj].x;
    uint8_t obj_y = game.objects[obj].y;
    uint8_t obj_width = game.objects[obj].width;
    return (obj_x + obj_width >= x1 && obj_x <= x2 && obj_y >= y1 && obj_y <= y2);
}

bool test_center_posn(uint16_t* offset)
{
    uint8_t obj = read_bank_data(bank, (*offset)++);
    uint8_t x1 = read_bank_data(bank, (*offset)++);
    uint8_t y1 = read_bank_data(bank, (*offset)++);
    uint8_t x2 = read_bank_data(bank, (*offset)++);
    uint8_t y2 = read_bank_data(bank, (*offset)++);
    uint8_t obj_x_center = game.objects[obj].x + game.objects[obj].width / 2;
    uint8_t obj_y_center = game.objects[obj].y + game.objects[obj].height / 2;
    return (obj_x_center >= x1 && obj_x_center <= x2 && obj_y_center >= y1 && obj_y_center <= y2);
}

bool test_right_posn(uint16_t* offset)
{
    uint8_t obj = read_bank_data(bank, (*offset)++);
    uint8_t x1 = read_bank_data(bank, (*offset)++);
    uint8_t y1 = read_bank_data(bank, (*offset)++);
    uint8_t x2 = read_bank_data(bank, (*offset)++);
    uint8_t y2 = read_bank_data(bank, (*offset)++);
    uint8_t obj_x_right = game.objects[obj].x + game.objects[obj].width;
    uint8_t obj_y = game.objects[obj].y;
    return (obj_x_right >= x1 && obj_x_right <= x2 && obj_y >= y1 && obj_y <= y2);
}