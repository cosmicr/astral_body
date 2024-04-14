; 65C02 instruction set
.pc02

; Commander X16 definitions
.include "cx16.inc"

; Custom Macros
.include "macros.inc"

.segment "RODATA"
;78563412 - each "2-bit byte" is little endian so, %00000001 is actually %00000010

mask_table:
    .byte %11111111 ; 0 $FF
    .byte %11111111 ; 1 $FD
    .byte %11111100 ; 2 $FC
    .byte %11111100 ; 3 $F4
    .byte %11110000 ; 4 $F0
    .byte %11110000 ; 5 $D0
    .byte %11000000 ; 6 $C0
    .byte %11000000 ; 7 $40

color_table:
    .byte $00, $11, $22, $33, $44, $55, $66, $77, $88, $99, $AA, $BB, $CC, $DD, $EE, $FF

; Switch to code segment
.code

; asm_plot_vis_hline_fast(unsigned short x0, unsigned short x1, unsigned char y, unsigned char color);
.export _asm_plot_vis_hline_fast
.proc _asm_plot_vis_hline_fast
    temp            = $A0
    color           = $A1
    start_mask      = $A2
    end_mask        = $A3
    length_low      = $A4
    length_high     = $A5
    Y0              = $A6
    X1_LOW          = $A7
    X1_HIGH         = $A8
    X0_LOW          = $A9
    X0_HIGH         = $AA
    
    sei ; Disable interrupts

    ; color is in A register
    sta color

    pop_c_stack Y0
    pop_c_stack X1_LOW
    pop_c_stack X1_HIGH
    pop_c_stack X0_LOW
    pop_c_stack X0_HIGH
    
    ; Calculate the line length and the loop count
    ; Ensure X1 >= X0 
    lda X1_LOW
    sec
    sbc X0_LOW
    sta length_low
    lda X1_HIGH
    sbc X0_HIGH
    sta length_high

    ; Is length less than or equal to 16?
    bne long_line ; length_high is in A
    lda length_low
    clc
    cmp #$10
    bcs long_line

    ; Line is a short line
    lda #%00000100  ; DCSEL = Mode 2 
    sta VERA::CTRL
    stz FX_CTRL ; Disable cache writing
    ; subtract 5 bytes from c_stack_addr
    lda C_STACK_ADDR
    sec
    sbc #5
    sta C_STACK_ADDR
    lda C_STACK_ADDR + 1
    sbc #0
    sta C_STACK_ADDR + 1
    lda color
    jsr _asm_plot_vis_hline
    rts ; Return from subroutine

long_line:
    ; Change DCSEL to mode 6 for cache write operations
    lda #%00001100
    sta VERA::CTRL

    ldx color
    lda color_table, x
    sta $9f29
    sta $9f2A
    sta $9f2B
    sta $9f2C

    ; Set up VERA for cache operations
    lda #%00000100  ; DCSEL = Mode 2 for enabling cache
    sta VERA::CTRL
    lda #%01000000  ; Enable cache writing
    sta FX_CTRL

    ; Calculate the mask for ending chunk
    lda X1_LOW
    and #$7
    tax
    lda mask_table, x
    sta end_mask

    ; Calculate the mask for starting chunk
    lda X0_LOW
    and #$7
    tax
    lda mask_table, x
    eor #$FF ; Invert the mask
    sta start_mask  

    ; Add pixels from start mask to length
    txa ; start length from above
    clc
    adc length_low
    sta length_low
    lda length_high
    adc #0
    sta length_high

    ; Calculate the number of full 8-pixel (32-bit) chunks by dividing by 8 (shift right 3 times)
    lsr length_high   ; Shift right, dividing the high byte by 2
    ror length_low    ; Rotate right the low byte through carry
    lsr length_high   ; Shift right again, further dividing the high byte
    ror length_low    ; Rotate right again
    lsr length_high   ; Final shift right
    ror length_low    ; Final rotate right

    ; Subtract 1 from length
    dec length_low ; length is never higher than 40

    ; *** call the vram address calculation routine ***
    calc_vram_addr X0_LOW, X0_HIGH, Y0

    ; Set address auto-increment to 4 bytes
    lda #%00110000
    sta VERA::ADDR + 2

    ; draw the starting chunk
    lda start_mask
    sta VERA::DATA0

    ; Loop counter
    lda #0 ; clear the mask
    ldx length_low

