; 65C02 instruction set
.pc02

; Commander X16 definitions
.include "cx16.inc"

; Custom Macros
.include "macros.inc"

; Switch to code segment
.code


; void asm_set_palette(unsigned char color, unsigned char r, unsigned char g, unsigned char b)
.export _asm_set_palette
.proc _asm_set_palette
    pal_color_l = $90
    pal_color_h = $91
    pal_r       = $92
    pal_g       = $93
    pal_b       = $94

    sta pal_b        ; Store the blue color

    pop_c_stack pal_g
    pop_c_stack pal_r
    pop_c_stack pal_color_l
    
    stz pal_color_h

    ; offset = color * 2
    lda pal_color_l
    asl
    sta pal_color_l
    lda pal_color_h
    rol
    sta pal_color_h

    ; address = 0x1FA00 + offset

    ; Set up the VERA address
    lda pal_color_l
    sta VERA::ADDR
    lda #$FA
    adc pal_color_h
    sta VERA::ADDR + 1
    lda #$01
    ora #VERA::INC1
    sta VERA::ADDR + 2

    ; Write the green and blue bits
    lda pal_g
    and #$F0            ; Keep the upper 4 bits for green
    sta pal_g              
    lda pal_b
    lsr
    lsr
    lsr
    lsr
    ora pal_g           ; Set the lower 4 bits to the color
    sta VERA::DATA0

    ; Write the red bits
    lda pal_r
    lsr
    lsr
    lsr
    lsr
    sta VERA::DATA0

    rts                     ; Return from subroutine
.endproc ; _asm_set_palette