;
; DinoRun
;
; Author: foliagecanine
; License: MIT License (see LICENSE.md)
; Processor: 65c02
;
; Gameplay based off of Chromium's T-Rex Runner game.
; https://chromium.googlesource.com/chromium/src.git/+/master/components/neterror/resources/offline.js
;

VIAIOBASE = $6000
MATHLIB = $0200 ; 6 bytes
PRINTLIB = MATHLIB + 6 ; 10 bytes
RANDLIB = PRINTLIB + 10 ; 4 bytes
counter = RANDLIB + 4 ; 2 bytes
divider = counter + 2 ; 1 byte
jumpcounter = divider + 1 ; 1 byte
clockdivision = jumpcounter + 1 ; 1 byte
cactus1pos = clockdivision + 1 ; 1 byte
cactus2pos = cactus1pos + 1 ; 1 byte
birdpos = cactus2pos + 1 ; 1 byte
speedstate = birdpos + 1 ; 1 byte

gameflags = speedstate + 1 ; 1 byte
; bit 0: has landed
; bit 1: scheduled tick
; bit 2: dino char toggle

  ; See comment above .org $E100
  .org $E000
nmi:
irq:
  pha
  txa
  pha
  tya
  pha
  ; Check whether the interrupt was caused by the timer or the button
  jsr clear_timer1_int
  bcc .nottimer
  
  ; Timer was a cause for the interrupt.
  ; Increment the divider's counter
  inc divider
  lda divider
  
  ; Check whether the divider counter is greater or equal to the clock division (game speed)
  clc
  cmp clockdivision
  bcc .notick
  
  ; Set the scheduled tick flag
  lda gameflags
  ora #%00000010
  sta gameflags
  
  ; We need to increment the game tick counter
  ; Reset the divider counter
  lda #0
  sta divider
  ; Increment game tick counter and handle carry
  inc counter
  bne .nocarry
  inc counter + 1
.nocarry:
  ; Decrement the jump counter if greater than zero
  lda jumpcounter
  beq .nojump
  dec jumpcounter
  jmp .notick
.nojump:
  ; Check whether haslanded is zero
  ; If it is, set the flag. We have now been landed for 1 tick.
  lda gameflags
  and #%00000001
  bne .skiplanded
  ora #%00000001
  sta gameflags
.skiplanded:
.notick:
.nottimer:
  lda jumpcounter
  bne .cantjump
  lda gameflags
  and #%00000001 ; haslanded flag
  beq .cantjump
  lda #%00000010
  jsr check_pin_int
  bcc .notpin
  lda #4
  sta jumpcounter
  jsr rand ; Generate entropy
.cantjump:
  jsr clear_pin_int
.notpin:
  pla
  tay
  pla
  tax
  pla
  rti

  ; Reset vector at $E100 to allow room for the interrupt vector.
  ; This way we don't have to reprogram the whole EEPROM every time
  ; since the vector pointers are at the end of memory.
  .org $E100
  
reset:
  ldx #$ff
  txs
  
  lda #0
  sta counter
  sta counter + 1
  sta divider
  sta jumpcounter
  lda #2
  sta gameflags
  lda #10 ; one tick per 3/4 sec
  sta clockdivision
  lda #$ff
  sta birdpos
  sta cactus1pos
  sta cactus2pos
  
  jsr init_lcd
  lda #%10000010
  jsr pin_interrupt
  jsr clear_lcd
  
  ; Register Dino character 1
  lda #0
  ldx #<dinochar1
  ldy #>dinochar1
  jsr lcd_customchar
  
  ; Register Dino character 2
  lda #4 ; so that we can use bit 4 as the dino char toggle in gameflags
  ldx #<dinochar2
  ldy #>dinochar2
  jsr lcd_customchar
  
  ; Register Bird character
  lda #1
  ldx #<birdchar
  ldy #>birdchar
  jsr lcd_customchar
  
  ; Register Cactus character
  lda #2
  ldx #<cactuschar
  ldy #>cactuschar
  jsr lcd_customchar
  
  ; Allow interrupts
  cli
  
  ; Start the VIA chip's internal "timer."
  ; Set it to interrupt every 50000 clock cycles.
  ; At 1MHz, that means it will interrupt every 20th of a second.
  lda #%01000000 ; Timer 1 continuous mode, no pin toggle
  ldx #<50000
  ldy #>50000
  jsr start_timer1
  