@loop:
    ; Plotting action
    sta VERA::DATA0
    dex
    bne @loop 

done_plotting:
    ; Handle the last partial chunk 
    lda end_mask
    sta VERA::DATA0

    lda #%00000100  ; DCSEL = Mode 2 for enabling cache
    sta VERA::CTRL
    stz FX_CTRL ; Disable cache writing

    cli ; Enable interrupts
    rts                     ; Return from subroutine
.endproc ; _plot_vis_hline_fast

; asm_plot_vis_hline(unsigned short x0, unsigned short x1, unsigned char y, unsigned char color);
; plots 2 pixels at a time for 160x200 mode
.export _asm_plot_vis_hline
.proc _asm_plot_vis_hline
    color           = $A0
    Y0              = $A1
    X1_LOW          = $A2
    X1_HIGH         = $A3
    X0_LOW          = $A4
    X0_HIGH         = $A5
    
    sei ; Disable interrupts

    ; color is in A register
    sta color

    pop_c_stack Y0
    pop_c_stack X1_LOW
    pop_c_stack X1_HIGH
    pop_c_stack X0_LOW
    pop_c_stack X0_HIGH

    ; *** call the vram address calculation routine ***
    calc_vram_addr X0_LOW, X0_HIGH, Y0
    
    lda #VERA::INC1      ; Enable auto-increment
    sta VERA::ADDR + 2

    ; Calculate the line length and the loop count / 2
    lda X1_LOW
    sec
    sbc X0_LOW
    tax
    lda X1_HIGH
    sbc X0_HIGH
    lsr             ; shift right but throw away result
    txa
    ror             ; rotate into low byte
    tax

    ldy color
    lda color_table, y
    @loop:
        ; Plotting action
        sta VERA::DATA0
        dex
        bne @loop

    cli ; Enable interrupts
    rts                     ; Return from subroutine
.endproc ; _plot_vis_hline

; asm_plot_pri_hline_fast(unsigned short x0, unsigned short x1, unsigned char y, unsigned char color);
.export _asm_plot_pri_hline_fast
.proc _asm_plot_pri_hline_fast
    temp            = $A0
    color           = $A1
    start_mask      = $A2
    end_mask        = $A3
    length_low      = $A4
    length_high     = $A5
    Y0              = $A6
    X1_LOW          = $A7
    X1_HIGH         = $A8
    X0_LOW          = $A9
    X0_HIGH         = $AA

    sei ; Disable interrupts

    ; color is in A register
    sta color

    ; Change DCSEL to mode 6 for cache write operations
    lda #%00001100
    sta VERA::CTRL

    ldx color
    lda color_table, x
    sta $9f29
    sta $9f2A
    sta $9f2B
    sta $9f2C

    pop_c_stack Y0
    pop_c_stack X1_LOW
    pop_c_stack X1_HIGH
    pop_c_stack X0_LOW
    pop_c_stack X0_HIGH
    
    ; Calculate the line length and the loop count
    ; Ensure X1 >= X0 
    lda X1_LOW
    sec
    sbc X0_LOW
    sta length_low
    lda X1_HIGH
    sbc X0_HIGH
    sta length_high

    ; Is length less than or equal to 16?
    bne long_line ; length_high is in A
    lda length_low
    clc
    cmp #$10
    bcs long_line

    ; Line is a short line
    lda #%00000100  ; DCSEL = Mode 2 
    sta VERA::CTRL
    stz FX_CTRL ; Disable cache writing
    ; subtract 5 bytes from c_stack_addr
    lda C_STACK_ADDR
    sec
    sbc #5
    sta C_STACK_ADDR
    lda C_STACK_ADDR + 1
    sbc #0
    sta C_STACK_ADDR + 1
    lda color
    jsr _asm_plot_pri_hline
    rts ; Return from subroutine

