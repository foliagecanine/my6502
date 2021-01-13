;
; PRINTLIB
;
; Text display library for 6502. Provides support for ASCII and decimal printing.
;
; RAM required: 10 bytes
; RAM base var: PRINTLIB
; Libs require: LCDLIB
;
; Author: foliagecanine
; License: MIT License (see LICENSE.md)
;

p_printbuffer = PRINTLIB + 0 ; 5 bytes
p_regstore = PRINTLIB + 5 ; 3 bytes
p_hibyte = PRINTLIB + 8 ; 1 byte
p_counter = PRINTLIB + 9 ; 1 byte

;
; print_decimal
;
; Prints a decimal number
;
; params:
;   a = number to print
; return: none
;  
print_decimal:
  ; Store registers so they don't get clobbered by this function
  sta p_regstore
  stx p_regstore + 1
  sty p_regstore + 2
  
  ; Initialize the buffer pointer
  ldy #0
  
.decloop:
  ; Divide the number in A by 10
  ldx #10
  jsr div8
  
  ; Push the remainder to the buffer
  pha
  txa
  clc
  adc #"0"
  jsr printlib_push_buffer
  pla
  
  ; Repeat if the result of the division was not zero
  ora #0
  bne .decloop
  
.print_buffer:
  ; Print characters in the buffer until y (the number of characters) reaches 0
  dey
  lda p_printbuffer,y
  jsr print_char
  tya
  ora #0
  bne .print_buffer
  
  ; Reload the saved values to prevent clobbering
  lda p_regstore
  ldx p_regstore + 1
  ldy p_regstore + 2
  
  rts
  
printlib_push_buffer:
  sta p_printbuffer,y
  iny
  rts

;
; print_decimal16
;
; Prints a 16-bit decimal number
;
; params:
;   a,y = lowbyte,hibyte number to print
; return: none
;  
print_decimal16:
  ; Store registers so they don't get clobbered by this function
  sta p_regstore
  stx p_regstore + 1
  sty p_regstore + 2
  
  ; Initialize the buffer pointer
  ldx #0
  stx p_counter
  sty p_hibyte
  
.decloop:
  ldy p_hibyte
  ; Divide the number in a,y by 10
  ldx #10
  jsr div16
  sty p_hibyte
  ldy p_counter
  
  ; Push the remainder to the buffer
  pha
  txa
  clc
  adc #"0"
  jsr printlib_push_buffer
  pla
  
  ; Repeat if the result of the division was not zero
  ora #0
  sty p_counter
  bne .decloop
  
.print_buffer:
  ; Print characters in the buffer until y (the number of characters) reaches 0
  dey
  lda p_printbuffer,y
  jsr print_char
  tya
  ora #0
  bne .print_buffer
  
  ; Reload the saved values to prevent clobbering
  lda p_regstore
  ldx p_regstore + 1
  ldy p_regstore + 2
  
  rts
  
;
; print_charat
;
; Prints a character at a coordinate on the LCD
;
; params:
;   a = character
;   x = x pos
;   y = y pos
; return: none
;
print_charat:
  jsr lcd_setcursor
  jsr print_char
  rts
  
;
; print_string
;
; Prints a NULL-terminated ASCII string
;
; params:
;   x,y - lowbyte,hibyte address of NULL-terminated ASCII string
; return: none
;  
print_string:
  stx $80
  sty $81
  pha
  tya
  pha
  ldy #0
.printstr_loop:
  lda ($80),y
  ora #0
  beq .exit_loop
  jsr print_char
  iny
  jmp .printstr_loop
.exit_loop:
  pla
  tay
  pla
  rts
  