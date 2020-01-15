.include "m128def.inc"

.def tmp=R16
.def leds=R17
.def buttons=R18
.def b=r19
.def c=r20
.def d=r21


.macro delay
ldi b,255
ldi c,255
ldi d,255
d0:
dec b
brne d0
d1:
dec c
brne d1
d2:
dec d
brne d0
.endmacro

.macro	showleds
	mov 	tmp,	leds
	swap	tmp
	andi	tmp,	0b11110000
	out	PORTB,tmp

	mov 	tmp,	leds
	andi	tmp,	0b11110000
	out	PORTD,	tmp
.endmacro 

.org 0x00  //ide helyezze el a köv.utasítást,a 0-s címre.
	    //itt van a reset vektor van
	rjmp init

.org 0x46
init:
	;init led,buttons,etc.

	;led 0-3
	in tmp,DDRB
	ori tmp,0xF0
	out DDRB,tmp

	;led 4-7
	in tmp,DDRD
	ori tmp,0xF0
	out DDRD,tmp

	;K0-K4
	ldi tmp,DDRG
	andi tmp,0b11100000
	sts DDRG,tmp

	rjmp main

main:

	clr leds  	;regiszter reset
	showleds	;kioltja az összes ledet
	sec		;beállítom a carryt


	lds buttons,	PING
	sbrc	buttons, 0
	rjmp LoopL

	sbrc	buttons, 1
	rjmp LoopR
	rjmp	main
	
LoopL:

	rol leds
	delay
	showleds

	sbrc leds,7  ;7nél kiugrik,megnézzük h milyen gomb van megnyomva
	rjmp main

	rjmp LoopL

	
LoopR:

	ror leds
	delay
	showleds

	sbrc leds,0
	rjmp main

	rjmp LoopR


rjmp main














































