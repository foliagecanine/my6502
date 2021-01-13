;
; LCDLIB
;
; LCD interface library for 6502. Requires a VIA chip.
;
; RAM required: none
; RAM base var: none
; Libs require: VIALIB 
;
; Author: foliagecanine
; License: MIT License (see LICENSE.md)
; Attribution: A portion of this code is based off of Ben Eater's 6502 code <eater.net/6502> and used under CC-BY-4.0
;

LCD_E  = %10000000
LCD_RW = %01000000
LCD_RS = %00100000

;
; init_lcd
; 
; Initializes 16x2 LCD display
;
; params: none
; return: none
;
init_lcd:
  lda #%11111111 ; Set all pins on port B to output
  sta DDRB
  lda #%11100000 ; Set top 3 pins on port A to output
  sta DDRA

  lda #%00111000 ; Set 8-bit mode; 2-line display; 5x8 font
  jsr lcd_instruction
  lda #%00001100 ; Display on; cursor on; blink off
  jsr lcd_instruction
  lda #%00000110 ; Increment and shift cursor; don't shift display
  jsr lcd_instruction
  jsr clear_lcd
  
  rts

;
; clear_lcd
;
; Clears the LCD display
;
; params: none
; return: none
; 
clear_lcd:
  pha
  lda #%00000001 ; Clear display
  jsr lcd_instruction
  pla
  rts

;
; lcd_home
;
; Returns the cursor to the home (0,0)
;
; params: none
; return: none
; 
lcd_home:
  pha
  lda #%00000010 ; Return home
  jsr lcd_instruction
  pla
  rts
  
lcd_wait:
  pha
  lda #%00000000  ; Port B is input
  sta DDRB
.lcdbusy:
  lda #LCD_RW
  sta PORTA
  lda #(LCD_RW | LCD_E)
  sta PORTA
  lda PORTB
  and #%10000000
  bne .lcdbusy

  lda #LCD_RW
  sta PORTA
  lda #%11111111  ; Port B is output
  sta DDRB
  pla
  rts

;
; lcd_instruction
;
; Sends an instruction to the LCD controller
;
; params:
;   a = instruction code
; return: none
;  
lcd_instruction:
  pha
  jsr lcd_wait
  sta PORTB
  lda #0         ; Clear LCD_RS/LCD_RW/LCD_E bits
  sta PORTA
  lda #LCD_E     ; Set LCD_E bit to send instruction
  sta PORTA
  lda #0         ; Clear LCD_RS/LCD_RW/LCD_E bits
  sta PORTA
  pla
  rts

;
; lcd_setcursor
;
; Sends an instruction to the LCD controller
;
; params:
;   x = column
;   y = row
; return: none
;
lcd_setcursor:
  pha
  stx $C0
  lda #$80
  cpy #0
  beq .add_xpos
  lda #$C0
.add_xpos:
  clc
  adc $C0 ; X reg stored
  jsr lcd_instruction
  pla
  rts

;
; lcd_customchar
;
; Registers a custom character on the LCD
; NOTE: This function will reset the cursor to home.
;
; params:
;   a = character number (0-7)
;   x,y = lowbyte,hibyte address of character memory
; return: none
;
lcd_customchar:
  pha
  ; Store in zeropage so we can access the correct address dynamically.
  ; We will also restore the x and y registers' original values from here.
  stx $80
  sty $81
  
  ; Multiply the character number by 8 to get the CGRAM offset
  ldx #8
  jsr mul8
  ; OR with $40 to set memory to CGRAM
  ora #$40
  jsr lcd_instruction
  
  ; Send all the data bytes to the LCD module
  ldx #8
  ldy #0
.loadloop:
  lda ($80),y
  jsr print_char
  iny
  dex
  bne .loadloop
  
  ; Return to DDRAM
  lda #%10000000
  jsr lcd_instruction
  
  ldx $80
  ldy $81
  pla
  rts
  
  
  
;
; print_char
;
; Prints a character to the LCD display
;
; params:
;   a = ASCII character
; return: none
;   
print_char:
  pha
  jsr lcd_wait
  sta PORTB
  lda #LCD_RS         ; Set LCD_RS; Clear LCD_RW/LCD_E bits
  sta PORTA
  lda #(LCD_RS | LCD_E)   ; Set LCD_E bit to send instruction
  sta PORTA
  lda #LCD_RS         ; Clear LCD_E bits
  sta PORTA
  pla
  rts