loop:
  ; Check whether dino is jumping or not
  lda jumpcounter
  beq .nojump
  ldx #0
  ldy #1
  lda #" "
  jsr print_charat
  dey
  lda gameflags
  and #%00000100
  jsr print_charat
  jmp .endjump
.nojump:
  ldx #0
  ldy #0
  lda #" "
  jsr print_charat
  iny
  lda gameflags
  and #%00000100
  jsr print_charat
.endjump:

  ; Print enemies
  lda #1
  ldx birdpos
  jsr drawenemy
  lda #2
  ldx cactus1pos
  jsr drawenemy
  lda #2
  ldx cactus2pos
  jsr drawenemy
  
  ; Print current game tick (score)
  ldx #11
  ldy #0
  jsr lcd_setcursor
  sei
  lda counter
  ldy counter + 1
  cli
  jsr print_decimal16
  
  ; Check for a scheduled tick
  lda gameflags
  and #%00000010
  bne .scheduledtick
  jmp .noscheduledtick
.scheduledtick
  
  ; Toggle dino char
  lda gameflags
  eor #%00000100
  sta gameflags
  
  ; Since we are in a scheduled tick, move all existing items towards dino
  ; Give other items a chance to spawn
  lda cactus1pos
  cmp #$ff
  beq .nomovecactus1
  dec cactus1pos
  jmp .endcactus1
.nomovecactus1:
  
  jsr rand
  cmp #$40 ; 25% chance
  bcs .endcactus1
  lda #16
  sta cactus1pos
.endcactus1:

  lda cactus2pos
  cmp #$ff
  beq .nomovecactus2
  dec cactus2pos
  jmp .endcactus2
.nomovecactus2:
  jsr rand
  cmp #$40 ; 25% chance
  bcs .endcactus2
  lda #16
  sta cactus2pos
.endcactus2:

  lda birdpos
  cmp #$ff
  beq .nomovebird
  dec birdpos
  jmp .endbird
.nomovebird:
  ; Check to make sure we aren't going to create an impossible circumstance, such as below
  ;      B
  ; D    C
  lda cactus1pos
  cmp #13
  bcs .endbird
  lda cactus2pos
  cmp #13
  bcs .endbird
  
  jsr rand
  cmp #$40 ; 25% chance
  bcs .endbird
  lda #16
  sta birdpos
.endbird:

  ; Check whether dino is touching a bird or a cactus
  lda cactus1pos
  cmp #0
  bne .skipcactus1test
  lda jumpcounter
  beq die
.skipcactus1test
  lda cactus2pos
  cmp #0
  bne .skipcactus2test
  lda jumpcounter
  beq die
.skipcactus2test:
  lda birdpos
  cmp #0
  bne .skipbirdtest
  lda jumpcounter
  bne die
.skipbirdtest:

  ; Increase speed if necessary
  ldy speedstate
  lda fastertimes,y
  cmp counter
  bne .noincrease
  iny
  lda fastertimes,y
  cmp counter + 1
  bne .noincrease
  inc speedstate
  inc speedstate
  dec clockdivision
.noincrease:

  ; Clear scheduled tick
  lda gameflags
  and #%11111101
  sta gameflags
.noscheduledtick:

  jsr rand
  wai
  jmp loop

drawenemy:
  cpx #$ff
  beq .nodraw
  tay
  dey
  jsr print_charat
  inx
  lda #" "
  jsr print_charat
.nodraw:
  rts
  
die:
  ldx #11
  ldy #1
  jsr lcd_setcursor
  ldx #<bonktext
  ldy #>bonktext
  jsr print_string
.here:
  jmp .here ; Loop
  
bonktext .asciiz "Bonk!"
dinochar1 .byte $06, $07, $07, $0E, $0F, $1E, $0A, $02
dinochar2 .byte $06, $07, $07, $0E, $0F, $1E, $0A, $08
cactuschar .byte $00, $04, $04, $15, $1D, $07, $04, $04
birdchar .byte $00, $00, $0C, $1C, $0F, $06, $00, $00
fastertimes .word 50, 125, 250, 400, 600, 850, 1200, 1500, 65535
  
  .include "lcdlib.inc"
  .include "mathlib.inc"
  .include "printlib.inc"
  .include "vialib.inc"
  .include "randlib.inc"
  
  .org $fffa
  .word nmi
  .word reset
  .word irq
  