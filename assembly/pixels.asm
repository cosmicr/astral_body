; 65C02 instruction set
.pc02

; Commander X16 definitions
.include "cx16.inc"

; Custom Macros
.include "macros.inc"

color_table:
    .byte $00, $11, $22, $33, $44, $55, $66, $77, $88, $99, $AA, $BB, $CC, $DD, $EE, $FF

; Switch to code segment
.code

.import popa

; plot_vis_pixel(unsigned short x, unsigned char y, unsigned char color);
.export _asm_plot_vis_pixel
.proc _asm_plot_vis_pixel
    ; Define temporary storage locations and labels ($80 is used by calc_vram_addr)
    X_LOW           = $B0
    X_HIGH          = $B1
    Y_VAL           = $B2
    COLOR           = $B3

    sta COLOR       ; Store color from A register

    ; Get parameters from the C stack
    jsr popa
    sta Y_VAL
    jsr popa
    sta X_LOW
    jsr popa
    sta X_HIGH

    ; Calculate address using macro
    calc_vram_addr X_LOW, X_HIGH, Y_VAL

    ; duplicate the bottom 4 bits of color to the upper 4 bits
    ldy COLOR
    lda color_table,y

    sta VERA::DATA0         ; Write the color to VRAM

    rts                     ; Return from subroutine
.endproc ; _asm_plot_vis_pixel

; plot_pri_pixel(unsigned short x, unsigned char y, unsigned char color);
.export _asm_plot_pri_pixel
.proc _asm_plot_pri_pixel
    ; Define temporary storage locations and labels ($80 is used by calc_vram_addr)
    X_LOW           = $B0
    X_HIGH          = $B1
    Y_VAL           = $B2
    COLOR           = $B3

    sta COLOR       ; Store color from A register

    ; Get parameters from the C stack
    jsr popa
    sta Y_VAL
    jsr popa
    sta X_LOW
    jsr popa
    sta X_HIGH

    ; Calculate address using macro
    calc_vram_addr X_LOW, X_HIGH, Y_VAL

    ; TODO: use separate macro instead?
    ; Add 0x9800 to the VERA::ADDR
    lda VERA::ADDR + 1
    clc
    adc #$96
    sta VERA::ADDR + 1
    lda VERA::ADDR + 2
    adc #$00
    and #%00000001            ; Throw away any other bits
    sta VERA::ADDR + 2

    ; duplicate the bottom 4 bits of color to the upper 4 bits
    ldy COLOR
    lda color_table,y

    sta VERA::DATA0         ; Write the color to VRAM

    rts                     ; Return from subroutine
.endproc ; _asm_plot_pri_pixel

; asm_get_vis_pixel(x, y)
; Returns color at x, y from the visual screen in A register
.export _asm_get_vis_pixel
.proc _asm_get_vis_pixel
    STATUSBAR_OFFSET = 8   ; TODO: get this from the C code?
    ; Define temporary storage locations and labels
    X_VAL           = $B0
    Y_VAL           = $B1
    ; Add STATUSBAR_OFFSET to y (Y_VAL is in A register)
    clc
    adc #STATUSBAR_OFFSET
    sta Y_VAL

    ; Get parameters from the C stack
    jsr popa
    sta X_VAL

    ; Calculate address for 160x200 mode
    calc_vram_addr_160 X_VAL, Y_VAL

    ; return the pixel (16 bits A and X)
    lda VERA::DATA0
    and #$0F    ; mask off the top 4 bits (no need to ldx because it's thrown away)
    rts

.endproc ; _asm_get_vis_pixel

; asm_get_pri_pixel(x, y)
; Returns color at x, y from the priority screen in A register
.export _asm_get_pri_pixel
.proc _asm_get_pri_pixel
    STATUSBAR_OFFSET = 8   ; TODO: get this from the C code?
    ; Define temporary storage locations and labels
    X_VAL           = $B0
    Y_VAL           = $B1
    ; Add STATUSBAR_OFFSET to y (Y_VAL is in A register)
    clc
    adc #STATUSBAR_OFFSET
    sta Y_VAL

    ; Get parameters from the C stack
    jsr popa
    sta X_VAL

    ; Calculate address for 160x200 mode
    calc_vram_addr_160 X_VAL, Y_VAL

    ; Add 0x9600 to the VERA::ADDR
    lda VERA::ADDR + 1
    clc
    adc #$96
    sta VERA::ADDR + 1
    lda VERA::ADDR + 2
    adc #$00
    sta VERA::ADDR + 2

    ; return the pixel (16 bits A and X)
    lda VERA::DATA0
    and #$0F    ; mask off the top 4 bits
    rts

.endproc ; _asm_get_pri_pixel