long_line:
    ; Set up VERA for cache operations
    lda #%00000100  ; DCSEL = Mode 2 for enabling cache
    sta VERA::CTRL
    lda #%01000000  ; Enable cache writing
    sta FX_CTRL

    ; Calculate the mask for ending chunk
    lda X1_LOW
    and #$7
    tax
    lda mask_table, x
    sta end_mask

    ; Calculate the mask for starting chunk
    lda X0_LOW
    and #$7
    tax
    lda mask_table, x
    eor #$FF ; Invert the mask
    sta start_mask  

    ; Add pixels from start mask to length
    txa ; start mask from above
    clc
    adc length_low
    sta length_low
    lda length_high
    adc #0
    sta length_high

    ; Calculate the number of full 8-pixel (32-bit) chunks by dividing by 8 (shift right 3 times)
    lsr length_high   ; Shift right, dividing the high byte by 2
    ror length_low    ; Rotate right the low byte through carry
    lsr length_high   ; Shift right again, further dividing the high byte
    ror length_low    ; Rotate right again
    lsr length_high   ; Final shift right
    ror length_low    ; Final rotate right

    ; Subtract 1 from length
    dec length_low ; length is never higher than 40

    ; *** call the vram address calculation routine ***
    calc_vram_addr X0_LOW, X0_HIGH, Y0

    ; Add 0x8000 to the VERA::ADDR ; TODO: use one hline function that takes a parameter for the offset
    lda VERA::ADDR + 1
    clc
    adc #$80
    sta VERA::ADDR + 1
    lda VERA::ADDR + 2
    adc #$00
    and #%00000001            ; Throw away any other bits

    ; Set address auto-increment to 4 bytes
    ora #%00110000
    sta VERA::ADDR + 2

    ; draw the starting chunk
    lda start_mask
    sta VERA::DATA0

    ; Loop counter
    ldx length_low
   
    lda #0 ; clear the mask
    loop:
        ; Plotting action
        sta VERA::DATA0
        dex
        bne loop ; continue if x is not zero

    ; Handle the last partial chunk 
    lda end_mask
    sta VERA::DATA0

    lda #%00000100  ; DCSEL = Mode 2 for enabling cache
    sta VERA::CTRL
    stz FX_CTRL ; Disable cache writing
    
    cli ; Enable interrupts
    rts                     ; Return from subroutine
.endproc ; _plot_pri_hline_fast

; asm_plot_pri_hline(unsigned short x0, unsigned short x1, unsigned char y, unsigned char color);
.export _asm_plot_pri_hline
.proc _asm_plot_pri_hline
    color           = $A0
    Y0              = $A1
    X1_LOW          = $A2
    X1_HIGH         = $A3
    X0_LOW          = $A4
    X0_HIGH         = $A5

    sei ; Disable interrupts

    ; color is in A register
    sta color

    pop_c_stack Y0
    pop_c_stack X1_LOW
    pop_c_stack X1_HIGH
    pop_c_stack X0_LOW
    pop_c_stack X0_HIGH

    ; *** call the vram address calculation routine ***
    calc_vram_addr X0_LOW, X0_HIGH, Y0

    ; Add 0x8000 to the VERA::ADDR 
    lda VERA::ADDR + 1
    clc
    adc #$80
    sta VERA::ADDR + 1
    lda VERA::ADDR + 2
    adc #$00
    ora #VERA::INC1      ; Enable auto-increment
    sta VERA::ADDR + 2

    ; Calculate the line length and the loop count / 2
    lda X1_LOW
    sec
    sbc X0_LOW
    tax
    lda X1_HIGH
    sbc X0_HIGH
    lsr             ; shift right but throw away result
    txa
    ror             ; rotate into low byte
    tax

    ldy color
    lda color_table, y
    @loop:
        ; Plotting action
        sta VERA::DATA0
        dex
        bne @loop

    cli ; Enable interrupts

    rts                     ; Return from subroutine
.endproc ; _asm_plot_pri_hline

