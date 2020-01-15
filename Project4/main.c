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

//óra változói
unsigned char Masodperc=40;//ez egyértelmû
unsigned char Perc=59;//ez egyértelmû
unsigned char Ora=23;//ez egyértelmû
unsigned char Kettospont=0;//0 vagy 1 annak megfelelõen hogy kell e világítani a kettõspontnak vagy nem minden másodpercben állapotot vált
unsigned char Temp=0;//átmeneti tároló

//gombokhoz tartozó változók
unsigned char gombaktualis=0;//aktuális gombállást tárolja
unsigned char gombelozo=0;//az elõzõ beolvasást tárolja
unsigned char gombmegnyomva=0;//a gombnyomások flagjeit tároló változó

//a függvények prototipusai felsorolásszerûen
void Init(void);
void led_out(unsigned char ertek);
void Init_Timer0(void);
void Init_Timer1(void);
void felbont(unsigned char Ora, unsigned char Perc);

//main függvény
int main(void)
{
	Init();//inicializálja a portokat
	Init_Timer0();//multiplexeléshez beállítja a timer0-t
	Init_Timer1();//másodpercek számlálásához beállítja a timer1-t	
	sei();//interruptok engefélyezése
	
	while(1){
		//az idõ beállítása gombokkal
		gombaktualis=PING & 0x1F;//gombok beolvasása
		//Perc le	
		if((gombelozo & 0x01)==0 && (gombaktualis & 0x01) == 1){
			gombmegnyomva |= 0b00000001;//ha meg lett nyomva a gomb 0->1 akkor beállít egy flaget egy regiszerben
		}
		if((gombmegnyomva & 0x01)==1 && (gombaktualis & 0x01)==0){
			gombmegnyomva &= 0b11111110;//ha el lett engedve 1->0 akkor kinullázza a flaget
			if (Perc==0){
				Perc=59;//eggyel csökkenti a perc regiszterét, figyelve az átfordulásra
			}
			else{
				Perc--;
			}
			felbont(Ora,Perc);//elvégzi az újonnan beírt érték felbontását, hogy jól jelenjen meg a kijelzõn
			Masodperc=0;//minden állítás után egész percrõl induljon az óra
		}
		//perc fel
		if((gombelozo & 0x02)==0 && (gombaktualis & 0x02) == 0x02){
			gombmegnyomva |= 0b00000010;//ha meg lett nyomva a gomb 0->1 akkor beállít egy flaget egy regiszerben
		}
		if((gombmegnyomva & 0x02)==0x02 && (gombaktualis & 0x02)==0){
			gombmegnyomva &= 0b11111101;//ha el lett engedve 1->0 akkor kinullázza a flaget
			if (Perc==59){
				Perc=0;//eggyel növeli a perc regiszterét, figyelve az átfordulásra
			}
			else{
				Perc++;
			}
			felbont(Ora,Perc);//elvégzi az újonnan beírt érték felbontását, hogy jól jelenjen meg a kijelzõn
			Masodperc=0;//minden állítás után egész percrõl induljon az óra
		}
		//Óra le
		if((gombelozo & 0x04)==0 && (gombaktualis & 0x04) == 0x04){
			gombmegnyomva |= 0b00000100;//ha meg lett nyomva a gomb 0->1 akkor beállít egy flaget egy regiszerben
		}
		if((gombmegnyomva & 0x04)==0x04 && (gombaktualis & 0x04)==0){
			gombmegnyomva &= 0b11111011;//ha el lett engedve 1->0 akkor kinullázza a flaget
			if (Ora==0){
				Ora=23;
			}
			else{
				Ora--;//eggyel csökkenti az óra regiszterét, figyelve az átfordulásra
			}
			felbont(Ora,Perc);//elvégzi az újonnan beírt érték felbontását, hogy jól jelenjen meg a kijelzõn
			Masodperc=0;//minden állítás után egész percrõl induljon az óra
		}
		//Óra fel
		if((gombelozo & 0x08)==0 && (gombaktualis & 0x08) == 0x08){
			gombmegnyomva |= 0b00001000;//ha meg lett nyomva a gomb 0->1 akkor beállít egy flaget egy regiszerben
		}
		if((gombmegnyomva & 0x08)==0x08 && (gombaktualis & 0x08)==0){
			gombmegnyomva &= 0b11110111;//ha el lett engedve 1->0 akkor kinullázza a flaget
			if (Ora==23){
				Ora=0;
			}
			else{
				Ora++;//eggyel növeli az óra regiszterét, figyelve az átfordulásra
			}
			felbont(Ora,Perc);//elvégzi az újonnan beírt érték felbontását, hogy jól jelenjen meg a kijelzõn
			Masodperc=0;//minden állítás után egész percrõl induljon az óra
		}		
		gombelozo=gombaktualis;//a jenenlegi beolvasást elmenti a a következõ olvasás régi értékeként
		_delay_ms(20);//a beolvasások között kis késleltetés		
	}
}
ISR (TIMER0_OVF_vect){//timer0 megszakítás
	PORTA=0x0;//rövid idõre kikapcsolás hogy ne legyen "ghost effekt"
	
	if(AktualisDigit<4){
		unsigned char ertek=0;
		ertek=AktualisDigit<<4;//a megfelelõ helyre shifteli a digitek jelét
		ertek=ertek|Digitek[AktualisDigit];
		ertek=ertek|0b10000000;//enable jel hozzávagyolása
		PORTA|=ertek;//érték kiírása		
	}
	if(AktualisDigit==4 && Kettospont){
		PORTA=0b11000000;
	}
	AktualisDigit++;//aktuális digit növelése
	
	if(AktualisDigit==5){//ha növelés után 4-gye egyenlõ 
		AktualisDigit=0;//akkor következõleg a nullással folytatja, így lesz 0,1,2,3,0,1,2,3 ciklikus mûködés
	}
	led_out(Masodperc);
	TCNT0=0xB1;//a számláló újratöltése													 
}
ISR (TIMER1_OVF_vect){
	Masodperc++;//másodperc növelése
	if(Masodperc==60){//ha a másodperc elérte a 60 at
		Masodperc=0;//ha elérte a 60-at akkor másodperc nullázása
		Perc++;//és perc növelése
		if(Perc==60){//ha a perc elérte a 60-at
			Perc=0;//akkor perc nullázása
			Ora++;//és óra növelése
			if(Ora==24){//ha az óra elérte a 24 et
				Ora=0;//óra nullázása
			}
		}
	}
	felbont(Ora,Perc);
	if(Kettospont==1){
		Kettospont=0;// ha az elõzõ másodpercben világított a kettõspont akkor kapcsolja ki
	}
	else{
		Kettospont=1;//egyébként kapcsolja be
	}	
	TCNT1H=0x85;//
	TCNT1L=0xED;//Újratölti az 1s-hoz tartozó regiszter értékeket 
}
void Init(void){
	//LED kimenetek inicializálása
	DDRB=0xF0;//portB felsõ 4 bit kimenet LED3|LED2|LED1|LED0|MISO|MOSI|SCK|SS|
	DDRD=0xF0;//portD felsõ 4 bit kimenet LED7|LED6|LED5|LED4|TXD1-INT3|RXD1-INT2|SDA-INT1|SCL-INT0|
	DDRC=0xF8;//portC felsõ 5 bit kimenet RGBLED|ROWD|ROWC|ROWB|ROWA|COLC|COLB|COLA|
	DDRE|=0x0C;//portE ledek beállítása 
	//7 szegmenses kijelzõ inicializálása
	DDRA=0xFF;//portA kimenet 
	PORTG=0x00;//portG bemenet
}
void led_out(unsigned char ertek){
	PORTD=ertek&0xF0;
	PORTB=ertek<<4;
}
void felbont(unsigned char Ora, unsigned char Perc){
	Digitek[3]=Ora/10;//óra tizesei
	Digitek[2]=Ora%10;//a tizeseken felüli maradék az óra egyesei
	Digitek[1]=Perc/10;//a percek tizesei
	Digitek[0]=Perc%10;//a tizeseken felüli maradéka percek egyesei
}
void Init_Timer0(void){//7 szegmenses kijelzõ multiplexálására használt 8 bites timer 400Hz-es megszakítás 8000000Hz/400Hz=20000 count -> 20000/256(elõosztó)=78
	TCCR0|=(1<<CS02);//256 os elõosztás 8000000Hz/256=31250Hz
	TCNT0=0xB1;//31250Hz -> 400Hz: 31250Hz/400Hz=78(0x4E) 0xFF-0x4E=0xB1 kerül a számlálóba
	TIMSK |= (1 << TOIE0);//megszakítás engedélyezés
	TIFR|=(1<<TOIE0);//Timer 0 Overflow Interrupt enable
}
void Init_Timer1(void){//a másodpercek számolására használt 16 bites timer 1Hz-es megszakítás 8000000Hz/1Hz=8000000 count -> 8000000/256(elõosztó)=31250
	TCCR1B|=(1<<CS12);//CS11 1 be állítása 8000000Hz/256=31250Hz 
	TCNT1H=0x85;//az ide beírt számtól kezd 0xFFFF felé számolni, tehát ha 31320ciklust akarunk számolni, akkor 0xFFFF-0x7A12(31250)=0x85ED kell a 16 bites regiszterbe ide kerül a felsõ byte
	TCNT1L=0xED;//és ide az alsó
	TIMSK|=(1<<TOIE1);//Timer 1 Overflow Interrupt enable
	TIFR|=(1<<TOV1);//Timer1 overflow flag automatikus törlése ha kiszolgálásra kerül	
}