; /*
;     Draws an AGI line using Bresenham's algorithm.
; */
; void drawline(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2)
; {
;     int16_t dx, dy, sx, sy, err, e2;

;     // Determine the direction to step in
;     dx = abs((int16_t)x2 - (int16_t)x1);
;     dy = -abs((int16_t)y2 - (int16_t)y1);
;     sx = x1 < x2 ? 1 : -1;
;     sy = y1 < y2 ? 1 : -1;
;     err = dx + dy; // error value

;     while (1) {
;         // inlined pset to avoid function call overhead
;         if (vis_enabled)
;             asm_plot_vis_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, vis_colour);
;         if (pri_enabled)
;             asm_plot_pri_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, pri_colour);
;         if (x1 == x2 && y1 == y2)
;             break;     // Check end condition
;         e2 = err << 1; // e2 = 2 * err
;         if (e2 >= dy) {
;             err += dy;
;             x1 += sx;
;         }
;         if (e2 <= dx) {
;             err += dx;
;             y1 += sy;
;         }
;     }
; }
.import _vis_colour, _pri_colour, _vis_enabled, _pri_enabled
.export _asm_drawline
.proc _asm_drawline
    STATUSBAR_OFFSET = 8   ; TODO: get this from the C code?

    ; Define temporary storage locations and labels
    ZP_BASE         = $C0
    Y1_VAL          = ZP_BASE
    Y2_VAL          = ZP_BASE + 1
    DX_LOW          = ZP_BASE + 2
    DX_HIGH         = ZP_BASE + 3
    DY_LOW          = ZP_BASE + 4
    DY_HIGH         = ZP_BASE + 5
    SX_LOW          = ZP_BASE + 6
    SX_HIGH         = ZP_BASE + 7
    SY_LOW          = ZP_BASE + 8
    SY_HIGH         = ZP_BASE + 9
    ERR_LOW         = ZP_BASE + 10
    ERR_HIGH        = ZP_BASE + 11
    E2_LOW          = ZP_BASE + 12
    E2_HIGH         = ZP_BASE + 13
    X_LOW_TEMP      = ZP_BASE + 14
    X_HIGH_TEMP     = ZP_BASE + 15

    X1_LOW          = ZP_BASE + 16
    X1_HIGH         = ZP_BASE + 17
    X2_LOW          = ZP_BASE + 18
    X2_HIGH         = ZP_BASE + 19
    Y_VAL_TEMP      = ZP_BASE + 20

start:
    sta Y2_VAL     ; Store y2 from A register

    ; Get parameters from the C stack
    jsr popa
    sta X2_LOW
    jsr popa
    sta X2_HIGH
    jsr popa
    sta Y1_VAL
    jsr popa
    sta X1_LOW
    jsr popa
    sta X1_HIGH

    ; dx = abs(x2 - x1);
    lda X2_LOW
    sec
    sbc X1_LOW
    sta DX_LOW
    lda X2_HIGH
    sbc X1_HIGH
    sta DX_HIGH
    bpl @dx_positive
    ; If it's negative, negate it
    lda #$FF
    sec
    sbc DX_LOW            ; A = 0xFF - (-1) = 0x100
    sta DX_LOW            ; DX_LOW = 0
    lda #$FF
    sbc DX_HIGH           ; A = 0xFF - 0 - 1 (borrow) = 0xFE
    sta DX_HIGH           ; DX_HIGH = 0xFE
    inc DX_LOW            ; DX_LOW = 1, DX_HIGH:DX_LOW = 0x00:0x01
@dx_positive:

    ; dy = -abs(y2 - y1);
    lda Y2_VAL
    sec
    sbc Y1_VAL
    sta DY_LOW
    lda #0
    sbc #0
    sta DY_HIGH
    ; If DY is positive, negate it
    bpl @negate_dy
    jmp @dy_done
@negate_dy:
    lda #0
    sec
    sbc DY_LOW
    sta DY_LOW
    lda #0
    sbc DY_HIGH
    sta DY_HIGH
@dy_done:

    ; sx = x1 < x2 ? 1 : -1;
    lda X1_LOW
    cmp X2_LOW
    lda X1_HIGH
    sbc X2_HIGH
    bvc @x_sign_check
    eor #$80
@x_sign_check:
    bpl @x1_greater_than_equal_x2
    lda #1
    sta SX_LOW
    lda #0
    sta SX_HIGH
    bra @sx_done
@x1_greater_than_equal_x2:
    lda #$FF
    sta SX_LOW
    sta SX_HIGH
@sx_done:

    ; sy = y1 < y2 ? 1 : -1;
    lda Y1_VAL
    cmp Y2_VAL
    bpl @y1_greater_than_equal_y2
    lda #1
    sta SY_LOW
    lda #0
    sta SY_HIGH
    bra @sy_done
