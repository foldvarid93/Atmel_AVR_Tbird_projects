; TanárÚr ZH megoldása
.include "m128def.inc"

.def tmp=	r16
.def num21=	r17;elsõ 2bit
.def num34=	r18;utolsó 2bit
.def digit=	r19
.def buttons=	r20
.def state=	r21;0->1918;1->2015 _kell alapállapot!


.org 0x00

	rjmp init

.org 0x46

init:
	;k0/k4
	lds tmp,DDRG
	andi tmp,0b11100000
	sts DDRG,tmp
	

	;7seg
	ldi tmp,0xFF
	out DDRA,tmp

	;stack
	ldi tmp,HIGH(RAMEND)
	out SPH,tmp
	ldi tmp,LOW(RAMEND)
	out SPL,tmp


	rjmp main


main:
	;ldi 	num34,0x19
	;ldi 	num21,0x18

	ldi	state,0


	loop:
	lds 	buttons,PING
	sbrc	buttons,0
	rcall	k0pressed
	sbrc	buttons,1
	rcall	k1pressed	

	rcall	show7
	;------
	cpi  	state,0
	breq	state0
	cpi	state,1
	breq	state1
	back:
	;------
	rjmp loop

state0:
	ldi 	num34,0x19
	ldi 	num21,0x18
	rjmp 	back
state1:
	ldi 	num34,0x20
	ldi 	num21,0x15
	rjmp 	back


k0pressed:

	;ldi 	num34,	0x20
	;ldi	num21,	0x15
	ldi 	state,	1
	rcall	show7 

	


ret


k1pressed:
ldi state,0
ret









show7:

		;digit1(jobbrol)
		mov 	tmp,num21
		andi	tmp,0b0000_1111
		ori	tmp,0b1_000_0000
		out 	PORTA,tmp
		rcall	delay
		;digit2(jobbrol)
		mov 	tmp,num21
		swap    tmp
		andi	tmp,0b00001111 		;0b0000001	
		ori	tmp,0b1_001_0000 	;0b1001001
		out 	PORTA,tmp
		swap	tmp
		rcall	delay
		;digit3(jobbrol)
		mov 	tmp,num34
		andi	tmp,0b00001111	
		ori	tmp,0b1_010_0000
		out 	PORTA,tmp
		rcall	delay
		;digit4(jobbrol)
		mov 	tmp,num34
		swap    tmp
		andi	tmp,0b00001111 			
		ori	tmp,0b1_011_0000 	
		out 	PORTA,tmp
		swap	tmp
		rcall	delay
	ret


delay:

	push	R25
	push	R24

	ldi 	r25,0xff
	ldi	r24,0x0f

	delay_loop:

		dec	R25
		brne	delay_loop
		dec	R24
		brne	delay_loop

	
	pop	R24
	pop	R25
	ret



































































