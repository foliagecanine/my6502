;
; VIALIB
;
; VIA interface library 
;
; RAM required: 16 byte IO range
; RAM base var: VIAIOBASE
; Libs require: none 
;
; Author: foliagecanine
; License: MIT License (see LICENSE.md)
;

PORTB = VIAIOBASE + 0
PORTA = VIAIOBASE + 1
DDRB  = VIAIOBASE + 2
DDRA  = VIAIOBASE + 3
T1_LC = VIAIOBASE + 4
T1_HC = VIAIOBASE + 5
T1_LL = VIAIOBASE + 6
T1_HL = VIAIOBASE + 7
T2_LC = VIAIOBASE + 8
T2_HC = VIAIOBASE + 9
SHIFT = VIAIOBASE + 10
ACR   = VIAIOBASE + 11
PCR   = VIAIOBASE + 12
IFR   = VIAIOBASE + 13
IER   = VIAIOBASE + 14
FASTA = VIAIOBASE + 15

;
; start_timer1
;
; Creates a timer on timer1 of the VIA chip
;
; params:
;   a = timer mode (see WDC65C22 datasheet)
;   x = timer low byte
;   y = timer high byte
; return: none
;
start_timer1:
  sta ACR
  stx T1_LC
  sty T1_HC
  pha
  lda #%11000000
  sta IER
  pla
  rts
  
; pin_interrupt
;
; Enable interrupts when a pin is pulled high or low.
; Pin information can be found in the VIA chip's datasheet.
;
; params:
;   a = pin
;     #%10000001 = Enable CA2
;     #%10000010 = Enable CA1
;     #%10001000 = Enable CB2
;     #%10010000 = Enable CB1
;       values may be OR-ed together to enable multiple pins at once
;     #%00000001 = Disable CA2
;     #%00000010 = Disable CA1
;     #%00001000 = Disable CB2
;     #%00010000 = Disable CB1
;       values may be OR-ed together to disable multiple pins at once
;   x = mode (see WDC65C22 datasheet)
; return: none
;
pin_interrupt:
  sta IER
  stx PCR
  rts

;
; clear_timer1_int
;
; Checks if the timer 1 is interrupting.
; If it is, it sets the carry bit and clears the interrupt.
;
; params: none
; return:
;   flags.carry = This was the source of the interrupt
;
clear_timer1_int:
  pha
  clc
  lda IFR
  and #%01000000
  beq .noint
  bit T1_LC
  sec ; Set carry to notify callee that the interrupt came from this source.
.noint:
  pla
  rts
  
; check_pin_int
;
; Checks whether the specified pin is the cause of an interrupt
; Pin information can be found in the VIA chip's datasheet.
;
; params:
;   a = pin
;     #%00000001 = CA2
;     #%00000010 = CA1
;     #%00001000 = CB2
;     #%00010000 = CB1
;       values may be OR-ed together to check multiple pins at once
; return:
;   flags.carry = Pin was cause of interrupt
;
check_pin_int:
  clc
  bit IFR
  beq .noint
  sec ; Set carry to notify callee that the interrupt came from this source.
.noint:
  rts
  
; clear_pin_int
;
; Clears all pin interrupts.
;
; params: none
; return: none
;
clear_pin_int:
  bit PORTA
  bit PORTB
  rts
  
  