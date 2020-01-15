#include <avr/io.h>
#define F_CPU 1600000000UL
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

void temp(*unsigned char );
void konstans(*unsigned char );

void port_init()
	{
		DDRB=0xF0;
		DDRD=0xF0;
		DDRG=0;
	}



int main()
	{
	
	port_init();
	led();


	}

void led()
{
unsigned char tmp=1;
unsigned char led=128;

if(PING==1)
	{

	while(1)
		{
		//	konstans(led);
		//	_delay_ms(100);
		//	temp(tmp);
		//	_delay_ms(100);
		}
	}




}
/*
void konstans(*unsigned char )
{

		PORTD=led;
		//led=led<<4
		PORTB=led<<4;
}
/*
void temp(*unsigned char )
{
	PORTD=led;
	//led=led<<4;
	PORTB=led<<4;



}





*/





