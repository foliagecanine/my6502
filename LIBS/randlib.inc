;
; RANDLIB
;
; Random integer library for the 6502
;
; RAM required: 4 bytes
; RAM base var: RANDLIB
; Libs require: none
;
; Author: foliagecanine
; License: MIT License (see LICENSE.md)
;
seed = RANDLIB ; 2 bytes
xorseed = RANDLIB + 2 ; 2 bytes

;
; rand
;
; Returns a "random" integer in A
; Appears random, but may loop after 65536 calls. This is untested.
;
; params: none
; return:
;   a = random integer 0-255
;
rand:
  ; Rotate left
  rol seed
  rol seed + 1
  
  ; Add $931 and store in xorseed
  lda seed
  clc
  adc #$31
  sta xorseed
  lda seed + 1
  adc #$09
  sta xorseed + 1
  
  ; rol again, then xor the xorseed with the seed
  lda seed + 1
  rol
  eor xorseed + 1
  sta seed + 1
  lda seed
  rol
  eor xorseed
  sta seed
  
  ; low byte of seed is in a, return
  rts
  