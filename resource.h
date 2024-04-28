#ifndef RESOURCE_H
#define RESOURCE_H
/*
    AGI Resource loading routines
*/
#include <stdint.h>

extern uint32_t logdir;
extern uint32_t picdir;
extern uint32_t viewdir;
extern uint32_t snddir;

extern uint32_t words_tok;
extern uint32_t object;

// Linked list to keep track off offset for each loaded resource
typedef struct dir_entry {
    uint8_t res_num;
    uint32_t offset;
    struct dir_entry* next;
} dir_entry_t;

extern dir_entry_t* logdir_head;
extern dir_entry_t* picdir_head;
extern dir_entry_t* viewdir_head;
extern dir_entry_t* snddir_head;

volatile uint8_t read_heap(uint32_t offset);
void write_heap(uint32_t offset, uint8_t data);
uint32_t load_file(char* filename);
void init_resources(void);
void load_resource(uint32_t directory, uint8_t num);
void unload_resource(uint32_t directory, uint8_t num);
dir_entry_t* add_to_dirlist(dir_entry_t** head, uint32_t offset);
uint32_t get_offset(dir_entry_t* head, uint8_t num);
dir_entry_t* get_last_entry(dir_entry_t* head);

#endif