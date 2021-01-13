;
; MATHLIB
;
; Math library for 6502. Provides division and multiplication support
;
; RAM required: 6 bytes
; RAM base var: MATHLIB
; Libs require: none
;
; Author: foliagecanine
; License: MIT License (see LICENSE.md)
; Attribution: A portion of this code is based off of Ben Eater's 6502 code <eater.net/6502> and used under CC-BY-4.0
;

m_modulo      = MATHLIB + 0 ; 2 bytes
m_numerator   = MATHLIB + 2 ; 2 bytes
m_denominator = MATHLIB + 4 ; 2 bytes

; Reuse variables
m_result      = m_modulo
m_factor1     = m_numerator
m_factor2     = m_denominator

;
; div8
;
; Divides a by x and returns the quotient and remainder (modulo)
;
; params:
;   a = dividend/numerator
;   x = divisor/denominator
; return:
;   a = quotient/result
;   x = remainder/modulo
;  
div8:
  sta m_numerator
  stx m_denominator
  
  ; Initialize the remainder to zero
  lda #0
  sta m_modulo
  clc
  
  ldx #8
.divloop8:
  ; Rotate quotient and remainder
  rol m_numerator
  rol m_modulo
  
  ; a = dividend - divisor
  sec
  lda m_modulo
  sbc m_denominator
  bcc .ignore_result8  ; branch if dividend < divisor
  sta m_modulo
.ignore_result8:
  dex
  bne .divloop8
  rol m_numerator ; shift in the last bit of the quotient
  
  lda m_numerator
  ldx m_modulo
  
  rts
  
;
; mul8
;
; Multiplies a by x and returns the product
;
; params:
;   a = factor 1
;   x = factor 2
; return:
;   a = product/result
;  
mul8:
  sta m_factor1
  stx m_factor2

  lda #0
  ldx #8
.mulloop8:
  lsr m_factor2
  bcc .ignore_bit8
  clc
  adc m_factor1
.ignore_bit8:
  ror a
  ror m_result
  dex
  bne .mulloop8
  tax
  lda m_result
  
  rts
  
;
; div8
;
; Divides a by x and returns the quotient and remainder (modulo)
;
; params:
;   a,y = lowbyte,hibyte dividend/numerator
;   x = divisor/denominator
; return:
;   a,y = lowbyte,hibyte quotient/result
;   x = remainder/modulo
; 
div16:
  sta m_numerator
  sty m_numerator + 1
  stx m_denominator
  
  ; Initialize the remainder to zero
  lda #0
  sta m_modulo
  sta m_modulo + 1
  clc
  
  ldx #16
.divloop16:
  ; Rotate quotient and remainder
  rol m_numerator
  rol m_numerator + 1
  rol m_modulo
  rol m_modulo + 1
  
  ; a,y = dividend - divisor
  sec
  lda m_modulo
  sbc m_denominator
  tay
  lda m_modulo + 1
  sbc #0
  bcc .ignore_result16  ; branch if dividend < divisor
  sty m_modulo
  sta m_modulo + 1
.ignore_result16:
  dex
  bne .divloop16
  rol m_numerator ; shift in the last bit of the quotient
  rol m_numerator + 1
  
  lda m_numerator
  ldy m_numerator + 1
  ldx m_modulo
  
  rts
  
