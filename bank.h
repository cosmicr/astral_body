#ifndef BANK_H
#define BANK_H
/*
    Banked memory access routines
*/
#include <cx16.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/*
MEMORY BANKS:

        | *DIR files(32k)   | Words   | Obj|
   +----+----+----+----+----+----+----+----+
00 | K  | LD | PD | VD | SD | W  | W  | O  | 07
   +----+----+----+----+----+----+----+----+
08 | L0 | L0 | L0 | L0 | L0 | L0 | L0 | L0 | 0F Logic 0 (64k)
   +----+----+----+----+----+----+----+----+
10 | L  | L  | L  | L  | L  | L  | L  | L  | 17 Additional Logic (64k)
   +----+----+----+----+----+----+----+----+
18 | P  | P  | P  | P  | P  | P  | P  | P  | 1F Picture(s) (64k)
   +----+----+----+----+----+----+----+----+
20 | V  | V  | V  | V  | V  | V  | V  | V  | 27 View(s) (64k)
   +----+----+----+----+----+----+----+----+
28 | S  | S  | S  | S  | S  | S  | S  | S  | 2F Sound(s) (64k)
   +----+----+----+----+----+----+----+----+
30 | LU |    |    |    |    |    |    |    | 37 Lookup Tables
   +----+----+----+----+----+----+----+----+
38 |    |    |    |    |    |    |    |    | 3F
   +----+----+----+----+----+----+----+----+

K:  Kernel Reserved (8k)
LD: LOGDIR (8k)
PD: PICDIR (8k)
VD: VIEWDIR (8k)
SD: SNDDIR (8k)
W:  Words Data (16k)
O:  Object Data (8k)
L0: Logic 0 (64k)
L:  Additional Logics (64k)
P:  Picture Data (64k)
V:  View Data (64k)
S:  Sound Data (64k)

Total Memory used: 384k
*/

#define LOGDIR_BANK 0x01
#define PICDIR_BANK 0x02
#define VIEWDIR_BANK 0x03
#define SNDDIR_BANK 0x04
#define WORDS_BANK 0x05
#define OBJECT_BANK 0x07
#define LOGIC0_BANK 0x08
#define LOGIC_BANK 0x10
#define PICTURE_BANK 0x18
#define VIEW_BANK 0x20
#define SOUND_BANK 0x28

#define BANK_SIZE     0x2000     // 8KB per bank
#define BANK_MASK     0x1FFF     // Mask for bank size (BANK_SIZE - 1)
#define BANK_START    0xA000     // Starting address of the first bank
    
uint8_t read_bank_data(uint8_t bank, uint16_t offset);
void write_bank_data(uint8_t bank, uint16_t offset, uint8_t data);

#endif // BANK_H