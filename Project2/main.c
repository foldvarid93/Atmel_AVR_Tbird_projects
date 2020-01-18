/*
 * HF.c
 *
 * Created: 2019. 10. 18. 16:05:17
 */ 
#define F_CPU 8000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//ki�r�shoz sz�ks�ges glob�lis v�ltoz�k
unsigned char AktualisDigit=0;//egy fut�index 0 �s 4 k�z�tt aktu�lis digitet mondja meg hogy �pp melyik kijelz� akt�v a 4b�l
unsigned char Digitek[4];// az egyes digitek sz�mait itt t�rolom el 	
unsigned char i=0;//a fut� v�ltoz�, ezzel c�mezz�k a t�mb�k egyes �rt�keit
unsigned char irany=0;//az oda-vissza fut�f�nyhez kell egy ir�ny v�ltoz� ami 0 ra balra 1 re jobbra l�ptet
unsigned char feladat=0;//az �ppen fut� feladat sors�m�t t�rolja
unsigned int szamlalo;//itt t�rolom a sz�ml�l� aktu�lis �rt�k�t
unsigned int atmenetitarolo;//hogy ne rontsa el a felbont�s az eredeti sz�mot

//gombokhoz tartoz� v�ltoz�k
unsigned char gombuj=0;//aktu�lis gomb�ll�st t�rolja
unsigned char gombregi=0;//az el�z� beolvas�st t�rolja

//a feladatok bemeneti param�terei
unsigned int szam=5555;//ebben t�rolom az 5. feladathoz tartoz� ki�rni k�v�nt int �rt�k�t 0 < szam < 9999
unsigned int szamlalomax=4444;//maximum �rt�k amin�l a sz�ml�l� ir�nyt v�lt szamlalomin < szamlalomax < 9999
unsigned int szamlalomin=1111;//minimum �rt�k amin�l a sz�ml�l� ir�nyt v�lt 0 < szamlalomin < szamlalomax

//a f�ggv�nyek prototipusai felsorol�sszer�en
void Init(void);
void led_out(unsigned char ertek);
void Init_Timer0(void);

//main f�ggv�ny
int main(void)
{
	Init();//inicializ�lja a portokat
	Init_Timer0();//multiplexel�shez be�ll�tja a timer0-t
	sei();
	szamlalo=szamlalomin;
    /* Replace with your application code */
	while(1){
		switch (feladat)
		{
			case 0:{//5. feladat
				atmenetitarolo=szam;
				led_out(0x01);//els� ledet bekapcsolja-> els� feladat megy �ppen							
				break;
			}
			case 1:{//6. feladat
				atmenetitarolo=szamlalo;
				led_out(0x02);//m�sodik ledet bekapcsolja-> m�sodik feladat megy �ppen
				break;
			}
		}
		//sz�m felbont�sa a k�l�nb�z� digitekre
		Digitek[3]=atmenetitarolo/1000;//ezresek
		atmenetitarolo%=1000;//ami az ezresek levon�sa ut�n marad
		Digitek[2]=atmenetitarolo/100;//sz�zasok
		atmenetitarolo%=100;//ami az sz�zasok levon�sa ut�n marad
		Digitek[1]=atmenetitarolo/10;//tizesek
		atmenetitarolo%=10;//ami az ezresek levon�sa ut�n marad
		Digitek[0]=atmenetitarolo;//ami maradt
		
		//nyom�gomb beolvas�sa a feladatok v�lt�s�hoz
		gombuj=PING&0x01;//PING.0 beolvas�sa
		if(gombregi==0 && gombuj==1){//ha gombnyom�s feltut� �l t�rt�nt v�ltson feladatot
			if(feladat==1){
				feladat=0;//ha a m�sodik ment v�ltson az els�re
			}
			else{
				feladat=1;//ha az els� feladat ment, v�ltson a m�sodikra
			}
		}
		gombregi=gombuj;//a jenenlegi beolvas�st elmenti a a k�vetkez� olvas�s r�gi �rt�kek�nt 
		//	
		//_delay_ms(10);//sz�ml�l�sok k�zti delay 10ms csak tesztel�sre 
		_delay_ms(300);//sz�ml�l�sok k�zti delay
		//
		if (irany==0){//felfel� sz�ml�l�s
			szamlalo++;//n�velje a sz�ml�l� �rt�k�t
			if(szamlalo==szamlalomax){
				irany=1;//ha el�rte a max �rt�ket akkor v�ltson ir�nyt
			}
		}
		if (irany==1){//ha lefel� sz�mol
			szamlalo--;//cs�kkentse a v�ltoz�t 
			if(szamlalo==szamlalomin){
				irany=0;//ha el�rte a minimumot v�ltson ir�nyt
			}
		}		
	}
}
ISR (TIMER0_OVF_vect){//timer0 megszak�t�s
	PORTA=0;//r�vid id�re kikapcsol�s hogy ne legyen "ghost effekt"
	unsigned char ertek=0;
	ertek=AktualisDigit<<4;//a megfelel� helyre shifteli a digitek jel�t
	ertek=ertek|Digitek[AktualisDigit];
	ertek=ertek|0b10000000;//enable jel hozz�vagyol�sa 
	PORTA=ertek;//�rt�k ki�r�sa
	AktualisDigit++;//aktu�lis digit n�vel�se
	if(AktualisDigit==4){//ha n�vel�s ut�n 4-gye egyenl� 
		AktualisDigit=0;//akkor k�vetkez�leg a null�ssal folytatja, �gy lesz 0,1,2,3,0,1,2,3 ciklikus m�k�d�s
	}
}
void Init(void){
	//LED kimenetek inicializ�l�sa
	DDRB=0xF0;//portB fels� 4 bit kimenet LED3|LED2|LED1|LED0|MISO|MOSI|SCK|SS|
	DDRD=0xF0;//portD fels� 4 bit kimenet LED7|LED6|LED5|LED4|TXD1-INT3|RXD1-INT2|SDA-INT1|SCL-INT0|
	//7 szegmenses kijelz� inicializ�l�sa
	DDRA=0xFF;//portA kimenet 
	PORTG=0x00;//portG bemenet
}
void led_out(unsigned char ertek){
	PORTD=ertek&0xF0;
	PORTB=ertek<<4;
}
void Init_Timer0(void){//7 szegmenses kijelz� multiplex�l�s�ra haszn�lt 8 bites timer
	TCCR0|=0B00000101;//128 as el�oszt�s Fclk/128 ez lesz a megszak�t�s �teme
	TIMSK |= (1 << TOIE0);//megszak�t�s enged�lyez�s
}