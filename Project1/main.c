/*
 * HF.c
 *
 * Created: 2019. 10. 18. 16:05:17
 */ 
#define F_CPU 8000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//globális változók
unsigned char i=0;//a futó változó, ezzel címezzük a tömbök egyes értékeit
unsigned char j=0;//a feladatok között eltelt ütemeket számolja
unsigned char irany=0;//az oda-vissza futófényhez kell egy irány változó ami 0 ra balra 1 re jobbra léptet
unsigned char feladat=1;//itt lehet megadni melyik feladat fusson le
//gombokhoz tartozó változók
unsigned char gombuj=0;//aktuális gombállást tárolja
unsigned char gombregi=0;//az előző beolvasást tárolja		
//feladat1
unsigned char feladat1[8]={
	0B00000011,
	0B00000110,
	0B00001100,
	0B00011000,
	0B00110000,
	0B01100000,
	0B11000000,
	0B10000001
};
//
unsigned char feladat2[8]={
	0B00000001,
	0B00000011,
	0B00000111,
	0B00001111,
	0B00011110,
	0B00111100,
	0B01111000,
	0B11110000
};
//
unsigned char feladat3[4]={
	0B00011000,
	0B00100100,
	0B01000010,
	0B10000001
};
void Init(void);
void led_out(unsigned char ertek);

int main(void)
{
	Init();
    /* Replace with your application code */
	while(1){
		switch (feladat)
		{
			case 1:{//elso feladat
				led_out(feladat1[i]);//az első feladathoz tartozó tömb elemeit kiteszi a kimenetre
				i++;//növeli a futó indexet
				if(i==8){
					i=0;//ha elérte a 8 as értéket beállítja 0-ra 
				}
				break;
			}
			case 2:{//masodik feladat
				led_out(feladat2[i]);//a második feladathoz tartozó tömb elemeit teszi ki a kimenetre
				if (irany==0){//ha az irány balra(0) 
					i++;//akkor növelje 
					if(i==8){
						irany=1;//és ha elérte a 8-at akkor legyen az irány jobbra
					}
				}
				if(irany==1){//és ha az irány jobbra 
					i--;//akkor csökkentse
					if(i==0){//és ha elérte a 0-t 
						irany=0;//akkor legyen az irány balra
					}
				}				
				break;
			}
			case 3:{//harmadik feladat
				led_out(feladat3[i]);//a harmadik feladathoz tartozó tömb elemeit teszi ki a kimenetre
				if (irany==0){//ha az irány balra 
					i++;//akkor növel
					if(i==4){
						irany=1;//és ha elérte a 4-et akkor legyen az irány jobbra
					}
				}
				if(irany==1){//ha az irány jobbra
					i--;//akkor csökkent
					if(i==0){
						irany=0;//és ha elérte a nullát akkor legyen az irány balra
					}
				}				
				break;
			}
			case 4:{//negyedik feladat
				led_out(feladat3[3-i]);//ugyanaz történik mint a harmadik feladatnál csak más sorrendben kerül ki a kimenetre az adat bentről ki<>kintről be
				if (irany==0){
					i++;
					if(i==4){
						irany=1;
					}
				}
				if(irany==1){
					i--;
					if(i==0){
						irany=0;
					}
				}				
				break;
			}
			default:{
				break;
			}
		}
		_delay_ms(250);//minden lépés között 0.25s késleltetés van
		//automatikus léptetés adott ciklusszám esetén
/*		j++;//növeli a ciklusszámot
		if(feladat==1||feladat==2){//az első két feladatban 32 ütemig engedélyezi az egyes futófényeket
			if(j==32){//32 ütemig engedélyezi az egyes feladatokat
				j=0;//a következő futófénynél újra nulláról indul a számolás
				feladat++;//következő feladatra ugrás
				i=0;
				irany=0;
				if(feladat==5){//ha 5. feladat következne
					feladat=1;//akkor ugorjon vissza az elsőre
				}
			}			
		} 
		if(feladat==3||feladat==4){
			if(j==16){//16 ütemig engedélyezi az egyes feladatokat
				j=0;//a következő futófénynél újra nulláról indul a számolás
				feladat++;//következő feladatra ugrás
				i=0;
				irany=0;
				if(feladat==5){//ha 5. feladat következne
					feladat=1;//akkor ugorjon vissza az elsőre
				}
			}
		}*/
		//nyomógomb beolvasása a feladatok váltásához
		gombuj=PING&0x01;//PING.0 beolvasása
		if(gombregi==0 && gombuj==1){//ha gombnyomás feltutó él történt váltson feladatot
			feladat++;//növelje a feladat számát
			i=0;
			irany=0;//biztosítja a helyes működést a feladatok váltása közt(minden feladat az elejéről indul)
			if(feladat==5){
				feladat=1;//ha túlindexelne akkor ugorjon vissza az elsőre
			}
		}
		gombregi=gombuj;//a jenenlegi beolvasást elmenti a a következő olvasás régi értékeként		
		//feladat sorszámának kiírása
		PORTA=0B10000000|feladat;			
	}
}
void Init(void){
	//LED kimenetek inicializálása
	DDRB=0xF0;//portB felső 4 bit kimenet LED3|LED2|LED1|LED0|MISO|MOSI|SCK|SS|
	DDRD=0xF0;//portD felső 4 bit kimenet LED7|LED6|LED5|LED4|TXD1-INT3|RXD1-INT2|SDA-INT1|SCL-INT0|
	DDRA=0xFF;//portA kimenet
	PORTG=0x00;//portG bemenet
}
void led_out(unsigned char ertek){
	PORTD=ertek&0xF0;
	PORTB=ertek<<4;
}