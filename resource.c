#include <cbm.h>
#include <cx16.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include "resource.h"

uint32_t last_offset = 0;

uint32_t logdir;
uint32_t picdir;
uint32_t viewdir;
uint32_t snddir;

uint32_t words_tok;
uint32_t object;

dir_entry_t* logdir_head;
dir_entry_t* picdir_head;
dir_entry_t* viewdir_head;
dir_entry_t* snddir_head;

/* MEMORY ROUTINES */

volatile uint8_t read_heap(uint32_t offset)
{
    RAM_BANK = 1 + (offset >> 13);
    return *((volatile uint8_t*)(BANK_RAM + (offset & 0x1FFF)));
}

void write_heap(uint32_t offset, uint8_t data)
{
    RAM_BANK = 1 + (offset >> 13);
    *((volatile uint8_t*)(BANK_RAM + (offset & 0x1FFF))) = data;
}

/* FILE ROUTINES */

uint32_t load_file(char* filename)
{
    uint8_t start_bank = RAM_BANK;
    uint16_t file_length = 0;
    uint32_t offset = BANK_RAM + (last_offset & 0x1FFF);

    RAM_BANK = 1 + (last_offset >> 13);

    cbm_k_setlfs(0, 8, 2);                   // Set logical file number, device number, and secondary address
    cbm_k_setnam(filename);                  
    file_length = cbm_k_load(0, offset);
    
    file_length = ((RAM_BANK-1) * 8192) + (file_length - 0xA000) - last_offset;

    // if load goes past bank boundary, it will wrap around to the start of the next bank

    last_offset += file_length;

    cbm_k_close(0);

    return last_offset - file_length;
}

/* RESOURCE LOADING */

void init_resources(void)
{
    logdir = load_file("logdir");
    picdir = load_file("picdir");
    viewdir = load_file("viewdir");
    snddir = load_file("snddir");

    words_tok = load_file("words.tok");
    object = load_file("object");
}

void load_resource(uint32_t directory, uint8_t num)
{
    const uint16_t index = 3 * num;
    register uint8_t vol_num;
    uint16_t i, resource_size;
    uint32_t file_offset;
    char vol_filename[8];
    dir_entry_t *entry;

    vol_num = read_heap(directory + index) >> 4;

    file_offset = (uint32_t)(read_heap(directory + index) & 0x0F) << 16;
    file_offset |= (uint32_t)read_heap(directory + index + 1) << 8;
    file_offset |= (uint32_t)read_heap(directory + index + 2);

    //printf("Resource %d from vol.%d, offset %lx\n", num, vol_num, file_offset);

    sprintf(vol_filename, "vol.%d", vol_num);

    cbm_k_setlfs(1, 8, 2); // LFN 1, device 8, secondary address 2
    cbm_k_setnam(vol_filename);
    cbm_k_open();
    cbm_k_chkin(1); // this LFN

    // Move to the offset in the vol file
    while (file_offset--) {
        cbm_k_chrin();
    }

    // Check the next two bytes are 0x12 and 0x34
    if ((cbm_k_chrin() << 8 | cbm_k_chrin()) != 0x1234) {
        printf("Error: 0x1234\n");
        free(entry);
        exit(1);
    }

    // skip resource vol number byte (we already know it)
    cbm_k_chrin();

    // Read the size of the resource
    resource_size = cbm_k_chrin() | (cbm_k_chrin() << 8); // size is little endian

    // Create an entry in the linked list for this resource
    entry = add_to_dirlist(directory == logdir ? &logdir_head : directory == picdir ? &picdir_head : directory == viewdir ? &viewdir_head : &snddir_head, last_offset);
    entry->res_num = num;

    // printf("loading into memory at %lX, size %u\n", last_offset, resource_size);
    // cbm_k_chkin(1);

    // Read data and store in memory
    for (i = 0; i < resource_size; ++i) {
        write_heap(last_offset++, cbm_k_chrin());
    }

    // TODO: if logic file, decrypt messages

    // Close file
    cbm_k_close(1);
}

void unload_resource(uint32_t directory, uint8_t num)
{
    dir_entry_t* current = directory;
    dir_entry_t* previous = NULL;

    while (current != NULL) {
        if (current->res_num == num) {
            if (previous == NULL) {
                directory = current->next;
            } else {
                previous->next = current->next;
            }
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

/* LINKED LISTS */

dir_entry_t* add_to_dirlist(dir_entry_t** head, uint32_t offset)
{
    dir_entry_t* new_entry = malloc(sizeof(dir_entry_t));
    new_entry->offset = offset;
    new_entry->next = NULL;

    if (*head == NULL) {
        *head = new_entry;
    } else {
        dir_entry_t* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_entry;
    }

    return new_entry;
}

uint32_t get_offset(dir_entry_t* head, uint8_t num)
{
    dir_entry_t* current = head;
    while (current != NULL) {
        if (current->res_num == num) {
            return current->offset;
        }
        current = current->next;
    }
    return 0;
}

dir_entry_t* get_last_entry(dir_entry_t* head)
{
    dir_entry_t* current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    return current;
}