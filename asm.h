#ifndef ASM_H
#define ASM_H
/*
    ASM Function Prototypes
*/

#include <stdint.h>

/* Graphics */
extern void asm_init_vera();
extern void asm_clear_screen();
extern void asm_clear_vis_color(unsigned char color);
extern void asm_clear_pri_color(unsigned char color);
extern void asm_clear_overlay();

extern void asm_plot_vis_pixel(unsigned short x, unsigned char y, unsigned char color);
extern void asm_plot_pri_pixel(unsigned short x, unsigned char y, unsigned char color);

extern uint8_t asm_get_vis_pixel(uint8_t x, uint8_t y);
extern uint8_t asm_get_pri_pixel(uint8_t x, uint8_t y);

extern void asm_drawline(unsigned short x1, unsigned char y1, unsigned short x2, unsigned char y2);

extern void asm_plot_vis_hline(unsigned short x0, unsigned short x1, unsigned char y, unsigned char color);
extern void asm_plot_vis_hline_fast(unsigned short x0, unsigned short x1, unsigned char y, unsigned char color);
extern void asm_plot_pri_hline(unsigned short x0, unsigned short x1, unsigned char y, unsigned char color);
extern void asm_plot_pri_hline_fast(unsigned short x0, unsigned short x1, unsigned char y, unsigned char color);

extern void asm_set_palette(unsigned char color, unsigned char r, unsigned char g, unsigned char b);

extern uint8_t asm_can_fill(uint8_t x, uint8_t y);
extern void asm_flood_fill(uint8_t x, uint8_t y);

/* Timing */
extern void asm_wait_for_refresh();

#endif // ASM_H