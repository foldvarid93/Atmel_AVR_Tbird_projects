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

//
unsigned char Allapot=1;
unsigned char Muvelet=0;
unsigned char SzamTomb[4];
unsigned char Op1;
unsigned char Op2;
int Eredmeny;

//billenty�zet 
unsigned char AktualisSor=0;//az aktu�lisan enged�lyezett sort tartalmazza kezdeti �rt�k legyen 0
unsigned char BillAllapot[36];//ebbe olvassa be a program a billenty�k �llapot�t  
unsigned char* BillAllapotElozo=&BillAllapot[0];//ebbe olvassa be a program a billenty�k �llapot�t
unsigned char* BillAllapotAktualis=&BillAllapot[12];//ebbe olvassa be a program a billenty�k �llapot�t
unsigned char* BillMegnyomva=&BillAllapot[24];//ebbe olvassa be a program a billenty�k �llapot�t
unsigned char Osszehasonlit=0;//�sszehasonl�t� flag 

//gombokhoz tartoz� v�ltoz�k
unsigned char gombaktualis=0;//aktu�lis gomb�ll�st t�rolja
unsigned char gombelozo=0;//az el�z� beolvas�st t�rolja
unsigned char gombmegnyomva=0;//a gombnyom�sok flagjeit t�rol� v�ltoz�

//a f�ggv�nyek prototipusai felsorol�sszer�en
void Init(void);
void led_out(unsigned char ertek);
void Init_Timer0(void);
void BillentyuBeolvasas(void);
void GombBeolvasas(void);
void felbont(int szam);

