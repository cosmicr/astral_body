; 65C02 instruction set
.pc02

; Commander X16 definitions
.include "cx16.inc"

; Custom Macros
.include "macros.inc"

; Switch to code segment
.code

.import pusha, popa, pushax, popax, decsp2, incsp2
.import _vis_colour, _pri_colour, _vis_enabled, _pri_enabled, _fill_stack_pointer
.import _scan_and_fill, _push, _pop, _asm_get_vis_pixel, _asm_get_pri_pixel
.import _asm_plot_vis_pixel, _asm_plot_pri_pixel

.macro can_fill x_val, y_val
.scope
    ; registers X and Y contain pixel coordinates
    ; returns 0 in A register if the pixel cannot be filled (early exit)
    ; returns 1 in A register if the pixel can be filled

    STATUSBAR_OFFSET = 8
    NEW_Y = $90
    VIS_PIXEL = $91
    PRI_PIXEL = $92

    ; add STATUSBAR_OFFSET to y_val
    lda y_val
    clc
    adc #STATUSBAR_OFFSET
    sta NEW_Y ; y_val + STATUSBAR_OFFSET

    ; get the vis pixel at the current x and y
    calc_vram_addr_160 x_val, NEW_Y
    lda VERA::DATA0
    and #$0F ; mask out the top 4 bits
    sta VIS_PIXEL

    ; get the pri pixel at the current x and y
    ; Add 0x9600 to the VERA::ADDR
    lda VERA::ADDR + 1
    clc
    adc #$96
    sta VERA::ADDR + 1
    lda VERA::ADDR + 2
    adc #$00
    sta VERA::ADDR + 2
    lda VERA::DATA0
    and #$0F ; mask out the top 4 bits
    sta PRI_PIXEL

    ; is the current vis colour 15 (white)?
    ; if (vis_colour == 15) return 0;
    lda _vis_colour
    cmp #15
    beq @cannot_fill

    ; is priority disabled and the current vis pixel not white?
    ; if (!pri_enabled && (asm_get_vis_pixel(x, y) != 15)) return 0;
    lda _pri_enabled
    bne @pri_enabled_check ; if priority is enabled, skip this check
    lda VIS_PIXEL
    cmp #15
    bne @cannot_fill

@pri_enabled_check:
    ; is priority enabled and vis disabled and the current pri pixel not red?
    ; if (pri_enabled && !vis_enabled && (asm_get_pri_pixel(x, y) != 4)) return 0;
    lda _vis_enabled
    bne @vis_enabled_check
    lda PRI_PIXEL
    cmp #4
    bne @cannot_fill

@vis_enabled_check:
    ; is priority enabled and the current vis pixel not white?
    ; if (pri_enabled && (asm_get_vis_pixel(x, y) != 15)) return 0;
    lda _pri_enabled
    beq @can_fill
    lda VIS_PIXEL
    cmp #15
    bne @cannot_fill

@can_fill:
    lda #1 ; return 1 (pixel can be filled)
    ldx #0 ; clear X register
    bra @end_macro

@cannot_fill:
    lda #0 ; return 0 (pixel cannot be filled)

@end_macro:
.endscope ; can_fill
.endmacro

.export _asm_flood_fill
.proc _asm_flood_fill
    LX      = $D0
    RX      = $D1
    Y1      = $D2
    NX      = $D3
    DY      = $D4
    X_VAL   = $D5
    Y_VAL   = $D6

    sta Y_VAL ; y is in A register
    jsr popa
    sta X_VAL 

    ; can_fill X_VAL, Y_VAL
    ; bne @ok_fill
    ; rts
@ok_fill:

    ; fill_stack_pointer = 0;
    stz _fill_stack_pointer

    ; scan_and_fill(x, y);
    lda X_VAL
    jsr pusha
    lda Y_VAL
    jsr _scan_and_fill

    ; while (pop(&lx, &rx, &y1)) {
pop_loop:
    ldx #$00
    lda #LX
    jsr pushax
    lda #RX
    jsr pushax
    lda #Y1
    jsr _pop
    bne @pop_not_done ; bne branches if Z flag is clear (ie not equal to zero)
    jmp pop_done
@pop_not_done:
    ; nx = lx;
    lda LX
    sta NX
    ; while (nx <= rx) {
outer_loop_start:
    lda RX
    ;This instruction subtracts the contents of memory from the contents of the accumulator.
    ;The use of the CMP affects the following flags: 
    ; Z flag is set on an equal comparison, reset otherwise (ie M==A Z=1)
    ; the N flag is set or reset by the result bit 7, (ie M-A<0 N=1)
    ; the carry flag is set when the value in memory is less than or equal to the accumulator, (ie M<=A C=1)
    cmp NX ; compare NX to RX
    bcs @nx_less_than_rx ; bcc branches if C flag is set (ie NX <= RX)
    jmp outer_loop_end
@nx_less_than_rx:
    ; if (can_fill(nx, y1)) {
    can_fill NX, Y1
    cmp #0
    bne @start_fill ; branch if can_fill returned true
    jmp else_increment_nx
    @start_fill:
    ; scan_and_fill(nx, y1);
    lda NX
    jsr pusha
    lda Y1
    jsr _scan_and_fill
    ; while (nx <= rx && can_fill(nx, y1)) {
inner_loop_start:
    lda NX
    cmp RX
    ; bcs outer_loop_start ; bcs branches if C flag is set (ie NX > RX)
    bcc @nx_less_than_rx_inner
    jmp else_increment_nx
@nx_less_than_rx_inner:
    can_fill NX, Y1
    bne @can_fill_inner
    jmp outer_loop_start
@can_fill_inner:
    ; ++nx;
    inc NX
    jmp inner_loop_start
else_increment_nx:
    ; ++nx;
    inc NX
    jmp outer_loop_start
outer_loop_end:
    jmp pop_loop
pop_done:
    rts
.endproc ; _asm_flood_fill

;-------------------------------------------------------------------------------

.export _asm_can_fill
.proc _asm_can_fill
    X_VAL = $80
    Y_VAL = $81

    sta Y_VAL
    jsr popa
    sta X_VAL

    can_fill X_VAL, Y_VAL

    rts ; return
.endproc