; 65C02 instruction set
.pc02

; Commander X16 definitions
.include "cx16.inc"

; Custom Macros
.include "macros.inc"

; Switch to code segment
.code

; Initialize the VERA modes, layers, and screen resolution
.export _asm_init_vera
.proc _asm_init_vera
    ; Set layer 0 to 4bpp bitmap mode
    lda #VERA::BITMAP4BPP
    sta VERA::L0::CONFIG

    ; Set layer 0 tilebase to $12000 in VRAM (0x12000 / 2048 = 0x24), (0x24 << 2 = 0x90)
    ; stz VERA::L0::TILE_BASE 
    lda #$90
    sta VERA::L0::TILE_BASE

    ; Set layer 1 to 256 color 1 bit tile mode
    ;lda #VERA::T256C | VERA::TILE1BPP |  VERA::MAP::WIDTH128 | VERA::MAP::HEIGHT64
    lda #VERA::TILE1BPP |  VERA::MAP::WIDTH128 | VERA::MAP::HEIGHT64
    sta VERA::L1::CONFIG

    ; Enable layers 0 and 1 and Sprite layer
    lda #VERA::DISP::ENABLE::LAYER0 | VERA::DISP::ENABLE::SPRITES | VERA::DISP::ENABLE::LAYER1 | VERA::DISP::MODE::VGA
    sta VERA::DISP::VIDEO

    ; Set the screen resolution to 320x240
    lda #$40 ; (320 / 640) * 128 = 64
    sta VERA::DISP::HSCALE
    lda #$36 ; (200 / 480) * 128 = 53.3333
    sta VERA::DISP::VSCALE

    jsr setup_line_table
    rts
.endproc ; _asm_init_vera

; Clear VRAM using VERA's 32-bit cache feature
.export _asm_clear_screen
.proc _asm_clear_screen
    ; Set the VERA address to 0x12000 (start of VRAM)
    stz VERA::ADDR
    lda #$20
    sta VERA::ADDR + 1
    lda #$01
    sta VERA::ADDR + 2

    ; Enable DCSEL mode 2 to allow cache operations
    lda #%00000100
    sta $9F25           ; VERA::CTRL

    ; Enable cache writing 
    lda #%01000000
    sta $9F29           ; FX_CTRL

    ; Change DCSEL to mode 6 for cache write operations
    lda #%00001100
    sta $9F25           ; VERA::CTRL

    ; Prepare the 32-bit cache with zeros
    stz $9f29
    stz $9f2A
    stz $9f2B
    stz $9f2C

    ; Set address auto-increment to 4 bytes
    lda #%00110000      ; Mode 3 is a 4 byte increment
    ora VERA::ADDR + 2  
    sta VERA::ADDR + 2  ; VERA::ADDR + 2

    ; Calculate loop count for 320x240 pixels in 4bpp mode (38400 bytes total)
    ; Since we are writing 4 bytes at a time, we need 38400/4 writes
    ; This will write 8 pixels at a time (32 bits)
    ldx #$20            ; High byte
    ldy #$00            ; Low byte

    lda #%00000000      ; Set the mask (none in this case)

clear_loop:
    sta $9F23           ; Write the 32-bit cache to VRAM (4 bytes of zeros) VERA::DATA0
    dey                 ; Decrement low byte of loop counter
    bne clear_loop      ; Continue loop if low byte is not zero
    dex                 ; Decrement high byte of loop counter
    bne clear_loop      ; Continue loop if high byte is not zero

    lda #%00000100      ; DCSEL = Mode 2
    sta $9F25           ; VERA::CTRL
    lda #%00000000      ; Disable cache writing
    sta $9F29           ; FX_CTRL

    lda #%00000000      ; DCSEL = Mode 0
    sta $9F25           ; VERA::CTRL

    rts                 ; Return from subroutine

.endproc ; _asm_clear_screen

