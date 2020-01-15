/*
 * HF.c
 *
 * Created: 2019. 10. 18. 16:05:17
 */ 
#define F_CPU 8000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//kiíráshoz szükséges globális változók
unsigned char AktualisDigit=0;//egy futóindex 0 és 4 között aktuális digitet mondja meg hogy épp melyik kijelzõ aktív a 4bõl
unsigned char Digitek[4];// az egyes digitek számait itt tárolom el 	
unsigned char i=0;//a futó változó, ezzel címezzük a tömbök egyes értékeit
unsigned char irany=0;//az oda-vissza futófényhez kell egy irány változó ami 0 ra balra 1 re jobbra léptet
unsigned char feladat=0;//az éppen futó feladat sorsámát tárolja
unsigned int szamlalo;//itt tárolom a számláló aktuális értékét
unsigned int atmenetitarolo;//hogy ne rontsa el a felbontás az eredeti számot

//gombokhoz tartozó változók
unsigned char gombuj=0;//aktuális gombállást tárolja
unsigned char gombregi=0;//az elõzõ beolvasást tárolja

//a feladatok bemeneti paraméterei
unsigned int szam=5555;//ebben tárolom az 5. feladathoz tartozó kiírni kívánt int értékét 0 < szam < 9999
unsigned int szamlalomax=4444;//maximum érték aminél a számláló irányt vált szamlalomin < szamlalomax < 9999
unsigned int szamlalomin=1111;//minimum érték aminél a számláló irányt vált 0 < szamlalomin < szamlalomax

//a függvények prototipusai felsorolásszerûen
void Init(void);
void led_out(unsigned char ertek);
void Init_Timer0(void);

//main függvény
int main(void)
{
	Init();//inicializálja a portokat
	Init_Timer0();//multiplexeléshez beállítja a timer0-t
	sei();
	szamlalo=szamlalomin;
    /* Replace with your application code */
	while(1){
		switch (feladat)
		{
			case 0:{//5. feladat
				atmenetitarolo=szam;
				led_out(0x01);//elsõ ledet bekapcsolja-> elsõ feladat megy éppen							
				break;
			}
			case 1:{//6. feladat
				atmenetitarolo=szamlalo;
				led_out(0x02);//második ledet bekapcsolja-> második feladat megy éppen
				break;
			}
		}
		//szám felbontása a különbözõ digitekre
		Digitek[3]=atmenetitarolo/1000;//ezresek
		atmenetitarolo%=1000;//ami az ezresek levonása után marad
		Digitek[2]=atmenetitarolo/100;//százasok
		atmenetitarolo%=100;//ami az százasok levonása után marad
		Digitek[1]=atmenetitarolo/10;//tizesek
		atmenetitarolo%=10;//ami az ezresek levonása után marad
		Digitek[0]=atmenetitarolo;//ami maradt
		
		//nyomógomb beolvasása a feladatok váltásához
		gombuj=PING&0x01;//PING.0 beolvasása
		if(gombregi==0 && gombuj==1){//ha gombnyomás feltutó él történt váltson feladatot
			if(feladat==1){
				feladat=0;//ha a második ment váltson az elsõre
			}
			else{
				feladat=1;//ha az elsõ feladat ment, váltson a másodikra
			}
		}
		gombregi=gombuj;//a jenenlegi beolvasást elmenti a a következõ olvasás régi értékeként 
		//	
		//_delay_ms(10);//számlálások közti delay 10ms csak tesztelésre 
		_delay_ms(300);//számlálások közti delay
		//
		if (irany==0){//felfelé számlálás
			szamlalo++;//növelje a számláló értékét
			if(szamlalo==szamlalomax){
				irany=1;//ha elérte a max értéket akkor váltson irányt
			}
		}
		if (irany==1){//ha lefelé számol
			szamlalo--;//csökkentse a változót 
			if(szamlalo==szamlalomin){
				irany=0;//ha elérte a minimumot váltson irányt
			}
		}		
	}
}
ISR (TIMER0_OVF_vect){//timer0 megszakítás
	PORTA=0;//rövid idõre kikapcsolás hogy ne legyen "ghost effekt"
	unsigned char ertek=0;
	ertek=AktualisDigit<<4;//a megfelelõ helyre shifteli a digitek jelét
	ertek=ertek|Digitek[AktualisDigit];
	ertek=ertek|0b10000000;//enable jel hozzávagyolása 
	PORTA=ertek;//érték kiírása
	AktualisDigit++;//aktuális digit növelése
	if(AktualisDigit==4){//ha növelés után 4-gye egyenlõ 
		AktualisDigit=0;//akkor következõleg a nullással folytatja, így lesz 0,1,2,3,0,1,2,3 ciklikus mûködés
	}
}
void Init(void){
	//LED kimenetek inicializálása
	DDRB=0xF0;//portB felsõ 4 bit kimenet LED3|LED2|LED1|LED0|MISO|MOSI|SCK|SS|
	DDRD=0xF0;//portD felsõ 4 bit kimenet LED7|LED6|LED5|LED4|TXD1-INT3|RXD1-INT2|SDA-INT1|SCL-INT0|
	//7 szegmenses kijelzõ inicializálása
	DDRA=0xFF;//portA kimenet 
	PORTG=0x00;//portG bemenet
}
void led_out(unsigned char ertek){
	PORTD=ertek&0xF0;
	PORTB=ertek<<4;
}
void Init_Timer0(void){//7 szegmenses kijelzõ multiplexálására használt 8 bites timer
	TCCR0|=0B00000101;//128 as elõosztás Fclk/128 ez lesz a megszakítás üteme
	TIMSK |= (1 << TOIE0);//megszakítás engedélyezés
}