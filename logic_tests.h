#ifndef LOGIC_TESTS_H
#define LOGIC_TESTS_H
/*
    Logic tests implementation for the LOGIC game code
*/
#include <stdint.h>
#include <stdbool.h>

bool test_equaln(uint16_t* offset);
bool test_equalv(uint16_t* offset);
bool test_lessn(uint16_t* offset);
bool test_lessv(uint16_t* offset);
bool test_greatern(uint16_t* offset);
bool test_greaterv(uint16_t* offset);
bool test_isset(uint16_t* offset);
bool test_issetv(uint16_t* offset);
bool test_has(uint16_t* offset);
bool test_obj_in_room(uint16_t* offset);
bool test_posn(uint16_t* offset);
bool test_controller(uint16_t* offset);
bool test_have_key(uint16_t* offset);
bool test_said(uint16_t* offset);
bool test_compare_strings(uint16_t* offset);
bool test_obj_in_box(uint16_t* offset);
bool test_center_posn(uint16_t* offset);
bool test_right_posn(uint16_t* offset);

#endif // LOGIC_TESTS_H