@y1_greater_than_equal_y2:
    lda #$FF
    sta SY_LOW
    sta SY_HIGH
@sy_done:

    ; err = dx + dy;
    lda DX_LOW
    clc
    adc DY_LOW
    sta ERR_LOW
    lda DX_HIGH
    adc DY_HIGH
    sta ERR_HIGH

loop_start:
    ; if (vis_enabled)
    ;     asm_plot_vis_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, vis_colour);
    lda _vis_enabled
    bne @plot_vis  ; Invert the condition
    jmp skip_vis  ; Jump to @skip_vis if _vis_enabled is zero

@plot_vis:
    lda X1_LOW
    asl
    sta X_LOW_TEMP
    lda X1_HIGH
    rol
    sta X_HIGH_TEMP
    lda Y1_VAL
    clc
    adc #STATUSBAR_OFFSET
    sta Y_VAL_TEMP
    calc_vram_addr X_LOW_TEMP, X_HIGH_TEMP, Y_VAL_TEMP
    ldy _vis_colour
    lda color_table,y
    sta VERA::DATA0         ; Write the color to VRAM

skip_vis:

    ; if (pri_enabled)
    ;     asm_plot_pri_pixel((x1 << 1), y1 + STATUSBAR_OFFSET, pri_colour);
    lda _pri_enabled
    bne @plot_pri  ; Invert the condition
    jmp skip_pri  ; Jump to @skip_pri if _pri_enabled is zero

@plot_pri:
    lda X1_LOW
    asl
    sta X_LOW_TEMP
    lda X1_HIGH
    rol
    sta X_HIGH_TEMP
    lda Y1_VAL
    clc
    adc #STATUSBAR_OFFSET
    sta Y_VAL_TEMP
    calc_vram_addr X_LOW_TEMP, X_HIGH_TEMP, Y_VAL_TEMP
    ; Add 0x9800 to the VERA::ADDR
    lda VERA::ADDR + 1
    clc
    adc #$96
    sta VERA::ADDR + 1
    lda VERA::ADDR + 2
    adc #$00
    and #%00000001            ; Throw away any other bits
    sta VERA::ADDR + 2
    ldy _pri_colour
    lda color_table,y
    sta VERA::DATA0         ; Write the color to VRAM

skip_pri:

    ; if (x1 == x2 && y1 == y2)
    ;     break;     // Check end condition
    lda X1_LOW
    cmp X2_LOW
    bne @continue ; if not equal, continue
    lda X1_HIGH
    cmp X2_HIGH
    bne @continue ; if not equal, continue
    lda Y1_VAL
    cmp Y2_VAL
    bne @continue ; if not equal, continue
    jmp @end_loop ; if all are equal, end loop
@continue:

    ; e2 = 2 * err;
    lda ERR_LOW
    asl
    sta E2_LOW
    lda ERR_HIGH
    rol
    sta E2_HIGH

    ; if (e2 >= dy) {
    ;     err += dy;
    ;     x1 += sx;
    ; }
    ; check if e2 >= dy
    lda E2_HIGH
    cmp DY_HIGH
    bmi @e2_less_than_dy ; do nothing if e2 < dy
    bne @e2_greater_than_dy ; branch if e2 != dy
    lda E2_LOW
    cmp DY_LOW
    bcc @e2_less_than_dy ; it must be greater or equal
@e2_greater_than_dy:
    ; err += dy;
    lda ERR_LOW
    clc
    adc DY_LOW
    sta ERR_LOW
    lda ERR_HIGH
    adc DY_HIGH
    sta ERR_HIGH
    ; x1 += sx;
    lda X1_LOW
    clc
    adc SX_LOW
    sta X1_LOW
    lda X1_HIGH
    adc SX_HIGH
    sta X1_HIGH
@e2_less_than_dy:

    ; if (e2 <= dx) {
    ;     err += dx;
    ;     y1 += sy;
    ; }
    lda E2_HIGH
    cmp DX_HIGH
    bmi @e2_less_than_dx
    bne @e2_greater_than_dx
    lda E2_LOW
    cmp DX_LOW
    bcc @e2_less_than_dx
    beq @e2_less_than_dx
@e2_greater_than_dx:
    jmp loop_start
@e2_less_than_dx:
    lda ERR_LOW
    clc
    adc DX_LOW
    sta ERR_LOW
    lda ERR_HIGH
    adc DX_HIGH
    sta ERR_HIGH
    ; y1 += sy;
    lda Y1_VAL
    clc
    adc SY_LOW
    sta Y1_VAL

    jmp loop_start

@end_loop:
    rts
.endproc ; _asm_drawline