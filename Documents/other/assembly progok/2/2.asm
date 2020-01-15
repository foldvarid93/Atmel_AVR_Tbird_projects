.include "m128def.inc"
.org 0x00

.def a=r16
.def b=r17
.def c=r18
.def d=r19
.def e=r20
/// stack létrehozása-->KI kell jelölni a tetejét és alját mivel memórai terület
.macro STACK
ldi	a,		high(ramend)
out	sph,	a    
ldi a,		low(ramend)
out spl,	a
.endmacro

.macro portinit
ldi a,		0x00
sts ddrg.	a
.endmacro


rjmp start

.org 0x100


start:
ldi b,11
ldi c,125
ldi e,0

STACK

lds d,PING ///LDS-el porba "tárol"
cpi d, 1
breq elso
cpi d, 2   ///összehasonlítja egy konstanssal a D-t
breq masodik
cpi d,4
breq harmadik
cpi d,8
breq negyedik



rjmp start
elso:
add b,c
rjmp elso
masodik:
sub b,c
harmadik:

sub c,b
breq egesz
brmi marad
inc e
rjmp harmadik
egesz:
mov r21,e
rjmp start
marad:
mov r21,e
add c,b
mov r22,c

rjmp harmadik

negyedik:









