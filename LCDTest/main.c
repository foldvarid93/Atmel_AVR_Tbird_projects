/*
 * LCDTest.c
 *
 * Created: 2019. 10. 26. 20:27:22
 * Author : David
 */ 
#define F_CPU 8000000U
#include <avr/io.h>
#include <util/delay.h>
void LCD_write(unsigned char c, unsigned char d) {
	if (d==0x00){
		PORTB&=~0x01;//HAL_GPIO_WritePin(GPIOB,RS_Pin,GPIO_PIN_RESET);
	}
	else {
		PORTB|=0x01;//HAL_GPIO_WritePin(GPIOB,RS_Pin,GPIO_PIN_SET);
	}
	_delay_ms(1);
	//LCD_DATA_PORT->ODR&=0xFFFFFF00;
	PORTD=c;//LCD_DATA_PORT->ODR|=c;
	_delay_ms(1);
	PORTB|=0x04;//HAL_GPIO_WritePin(GPIOB,E_Pin,GPIO_PIN_SET);
	_delay_ms(1);
	PORTB&=~0x04;//HAL_GPIO_WritePin(GPIOB,E_Pin,GPIO_PIN_RESET);
}

void LCD_text(const char *q) {
	while (*q) {
		LCD_write(*q++,0xFF);
	}
}

void LCD_init(void) {
	PORTB&=~0x04;//HAL_GPIO_WritePin(GPIOB,E_Pin,GPIO_PIN_RESET);
	PORTB&=~0x01;//HAL_GPIO_WritePin(GPIOB,RS_Pin,GPIO_PIN_RESET);
	PORTB&=~0x02;//HAL_GPIO_WritePin(GPIOB,R_W_Pin,GPIO_PIN_RESET);

	_delay_ms(15);
	LCD_write(0x38,0x00);
	_delay_ms(1);
	LCD_write(0x38,0x00);
	_delay_ms(1);
	LCD_write(0x38,0x00);
	LCD_write(0x38,0x00);
	LCD_write(0x38,0x00);
	LCD_write(0x0F,0x00); // Make cursorinvisible
	LCD_write(0x10,0x00);
	LCD_write(0x01,0x00);//
	_delay_ms(2);
	LCD_write(0x6,0x00); // Set entry Mode(auto increment of cursor)

	LCD_write(0x01,0x00);//clear display
	_delay_ms(2);
	LCD_write(0x80,0x00);
	/*
	for(int i=0;i<26;i++){
		LCD_write(0x41+i,0xFF);
		_delay_ms(000);
	}
	LCD_write('A',0xFF);
	_delay_ms(5000);
	LCD_write(0xC0,0x00);
	LCD_write('B',0xFF);
	_delay_ms(5000);
	LCD_write(0x94,0x00);
	LCD_write('C',0xFF);
	_delay_ms(5000);
	LCD_write(0xD4,0x00);
	LCD_write('D',0xFF);
	_delay_ms(5000);
	*/
	LCD_text("Bemeno:    C");
	LCD_write(0xC0,0x00);
	LCD_text("Visszatero:    C");
	LCD_write(0x94,0x00);
	LCD_text("Bojler:    C");
	LCD_write(0xD4,0x00);
	LCD_text("Bekapcs.:    C");

	LCD_write(0x8A,0x00);
	LCD_write(0xDF,0xFF);
	LCD_write(0xCE,0x00);
	LCD_write(0xDF,0xFF);
	LCD_write(0x9E,0x00);
	LCD_write(0xDF,0xFF);
	LCD_write(0xE0,0x00);
	LCD_write(0xDF,0xFF);
}
int main(void)
{
	DDRB=0xFF;
	DDRD=0xFF;
	LCD_init();
    /* Replace with your application code */
    while (1) 
    {
    }
}

