#ifndef RESOURCE_H
#define RESOURCE_H
/*
    AGI Resource loading routines
*/
#include <stdint.h>

#include "bank.h"

// todo: consider another data type (struct) for optimisation
enum kind {
    LOGIC = LOGDIR_BANK,
    PICTURE = PICDIR_BANK,
    VIEW = VIEWDIR_BANK,
    SOUND = SNDDIR_BANK,
    WORDS = WORDS_BANK,
    OBJECT = OBJECT_BANK
};

extern uint8_t last_logic_num;
extern uint8_t last_pic_num;
// todo: view loading management

void init_resources(void);

void load_file(char* filename, uint8_t bank);

void load_resource(enum kind dir, uint8_t num);

#endif