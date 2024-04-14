#ifndef LOGIC_H
#define LOGIC_H
/*
    Logic function execution routines
*/
#include <stdint.h>
#include <stdbool.h>
#include "bank.h"
#include "interp.h"

extern uint8_t bank;

// Jump/Goto/Branch etc from an offset read from current file
void advance(uint16_t* offset);

// DEBUG: Print the opcode and offset
void print_opcode(uint8_t code, uint16_t offset, char* str);

// Execute the next logic function
void execute_logic(uint8_t num);

// Execute a command
void execute_command(uint8_t code, uint16_t *offset);
// Execute a test
bool execute_test(uint8_t code, uint16_t *offset);

// Parse/execute an if statement
void execute_if(uint16_t* offset);

// Get a message from the logic file
char *get_message(uint8_t message_num);
void decrypt_messages();

#endif // LOGIC_H