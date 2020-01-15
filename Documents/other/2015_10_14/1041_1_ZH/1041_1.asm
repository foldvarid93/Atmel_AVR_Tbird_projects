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

.macro port_init
ldi a,0
sts DDRG,a
ldi a,0xFF
out DDRA,a
.endmacro




.org 0x00
rjmp init
.org 0x100

init:
	port_init
	stack_init
	rjmp start

start:

lds a,PORTG
cpi a,0
breq alap
cpi a,1
breq g1
cpi a,2
breq g1
rjmp start


alap:

ldi b,0b10000000
ori b,2
out porta,b
call delay
ldi b,0b10010000
ori b,9
out porta,b
call delay
ldi b,0b10100000
ori b,9
out porta,b
call delay
ldi b,0b10110000
ori b,1
out porta,b
call delay
rjmp start

g0:
lds a,portg
cpi a,0
breq start
ldi b,0b10000000
ori b,4
out porta,b
call delay
ldi b,0b10010000
ori b,1
out porta,b
call delay
ldi b,0b10110000
ori b,0
out porta,b
call delay
ldi b,0b10100000
ori b,1
out porta,b
call delay
rjmp g0



g1:
lds a,portg
cpi a,0
breq start
ldi b,0b10000000
ori b,9
out porta,b
call delay
ldi b,0b10010000
ori b,7
out porta,b
call delay
ldi b,0b10000000
ori b,8
out porta,b
call delay
rjmp g1




delay:
ldi c,1
ldi d,100
ldi e,50
d1:
dec e
brne d1
d2:
dec d
brne d1
d3:
dec c
brne d1
ret






































	
