#include <cbm.h>
#include <cx16.h>

#include "bank.h"
#include "resource.h"

uint8_t last_logic_num = 0xFF;
uint8_t last_pic_num = 0xFF;

void init_resources(void)
{
    //num_loaded_resources = 0;
    // TODO: Version 3 games
    load_file("logdir", LOGIC);
    load_file("picdir", PICTURE);
    load_file("viewdir", VIEW);
    load_file("snddir", SOUND);

    load_file("words.tok", WORDS);
    load_file("object", OBJECT);
}

void load_file(char* filename, uint8_t bank)
{
    int file_length = 0;

    RAM_BANK = bank;                          // Set the bank
    cbm_k_setlfs(0, 8, 2);                   // Set logical file number, device number, and secondary address
    cbm_k_setnam(filename);                  // Set the file name
    file_length = cbm_k_load(0, BANK_START); // Load the file into memory
    // if load goes past bank boundary, it will wrap around to the start of the next bank

    if (file_length == BANK_START) {
        printf("\nError loading file %s", filename); // todo: I don't think this is right
        exit(0);
    }
    cbm_k_close(0);
}

// todo: Version 3 games (first check if it's version 2, then version 3)
// todo: use buffering to load faster?
// todo: **use a resource struct to keep track of resource offsets into each bank.**
void load_resource(enum kind dir, uint8_t num)
{
    // Logical file number, device number, and secondary address as constants
    static const uint8_t LFN = 1;
    static const uint8_t DEVICE = 8;
    static const uint8_t SA = CBM_READ;

    const uint16_t dir_index = 3 * num; // each entry is 3 bytes long

    char vol_filename[8];

    uint8_t vol_num, bank;
    uint16_t i, header, size;
    uint16_t bank_offset = 0;
    uint32_t offset;

    if (dir == LOGIC && num == last_logic_num) {
        return;
    }
    if (dir == PICTURE && num == last_pic_num) {
        return;
    }

    // get the vol number and offset (todo: Version 3)
    RAM_BANK = dir;
    // get the 24-bit value from memory address 0xA000 + dir_index
    // vol num is first 4 bits
    vol_num = *(uint8_t*)(0xA000 + dir_index) >> 4;
    // offset is last 20 bits
    offset = ((uint32_t)(*(uint8_t*)(BANK_START + dir_index) & 0x0F) << 16) | // top 4 bits
             ((uint32_t)(*(uint8_t*)(BANK_START + dir_index + 1)) << 8) |     // middle 8 bits
             (uint32_t)(*(uint8_t*)(BANK_START + dir_index + 2));             // bottom 8 bits

    sprintf(vol_filename, "vol.%d", vol_num); // set the filename

    // Set file name and logical file parameters
    cbm_k_setnam(vol_filename);
    cbm_k_setlfs(LFN, DEVICE, SA);

    // Open file
    if (cbm_k_open() != 0) {
        printf("Error opening file\n");
        return;
    }

    printf("Loading %s, offset %lu\n", vol_filename, offset);

    cbm_k_chkin(LFN); // set file as input

    // Move to the offset in the vol file
    while (offset--) {
        __asm__("jsr $ffcf"); // cbm_k_chrin();
        if (cbm_k_readst() != 0) {
            printf("Error or end of file reached.\n");
            return;
        }
    }

    // Check the next two bytes are 0x12 and 0x34
    if ((header = (cbm_k_chrin() << 8) | cbm_k_chrin()) != 0x1234) {
        printf("Error: header is not 0x1234 (value was %x)\n", header);
        return;
    }

    // skip resource vol number byte (we already know it)
    cbm_k_chrin();

    // Read the size of the resource
    size = cbm_k_chrin() | (cbm_k_chrin() << 8); // size is little endian

    printf("Loading resource %d from %s, size %u\n", num, vol_filename, size);
    cbm_k_chkin(LFN); // set file as input

    // set the appropriate bank depending on resource
    switch (dir) {
    case LOGIC:
        bank = num ? LOGIC_BANK : LOGIC0_BANK; // logic.0 is in bank 8, logic.? is in bank 16
        last_logic_num = num;
        break;
    case PICTURE:
        bank = PICTURE_BANK;
        last_pic_num = num;
        break;
    case VIEW:
        bank = VIEW_BANK;
        break;
    case SOUND:
        bank = SOUND_BANK;
        break;
    }

    RAM_BANK = bank;

    // Read data and store in memory
    for (i = 0; i < size; ++i) {
        uint8_t byte = cbm_k_chrin();
        if (cbm_k_readst() != 0) {
            printf("Error or end of file reached\n");
            return;
        }
        *(uint8_t*)(BANK_START + bank_offset) = byte;
        bank_offset++;
        if (bank_offset == BANK_SIZE) {
            bank_offset = 0;
            bank++;
            RAM_BANK = bank;
        }
    }

    // Close file
    cbm_k_close(LFN);
}
