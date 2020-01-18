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

//�ra v�ltoz�i
unsigned char Masodperc=40;//ez egy�rtelm�
unsigned char Perc=59;//ez egy�rtelm�
unsigned char Ora=23;//ez egy�rtelm�
unsigned char Kettospont=0;//0 vagy 1 annak megfelel�en hogy kell e vil�g�tani a kett�spontnak vagy nem minden m�sodpercben �llapotot v�lt
unsigned char Temp=0;//�tmeneti t�rol�

//gombokhoz tartoz� v�ltoz�k
unsigned char gombaktualis=0;//aktu�lis gomb�ll�st t�rolja
unsigned char gombelozo=0;//az el�z� beolvas�st t�rolja
unsigned char gombmegnyomva=0;//a gombnyom�sok flagjeit t�rol� v�ltoz�

//a f�ggv�nyek prototipusai felsorol�sszer�en
void Init(void);
void led_out(unsigned char ertek);
void Init_Timer0(void);
void Init_Timer1(void);
void felbont(unsigned char Ora, unsigned char Perc);

//main f�ggv�ny
int main(void)
{
	Init();//inicializ�lja a portokat
	Init_Timer0();//multiplexel�shez be�ll�tja a timer0-t
	Init_Timer1();//m�sodpercek sz�ml�l�s�hoz be�ll�tja a timer1-t	
	sei();//interruptok engef�lyez�se
	
	while(1){
		//az id� be�ll�t�sa gombokkal
		gombaktualis=PING & 0x1F;//gombok beolvas�sa
		//Perc le	
		if((gombelozo & 0x01)==0 && (gombaktualis & 0x01) == 1){
			gombmegnyomva |= 0b00000001;//ha meg lett nyomva a gomb 0->1 akkor be�ll�t egy flaget egy regiszerben
		}
		if((gombmegnyomva & 0x01)==1 && (gombaktualis & 0x01)==0){
			gombmegnyomva &= 0b11111110;//ha el lett engedve 1->0 akkor kinull�zza a flaget
			if (Perc==0){
				Perc=59;//eggyel cs�kkenti a perc regiszter�t, figyelve az �tfordul�sra
			}
			else{
				Perc--;
			}
			felbont(Ora,Perc);//elv�gzi az �jonnan be�rt �rt�k felbont�s�t, hogy j�l jelenjen meg a kijelz�n
			Masodperc=0;//minden �ll�t�s ut�n eg�sz percr�l induljon az �ra
		}
		//perc fel
		if((gombelozo & 0x02)==0 && (gombaktualis & 0x02) == 0x02){
			gombmegnyomva |= 0b00000010;//ha meg lett nyomva a gomb 0->1 akkor be�ll�t egy flaget egy regiszerben
		}
		if((gombmegnyomva & 0x02)==0x02 && (gombaktualis & 0x02)==0){
			gombmegnyomva &= 0b11111101;//ha el lett engedve 1->0 akkor kinull�zza a flaget
			if (Perc==59){
				Perc=0;//eggyel n�veli a perc regiszter�t, figyelve az �tfordul�sra
			}
			else{
				Perc++;
			}
			felbont(Ora,Perc);//elv�gzi az �jonnan be�rt �rt�k felbont�s�t, hogy j�l jelenjen meg a kijelz�n
			Masodperc=0;//minden �ll�t�s ut�n eg�sz percr�l induljon az �ra
		}
		//�ra le
		if((gombelozo & 0x04)==0 && (gombaktualis & 0x04) == 0x04){
			gombmegnyomva |= 0b00000100;//ha meg lett nyomva a gomb 0->1 akkor be�ll�t egy flaget egy regiszerben
		}
		if((gombmegnyomva & 0x04)==0x04 && (gombaktualis & 0x04)==0){
			gombmegnyomva &= 0b11111011;//ha el lett engedve 1->0 akkor kinull�zza a flaget
			if (Ora==0){
				Ora=23;
			}
			else{
				Ora--;//eggyel cs�kkenti az �ra regiszter�t, figyelve az �tfordul�sra
			}
			felbont(Ora,Perc);//elv�gzi az �jonnan be�rt �rt�k felbont�s�t, hogy j�l jelenjen meg a kijelz�n
			Masodperc=0;//minden �ll�t�s ut�n eg�sz percr�l induljon az �ra
		}
		//�ra fel
		if((gombelozo & 0x08)==0 && (gombaktualis & 0x08) == 0x08){
			gombmegnyomva |= 0b00001000;//ha meg lett nyomva a gomb 0->1 akkor be�ll�t egy flaget egy regiszerben
		}
		if((gombmegnyomva & 0x08)==0x08 && (gombaktualis & 0x08)==0){
			gombmegnyomva &= 0b11110111;//ha el lett engedve 1->0 akkor kinull�zza a flaget
			if (Ora==23){
				Ora=0;
			}
			else{
				Ora++;//eggyel n�veli az �ra regiszter�t, figyelve az �tfordul�sra
			}
			felbont(Ora,Perc);//elv�gzi az �jonnan be�rt �rt�k felbont�s�t, hogy j�l jelenjen meg a kijelz�n
			Masodperc=0;//minden �ll�t�s ut�n eg�sz percr�l induljon az �ra
		}		
		gombelozo=gombaktualis;//a jenenlegi beolvas�st elmenti a a k�vetkez� olvas�s r�gi �rt�kek�nt
		_delay_ms(20);//a beolvas�sok k�z�tt kis k�sleltet�s		
	}
}
ISR (TIMER0_OVF_vect){//timer0 megszak�t�s
	PORTA=0x0;//r�vid id�re kikapcsol�s hogy ne legyen "ghost effekt"
	
	if(AktualisDigit<4){
		unsigned char ertek=0;
		ertek=AktualisDigit<<4;//a megfelel� helyre shifteli a digitek jel�t
		ertek=ertek|Digitek[AktualisDigit];
		ertek=ertek|0b10000000;//enable jel hozz�vagyol�sa
		PORTA|=ertek;//�rt�k ki�r�sa		
	}
	if(AktualisDigit==4 && Kettospont){
		PORTA=0b11000000;
	}
	AktualisDigit++;//aktu�lis digit n�vel�se
	
	if(AktualisDigit==5){//ha n�vel�s ut�n 4-gye egyenl� 
		AktualisDigit=0;//akkor k�vetkez�leg a null�ssal folytatja, �gy lesz 0,1,2,3,0,1,2,3 ciklikus m�k�d�s
	}
	led_out(Masodperc);
	TCNT0=0xB1;//a sz�ml�l� �jrat�lt�se													 
}
ISR (TIMER1_OVF_vect){
	Masodperc++;//m�sodperc n�vel�se
	if(Masodperc==60){//ha a m�sodperc el�rte a 60 at
		Masodperc=0;//ha el�rte a 60-at akkor m�sodperc null�z�sa
		Perc++;//�s perc n�vel�se
		if(Perc==60){//ha a perc el�rte a 60-at
			Perc=0;//akkor perc null�z�sa
			Ora++;//�s �ra n�vel�se
			if(Ora==24){//ha az �ra el�rte a 24 et
				Ora=0;//�ra null�z�sa
			}
		}
	}
	felbont(Ora,Perc);
	if(Kettospont==1){
		Kettospont=0;// ha az el�z� m�sodpercben vil�g�tott a kett�spont akkor kapcsolja ki
	}
	else{
		Kettospont=1;//egy�bk�nt kapcsolja be
	}	
	TCNT1H=0x85;//
	TCNT1L=0xED;//�jrat�lti az 1s-hoz tartoz� regiszter �rt�keket 
}
void Init(void){
	//LED kimenetek inicializ�l�sa
	DDRB=0xF0;//portB fels� 4 bit kimenet LED3|LED2|LED1|LED0|MISO|MOSI|SCK|SS|
	DDRD=0xF0;//portD fels� 4 bit kimenet LED7|LED6|LED5|LED4|TXD1-INT3|RXD1-INT2|SDA-INT1|SCL-INT0|
	DDRC=0xF8;//portC fels� 5 bit kimenet RGBLED|ROWD|ROWC|ROWB|ROWA|COLC|COLB|COLA|
	DDRE|=0x0C;//portE ledek be�ll�t�sa 
	//7 szegmenses kijelz� inicializ�l�sa
	DDRA=0xFF;//portA kimenet 
	PORTG=0x00;//portG bemenet
}
void led_out(unsigned char ertek){
	PORTD=ertek&0xF0;
	PORTB=ertek<<4;
}
void felbont(unsigned char Ora, unsigned char Perc){
	Digitek[3]=Ora/10;//�ra tizesei
	Digitek[2]=Ora%10;//a tizeseken fel�li marad�k az �ra egyesei
	Digitek[1]=Perc/10;//a percek tizesei
	Digitek[0]=Perc%10;//a tizeseken fel�li marad�ka percek egyesei
}
void Init_Timer0(void){//7 szegmenses kijelz� multiplex�l�s�ra haszn�lt 8 bites timer 400Hz-es megszak�t�s 8000000Hz/400Hz=20000 count -> 20000/256(el�oszt�)=78
	TCCR0|=(1<<CS02);//256 os el�oszt�s 8000000Hz/256=31250Hz
	TCNT0=0xB1;//31250Hz -> 400Hz: 31250Hz/400Hz=78(0x4E) 0xFF-0x4E=0xB1 ker�l a sz�ml�l�ba
	TIMSK |= (1 << TOIE0);//megszak�t�s enged�lyez�s
	TIFR|=(1<<TOIE0);//Timer 0 Overflow Interrupt enable
}
void Init_Timer1(void){//a m�sodpercek sz�mol�s�ra haszn�lt 16 bites timer 1Hz-es megszak�t�s 8000000Hz/1Hz=8000000 count -> 8000000/256(el�oszt�)=31250
	TCCR1B|=(1<<CS12);//CS11 1 be �ll�t�sa 8000000Hz/256=31250Hz 
	TCNT1H=0x85;//az ide be�rt sz�mt�l kezd 0xFFFF fel� sz�molni, teh�t ha 31320ciklust akarunk sz�molni, akkor 0xFFFF-0x7A12(31250)=0x85ED kell a 16 bites regiszterbe ide ker�l a fels� byte
	TCNT1L=0xED;//�s ide az als�
	TIMSK|=(1<<TOIE1);//Timer 1 Overflow Interrupt enable
	TIFR|=(1<<TOV1);//Timer1 overflow flag automatikus t�rl�se ha kiszolg�l�sra ker�l	
}