#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#define del (10)

void init();
void hetseg(unsigned char ki);

void init()
	{
		
		DDRG=	0;
		DDRA=	0b11111111;
		DDRC=	0x78;
	}

int main()
{	
	unsigned char i=0;
	unsigned char tomb[10]={66,9,10,12,17,18,20,33,34,36};
	init();
	int sor=0b00001000,vissza=0,elozo=0;
		
	while(1)
	{
		if(sor==64)	sor=8;
		PORTC=sor;
		_delay_ms(5);
		vissza=((~PINC&0b00000111)|sor);
		if(vissza==tomb[i])
			{
				hetseg(i);
			}
			
			i++;
			if(i>=10) i=0;
			sor<<=1;
		
	}
	
		
	//void hethes();	


return 0;
}

void hetseg(unsigned char ki)
{
	PORTA=0b10110000|ki;
	_delay_ms(del);

}
