.include "m128def.inc"

.CSEG

.def	LED		=	r17

.ORG	0x00
		rjmp	start
.macro	port_init
		ldi		r16,	0xf0
		out		DDRB,	r16
		out		DDRD,	r16
.endmacro

.macro	stack_init
		ldi		r16,	high(RAMEND)
		out		SPH,	r16
		ldi		r16,	low(RAMEND)
		out		SPL,	r16
.endmacro

LED_out:
		out		PORTD,	LED
		swap	LED
		out		PORTB,	LED
		swap	LED
		ret

.ORG	0x100

LED_running:
		call	LED_out
		call	delay_1s
		ror		LED
		ret

delay_1s:
		ldi		r18,	10
		eor		r19,	r19
		eor		r20,	r20
vissza:
		dec		r20
		brne	vissza
		dec		r19
		brne	vissza
		dec		r18
		brne	vissza

		ret

start:
		stack_init
		port_init

		ldi		LED,	0x80
		
start1:
		call	LED_running

		rjmp	start1