; Clear VRAM from address 0x0000 to a colour
.export _asm_clear_vis_color
.proc _asm_clear_vis_color
    color = $A0

    sta color            ; Store the color
    ; Set the starting VRAM address to 0x0000
    set_vram_addr #$00, #$00, #0, #VERA::INC1

    ; Set up the loop counter to 36768 bytes (320x240 pixels in 4bpp mode)
    ldx #$80           ; High byte of loop counter 
    ldy #$00           ; Low byte of loop counter 

    ; Set the upper and lower 4 bits of the color
    lda color           ; Load the color
    asl
    asl
    asl
    asl
    ora color

clear_loop:
    sta VERA::DATA0    ; Write the color to the current VRAM address
    dey                ; Decrement low byte of loop counter
    bne clear_loop     ; Continue loop if low byte is not zero
    dex                ; Decrement high byte of loop counter
    bne clear_loop     ; Continue loop if high byte is not zero

    rts                ; Return from subroutine
.endproc ; _asm_clear_vis_color

; Clear VRAM from address 0x9600 to a colour
.export _asm_clear_pri_color
.proc _asm_clear_pri_color
    color = $A0
  
    sta color            ; Store the color
    ; Set the starting VRAM address to $7D00
    set_vram_addr #$00, #SCREEN_SIZE, #0, #VERA::INC1

    ; Set up the loop counter to 160x167 pixels
    ldx #$68           ; High byte of loop counter 
    ldy #$60           ; Low byte of loop counter 

    ; Set the upper and lower 4 bits of the color
    lda color           ; Load the color
    asl
    asl
    asl
    asl
    ora color

clear_loop:
    sta VERA::DATA0    ; Write the color to the current VRAM address
    dey                ; Decrement low byte of loop counter
    bne clear_loop     ; Continue loop if low byte is not zero
    dex                ; Decrement high byte of loop counter
    bne clear_loop     ; Continue loop if high byte is not zero

    rts                ; Return from subroutine
.endproc ; _asm_clear_pri_color


; Clear VRAM for layer 1 starting from address 0x1B000 for $3C00 bytes
.export _asm_clear_overlay
.proc _asm_clear_overlay
    set_vram_addr #$00, #$B0, #$01, #VERA::INC1

    ; Set up the loop counter (256*60 = 15360 bytes)
    ldx #$3C                ; High byte of loop counter
    ldy #$00                ; Low byte of loop counter

@loop:
    stz VERA::DATA0         ; Write zero to the current VRAM address
    dey                     ; Decrement low byte of loop counter
    bne @loop               ; Continue loop if low byte is not zero
    dex                     ; Decrement high byte of loop counter
    bne @loop               ; Continue loop if high byte is not zero

    rts                     ; Return from subroutine
.endproc ; _asm_clear_overlay


.proc setup_line_table
    LUT_BANK = $30
    LINE_LENGTH = 160
    ADDRESS = $400

    ; Set the current bank to the line table bank
    ; lda #LUT_BANK
    ; sta $00
    
    ; each entry is a 16 bit value
    stz ADDRESS
    stz ADDRESS+1

    ldx #0
    ; load the previous value
    lda ADDRESS+1,x
    tay
    lda ADDRESS,x
@loop1:
    clc
    inx
    inx
    adc #<LINE_LENGTH   ; add LINE_LENGTH
    sta ADDRESS,x
    tya
    adc #>LINE_LENGTH   ; add carry
    sta ADDRESS+1,x
    tay                 ; load previous value for next iteration
    lda ADDRESS,x
    cpx #200            ; did we reach 200 lines?
    bne @loop1

    ldx #0

@loop2:
    clc
    inx
    inx
    adc #<LINE_LENGTH   ; add LINE_LENGTH
    sta ADDRESS+200,x
    tya
    adc #>LINE_LENGTH   ; add carry
    sta ADDRESS+201,x
    tay                 ; load previous value for next iteration
    lda ADDRESS+200,x
    cpx #200            ; did we reach 200 lines?
    bne @loop2

    ; stz $00 ; restore the bank
    rts

.endproc ; setup_line_table
