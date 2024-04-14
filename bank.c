#include "bank.h"

/*
    Read a byte from a banked memory address
*/
uint8_t read_bank_data(uint8_t bank, uint16_t offset) {
    //volatile uint16_t new_offset = offset & BANK_MASK; // equivalent to offset % 8192

    // set memory address 0x00 to the final bank
    RAM_BANK = bank + (offset >> 13); // equivalent to bank + offset / 8192

    // Calculate the address within the bank and return the data
    return *((volatile uint8_t*)(BANK_START + (offset & BANK_MASK)));
}

/*
    Write a byte to a banked memory address
*/
void write_bank_data(uint8_t bank, uint16_t offset, uint8_t data) {
    //volatile uint16_t new_offset = offset & BANK_MASK; // equivalent to offset % 8192

    // set memory address 0x00 to the final bank
    RAM_BANK = bank + (offset >> 13); // equivalent to bank + offset / 8192

    // Calculate the address within the bank and write the data
    *((volatile uint8_t*)(BANK_START + (offset & BANK_MASK))) = data;
}