//main f�ggv�ny
int main(void)
{
	Init();//inicializ�lja a portokat
	Init_Timer0();//multiplexel�shez be�ll�tja a timer0-t
	sei();
	
	for(unsigned char i=0;i<12;i++){
		BillAllapotElozo[i]=1;//kezdeti �rt�k
	}
    /* Replace with your application code */
	while(1){
		//�llapotg�p
		if(Allapot==1){//els� operandus beolvas�sa
			led_out(0x01);//1. operandusn�l az 1. LED vil�g�t			
			BillentyuBeolvasas();//ciklikusan olvassa be a billenty�zetet
			Digitek[0]=SzamTomb[0];
			Digitek[1]=SzamTomb[1];
			Digitek[2]=0;
			Digitek[3]=0;//a sz�mokat �tadja a kijelz�nek mivel az operandusok 2 digitesek �gy a fels� 2 digit 0 
			Op1=(Digitek[1]*10)+Digitek[0];//a be�rt sz�mokat �takal�tja egy k�tjegy� sz�mm�
			GombBeolvasas();//beolvassa a gombokat (+,-,*) �s l�p �j �llapotba						
		}
		if(Allapot==2){//m�sodik operandus beolvas�sa
			led_out(0x02);//2. operandusn�l a 2. LED vil�g�t			
			BillentyuBeolvasas();//ciklikusan olvassa be a billenty�zetet
			Digitek[0]=SzamTomb[0];
			Digitek[1]=SzamTomb[1];
			Digitek[2]=0;
			Digitek[3]=0;
			Op2=(Digitek[1]*10)+Digitek[0];//a sz�mokat �tadja a kijelz�nek mivel az operandusok 2 digitesek �gy a fels� 2 digit 0 
			GombBeolvasas();//beolvassa a gombokat (=) �s l�p �j �llapotba				
		}
		if(Allapot==3){
			led_out(0x04);//eredm�nyekn�l a 3. LED vil�g�t
			if(Muvelet==0){//�sszead�s
				Eredmeny=Op1+Op2;
			}
			if(Muvelet==1){//kivon�s
				if(Op1>Op2){
					Eredmeny=Op1-Op2;	
				}
				else{
					Eredmeny=Op2-Op1;// a kivon�s k�l�nleges b�n�sm�dot ig�nyel, mindig a nagyobb�l vonja ki a kisebbet
					led_out(0x04|0x80);//de ha negat�v lenne az eredm�ny akkor a LED8-at bekapcsolja 
				}
			}
			if(Muvelet==2){//szorz�s
				Eredmeny=Op1*Op2;
			}
			felbont(Eredmeny);//az eredm�nyt �talak�tja 4 digitt� 
			while(Allapot==3){
				_delay_ms(10);
				BillentyuBeolvasas();//ameddig a 3. �llapot �ll fenn, addig ciklikusan olvassa be billenty�ket
			}
			for(unsigned char j=0;j<4;j++){
				SzamTomb[j]=0;
				Digitek[j]=0;//ha felhaszn�l�i beavatkoz�s t�rt�nt, kinull�zza az �tmeneti t�rol�kat �s a kijelz�t egy �jabb m�velet elv�gz�s�hez
			}																								
		}
	}
}
ISR (TIMER0_OVF_vect){//timer0 megszak�t�s
	//PORTA=0;//r�vid id�re kikapcsol�s hogy ne legyen "ghost effekt"
	unsigned char ertek=0;
	ertek=AktualisDigit<<4;//a megfelel� helyre shifteli a digitek jel�t
	ertek=ertek|Digitek[AktualisDigit];
	ertek=ertek|0b10000000;//enable jel hozz�vagyol�sa 
	PORTA=ertek;//�rt�k ki�r�sa
	AktualisDigit++;//aktu�lis digit n�vel�se
	if(Allapot==1||Allapot==2){//ha az operandusokat beolvas� �llapotok vannak akkor csak 2 digitet haszn�ljunk
		if(AktualisDigit>1){
			AktualisDigit=0;//ilyenkor csak a 0. �s az 1. digitet haszn�ljuk
		}
	}
	else{
		if(AktualisDigit>3){//egy�bk�nt pedig kiteszi a 4 digitet
			AktualisDigit=0;//akkor k�vetkez�leg a null�ssal folytatja, �gy lesz 0,1,2,3,0,1,2,3 ciklikus m�k�d�s
		}		
	}
	TCNT0=0xB1;//31250Hz -> 400Hz: 31250Hz/400Hz=78(0x4E) 0xFF-0x4E=0xB1 ker�l a sz�ml�l�ba												 
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
void Init_Timer0(void){//7 szegmenses kijelz� multiplex�l�s�ra haszn�lt 8 bites timer 400Hz-es megszak�t�s 8000000Hz/400Hz=20000 count -> 20000/256(el�oszt�)=78
	TCCR0|=(1<<CS02);//256 os el�oszt�s 8000000Hz/256=31250Hz
	TCNT0=0xB1;//31250Hz -> 400Hz: 31250Hz/400Hz=78(0x4E) 0xFF-0x4E=0xB1 ker�l a sz�ml�l�ba
	TIMSK |= (1 << TOIE0);//megszak�t�s enged�lyez�s
	TIFR|=(1<<TOIE0);//Timer 0 Overflow Interrupt enable
}
void BillentyuBeolvasas(void){
		for(AktualisSor=0;AktualisSor<4;AktualisSor++){//v�gigmegy a 4 soron egy for ciklussal
			if(AktualisSor==0){
				PORTC=(PINC & 0B10000111) | 0B00001000;//els� sor akt�v
			}
			if(AktualisSor==1){
				PORTC=(PINC & 0B10000111) | 0B00010000;//m�sodik sor akt�v
			}
			if(AktualisSor==2){
				PORTC=(PINC & 0B10000111) | 0B00100000;//harmadik sor akt�v
			}
			if(AktualisSor==3){
				PORTC=(PINC & 0B10000111) | 0B01000000;//negyedik sor akt�v
			}
			_delay_ms(10);//minim�lis v�rakoz�s az esetelges tranziensek (nyom�gomb kapacit�sok stb...) lezajl�s�hoz
			unsigned char tmp=PINC;//fels� bitek kimaszkol�sa
			if(tmp&0x01){//ha az els� oszlop 1-es
				BillAllapotAktualis[(AktualisSor*3)+0]=1;//els� oszlop nincs megnyomva
			}
			else{
				BillAllapotAktualis[(AktualisSor*3)+0]=0;//els� oszlop megnyomva 
			}
			if(tmp&0x02){//ha az m�sodik oszlop 1-es
				BillAllapotAktualis[(AktualisSor*3)+1]=1;//m�sodik oszlop nincs megnyomva
			}
			else{
				BillAllapotAktualis[(AktualisSor*3)+1]=0;//m�sodik oszlop megnyomva
			}
			if(tmp&0x04){//ha a harmadik oszlop 1-es
				BillAllapotAktualis[(AktualisSor*3)+2]=1;//harmadik oszlop nincs megnyomva
			}
			else{//egy�bk�nt 0
				BillAllapotAktualis[(AktualisSor*3)+2]=0;//harmadik oszlop megnyomva 
			}
			//v�gigellenz�rzi hogy t�rt�nt-e billenty� le�t�s illetve pontosabban elenged�s
			for(unsigned char i=0;i<12;i++){
				if(BillAllapotElozo[i]==0 && BillAllapotAktualis[i]==1){//elenged�st vizsg�l
					BillMegnyomva[i]=1;//be�rja hogy a gomb meg volt nyomva
				}
				else {
					BillMegnyomva[i]=0;//ellenkez� esetben null�t �r be az adott flag hely�re
				}
			}
			//ki�rt�keli a billenty�zet teljes �llapot�t
			if(BillMegnyomva[(AktualisSor*3)+2]==1 || BillMegnyomva[(AktualisSor*3)+1]==1 || BillMegnyomva[(AktualisSor*3)+0]==1){//ha az aktu�lis sorban t�rt�nt gombnyom�s m�r be is avatkozik
				for(unsigned char j=0;j<12;j++){//v�gign�zi a teljes t�mb�t	
					if(BillMegnyomva[j]==1){//az els� helyen ahol billenty� lenyom�s van bejegyezve ott meg�ll
						if(Allapot==3){
							Allapot=1;	//ha a 3. �llapotban gombnyom�s t�rt�nt, akkor �jra az els� �llapotba ugrik				
						}
						else{
							if(j==9||j==11){//* vagy # gomb lenyom�sa eset�n 																
							
							}
							else{
								SzamTomb[3]=SzamTomb[2];
								SzamTomb[2]=SzamTomb[1];
								SzamTomb[1]=SzamTomb[0];//a be�rt sz�mokat eggyel magasabb helyre teszi
								//�s az als� digit hely�re beteszi a legut�tt megnyomott sz�mot
								if(j==10){
									SzamTomb[0]=0;//ha nulla volt megnyomva akkor az ezzel a megold�ssal egy k�l�n elj�r�s
								}
								else{
									SzamTomb[0]=j+1;//minden m�s esetben a t�mbben t�rolt sorsz�m +1 egyenl� a lenyomott gombbal 
								}						
							}
						}
					}
				}
			}
			_delay_ms(2);//kis k�sleltet�s a sorok k�z�tt		
		}
		for(unsigned char i=0;i<12;i++){
			BillAllapotElozo[i]=BillAllapotAktualis[i];//a legut�bb beolvasott �rt�keket elmenti mint egy el�z� �rt�k hogy  ak�vetkez� beolvas�s az aktu�lis �rt�kek t�mbj�be ker�lj�n
		}	
}
void GombBeolvasas(void){
	gombaktualis=PING & 0x1F;//gombok beolvas�sa
	//szorz�s
	if((gombelozo & 0x01)==0 && (gombaktualis & 0x01) == 1){
		gombmegnyomva |= 0b00000001;//ha meg lett nyomva a gomb 0->1 akkor be�ll�t egy flaget egy regiszerben
	}
	if((gombmegnyomva & 0x01)==1 && (gombaktualis & 0x01)==0){
		gombmegnyomva &= 0b11111110;//ha el lett engedve 1->0 akkor kinull�zza a flaget
		if(Allapot==1){
			Allapot=2;//a k�t operandus bevitele k�z�tti �llapot (*)
			for(unsigned char j=0;j<4;j++){
				SzamTomb[j]=0;//kinull�zza a t�mb�t, az el�z� �rt�k marad�kai elt�vol�t�sa �rdek�ben
			}
			Muvelet=2;//szorz�s lesz a m�velet
		}
	}
	//kivon�s
	if((gombelozo & 0x02)==0 && (gombaktualis & 0x02) == 0x02){
		gombmegnyomva |= 0b00000010;//ha meg lett nyomva a gomb 0->1 akkor be�ll�t egy flaget egy regiszerben
	}
	if((gombmegnyomva & 0x02)==0x02 && (gombaktualis & 0x02)==0){
		gombmegnyomva &= 0b11111101;//ha el lett engedve 1->0 akkor kinull�zza a flaget
		if(Allapot==1){
			Allapot=2;//a k�t operandus bevitele k�z�tti �llapot (-)
			for(unsigned char j=0;j<4;j++){
				SzamTomb[j]=0;//kinull�zza a t�mb�t, az el�z� �rt�k marad�kai elt�vol�t�sa �rdek�ben
			}			
			Muvelet=1;//kivon�s
		}		
	}
	//�sszead�s
	if((gombelozo & 0x04)==0 && (gombaktualis & 0x04) == 0x04){
		gombmegnyomva |= 0b00000100;//ha meg lett nyomva a gomb 0->1 akkor be�ll�t egy flaget egy regiszerben
	}
	if((gombmegnyomva & 0x04)==0x04 && (gombaktualis & 0x04)==0){
		gombmegnyomva &= 0b11111011;//ha el lett engedve 1->0 akkor kinull�zza a flaget
		if(Allapot==1){
			Allapot=2;//a k�t operandus bevitele k�z�tti �llapot (+)
			for(unsigned char j=0;j<4;j++){
				SzamTomb[j]=0;//kinull�zza a t�mb�t, az el�z� �rt�k marad�kai elt�vol�t�sa �rdek�ben
			}			
			Muvelet=0;//�sszead�s
		}
	}
	//egyenl�
	if((gombelozo & 0x08)==0 && (gombaktualis & 0x08) == 0x08){
		gombmegnyomva |= 0b00001000;//ha meg lett nyomva a gomb 0->1 akkor be�ll�t egy flaget egy regiszerben
	}
	if((gombmegnyomva & 0x08)==0x08 && (gombaktualis & 0x08)==0){
		gombmegnyomva &= 0b11110111;//ha el lett engedve 1->0 akkor kinull�zza a flaget
		if(Allapot==2){
			Allapot=3;//ha 2. �llapotban bevitt�k az Op2-t akkor az egyenl� megnyom�s�ra a 3. �llapotba ugrik.
		}		
	}
	gombelozo=gombaktualis;//a jenenlegi beolvas�st elmenti a a k�vetkez� olvas�s r�gi �rt�kek�nt
}
void felbont(int szam){
	Digitek[3]=szam/1000;//ezresek
	szam%=1000;//ami az ezresek ut�n marad
	Digitek[2]=szam/100;//sz�zasok
	szam%=100;//ami a sz�zasok ut�n marad	
	Digitek[1]=szam/10;//tizesek
	szam%=10;// ami a tizesek ut�n marad	
	Digitek[0]=szam;//egyesek
}
void led_out(unsigned char ertek){
	PORTD=ertek&0xF0;
	PORTB=ertek<<4;
}