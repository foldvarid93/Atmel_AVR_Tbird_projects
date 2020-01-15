#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

void init();


void init()
	{
		DDRB=0xF0;
		DDRD=0xF0;
		DDRG=0;
		DDRE=0b00000100;
	}

int main()
{
	int n=0;
	init();
	while(1)
	{
PORTE=8;

_delay_ms(20);
n++;
PORTE=0;
_delay_ms(20);
	}
	
	
		
	return 0;
}




