.include "m128def.inc"

.def ki=R16
.def rgb=R17
.def a=R18
.def gomb=r19
.def ki2=r20
.def d=r21

.macro stack_init
ldi a,high(ramend)
out sph,a
ldi a,low(ramend)
out spl,a
.endmacro

.macro port_init
ldi a, 0b10000000
out DDRC,a
ldi a,0b00001100
out DDRE,a
ldi a,0b11100000
sts DDRG,a
.endmacro




.org 0x00
rjmp start
.org 0x100

start:

stack_init
port_init
rjmp menu

menu:
lds	gomb,PING
cpi	gomb,1
breq	red
cpi	gomb,2
breq	green
cpi	gomb,4
breq	blue
cpi 	gomb,8
breq	RB
rjmp 	menu


red:
ldi	ki,0b10000000
and	ki,ki
call	kitesz
rjmp	menu


green:
ldi	ki,0b00000100
and     ki,ki
call kitesz
rjmp	menu

blue:
ldi	ki,0b00001000
call kitesz
rjmp	menu

kitesz:
out PORTC,ki
out PORTE,ki
ret



RB:
ldi	ki, 0b10000000
ldi 	ki2,0b00001000
neg	ki
;01111111
andi	ki, 0b10000000
andi	ki2,0b00001000
call kitesz
neg ki2
rjmp RB






































