; 65C02 instruction set
.pc02

; Commander X16 definitions
.include "cx16.inc"

; Custom Macros
.include "macros.inc"

; Switch to code segment
.code

; wait function - system clock ticks at 60Hz
.export _asm_wait_for_refresh
.proc _asm_wait_for_refresh
    time = $80; zero page address
    old_bank = $81; zero page address
    
    cli ; clear interrupts
    
    lda $00;
    sta old_bank; store the current bank in the zero page

    stz $00; set the bank to 0

    jsr $FFDE; call the kernal function
    sta time; store the value in the start variable

    loop:
        jsr $FFDE; call the kernal function again
        cmp time; compare the value to the start variable
        beq loop; if they are equal, loop again
    
    lda old_bank; load the old bank value
    sta $00; set the bank back to the old value
.endproc; end of _asm_wait_for_refresh