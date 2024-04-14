; Zero page registers $80 - $8F are reserved for macros

C_STACK_ADDR = $22

; note: screen size is actually 160x200 ($7D00) bytes but we put some extra padding at the end
SCREEN_SIZE = $7D

; VERA register addresses
FX_CTRL = $9F29

; Line look up table address
LINE_LOOKUP_TABLE = $400

.macro push_registers
    pha
    txa
    pha
    tya
    pha
.endmacro

.macro pop_registers
    pla
    tay
    pla
    tax
    pla
.endmacro

.macro set_vram_addr low_byte, mid_byte, bank, increment
    ; Set the starting VRAM address
    lda low_byte        ; Low byte of address 
    sta VERA::ADDR
    lda mid_byte        ; Middle byte of address 
    sta VERA::ADDR + 1
    lda bank            ; High bit of the address and auto-increment setup
    ora increment       ; Auto-increment amount
    sta VERA::ADDR + 2
.endmacro

; TODO: this could be done as a subroutine to save space if needed
.macro calc_vram_addr xpos_low, xpos_high, ypos
.scope
    y_temp_low  = $F0
    y_temp_high = $F1

    ; set bank to 30 TODO: use rodata or somewhere else?
    ; lda #$30
    ; sta $00

    ; make use of the lookup table
    clc
    lda ypos
    asl                 ; (y << 1)
    bcc @lower_bound    ; if carry is clear, then the result is less than 256
    tax
    lda LINE_LOOKUP_TABLE+256,x     ; Get the low byte of the address
    sta y_temp_low
    lda LINE_LOOKUP_TABLE+256+1,x   ; Get the high byte of the address
    sta y_temp_high
    bra @done
    @lower_bound:
    tax
    lda LINE_LOOKUP_TABLE,x         ; Get the low byte of the address
    sta y_temp_low
    lda LINE_LOOKUP_TABLE+1,x         ; Get the high byte of the address
    sta y_temp_high
    @done:

    ; set bank back to 0
    ; stz $00

    ; Calculate (x0 >> 1)
    lda xpos_high
    lsr
    lda xpos_low
    ror                 ; keep result in A

    ; Add (y << 5) + (y << 7) + (x0 >> 1)
    clc
    adc y_temp_low
    sta y_temp_low
    lda y_temp_high
    adc #$00            ; keep result in A

    ; Store the result in the VRAM address register
    sta VERA::ADDR + 1
    lda y_temp_low
    sta VERA::ADDR
    stz VERA::ADDR + 2 ; Disable auto-increment, set address bank to 0
.endscope
.endmacro ; calc_vram_addr

.macro calc_vram_addr_160 xpos, ypos
    ; same as calc_vram_addr without the (x >> 1) part
    vram_addr_l     = $F0    
    vram_addr_h     = $F1   

    ; set bank to 30
    ; lda #$30
    ; sta $00

    ; make use of the lookup table at $30A000
    lda ypos
    asl                 ; (y << 1)
    bcc @lower_bound    ; if carry is clear, then the result is less than 256
    tax
    lda LINE_LOOKUP_TABLE+256,x     ; Get the low byte of the address
    sta vram_addr_l
    lda LINE_LOOKUP_TABLE+256+1,x   ; Get the high byte of the address
    sta vram_addr_h
    bra @done
    @lower_bound:
    tax
    lda LINE_LOOKUP_TABLE,x         ; Get the low byte of the address
    sta vram_addr_l
    lda LINE_LOOKUP_TABLE+1,x         ; Get the high byte of the address
    sta vram_addr_h
    @done:

    ; set bank back to 0
    ; stz $00
    
    ; Add vram_addr + x
    lda xpos
    clc
    adc vram_addr_l            ; add low byte of (y << 5) + (y << 7)
    sta vram_addr_l            ; store low byte result (because 160<0xff)
    lda vram_addr_h
    adc #$00                   ; add carry
    sta vram_addr_h            ; store high byte result

    ; Store the result in the VRAM address register
    sta VERA::ADDR + 1
    lda vram_addr_l
    sta VERA::ADDR
    stz VERA::ADDR + 2 ; clear the upper byte of the VRAM address and any auto increment
.endmacro ; calc_vram_addr_160

.macro get_c_stack offset, addr
    ldy offset
    lda (C_STACK_ADDR),y   ; Get the parameter from the C-Stack
    sta addr
.endmacro

.macro pop_c_stack addr
    ldy #$00
    lda (C_STACK_ADDR),y  ; Get the parameter from the C-Stack
    sta addr
    ; move the C-Stack pointer up by 1
    inc C_STACK_ADDR
    bne *+4  ; If no overflow, skip the next instruction
    inc C_STACK_ADDR + 1
.endmacro

.macro push_c_stack value
    ; move the C-Stack pointer down by 1
    dec C_STACK_ADDR
    bne *+4  ; If no underflow, skip the next instruction
    dec C_STACK_ADDR + 1
    ; Store the value on the C-Stack
    ldy #$00
    lda value
    sta (C_STACK_ADDR),y
.endmacro