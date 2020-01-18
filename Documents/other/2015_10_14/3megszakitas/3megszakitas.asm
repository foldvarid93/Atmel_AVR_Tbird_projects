.include "m128def.inc"
.def tmp=r16
.def leds=r17
.def buttons=r18

.macro ShowLeds
mov tmp,leds
swap tmp
andi tmp,0b11110000
out PORTB,tmp

mov  tmp,leds
andi tmp,0b11110000
out ddrd,tmp
.endmacro





.org 0x00
		jmp RESET;reset handler
.org 0x1e
		jmp TIM0_CMP;

.org 0x20
		jmp TIM0_OVF; Timer0 Overflow Handler;minden overflow interruptn�l lefut ez!
RESET:
;led 0-3
in tmp,ddrb
ori tmp,0xf0
out ddrb, tmp
;led 4-7
in tmp,ddrd
ori tmp,0xf0
out ddrd, tmp


;stack
ldi tmp,HIGH(RAMEND)
out SPH,tmp
ldi tmp,LOW(RAMEND)
out SPL,tmp

;Timer0 init
ldi tmp,0b0000_0111;on prescaller 1024-enk�nt n�v.�s k�ldi az INTERRUPT bitet
out TCCR0,tmp ;bekapcsoltuk a timert

ldi tmp,10;itt �l�tom az overflowhoz k�pesti abszol�t t�vols�got(l�pcs�k).OVF=255, ha ez 25 kb.10%os kit�lt�s�
out OCR0,tmp

ldi tmp,0b0000_0011;xx1!-overflow interrupt bekapcs..x1x! compair inter.is elindul
out TIMSK,tmp;milyen helyzetben szeretn�k interruptot kapni a timert�l

sei;enged�lyezem a glob�lis interrupt k�ld�st


rjmp main

main:



	rjmp main

TIM0_OVF:
ldi leds,0xff
ShowLeds
reti
TIM0_CMP:
ldi leds,0x00
ShowLeds
reti
