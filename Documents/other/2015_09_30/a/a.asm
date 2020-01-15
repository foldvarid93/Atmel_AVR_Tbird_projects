.include "m128def.inc"

.def a=r16
.def b=r17
.def c=r18
.def d=r19
.def e=r20
.def f=r21

.macro stack_init
ldi a,HIGH(RAMEND)
out SPH,a
ldi a,LOW(RAMEND)
out SPL,a
.endmacro

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

.macro port_init
ldi a,0
sts DDRG,a
ldi a,0xF0
out DDRD,a
out DDRB,a
.endmacro


.macro led_out
out PORTD,e
swap e
out PORTB,e
swap e
.endmacro
 
.org 0x00
rjmp init
.org 0x100

init:
	stack_init
	port_init
	//led_out
	rjmp start

start:


	

lds a,PING
cpi a,0
breq nulla
cpi a,1
breq g0
cpi a,2
breq g1
cpi a,4
breq g3


nulla:
ldi e,0
led_out
rjmp start

g0:
ldi e,1
v:
rol e
delay
led_out
rjmp v


g1:
clc
ldi e,128
v2:
ror e
delay
led_out
brcs g0
rjmp v2

g3:
clc 
ldi e,1
v3:
rol e
delay
led_out
brcs g1
rjmp v3







