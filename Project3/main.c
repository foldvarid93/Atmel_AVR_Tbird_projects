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

//a feladatok bemeneti param�terei
unsigned char KodDigitek[4]={0,1,3,4};//el�re be�ll�tott k�d digitjei

//billenty�zet 
unsigned char AktualisSor=0;//az aktu�lisan enged�lyezett sort tartalmazza kezdeti �rt�k legyen 0
unsigned char BillAllapot[36];//ebbe olvassa be a program a billenty�k �llapot�t  
unsigned char* BillAllapotElozo=&BillAllapot[0];//ebbe olvassa be a program a billenty�k �llapot�t
unsigned char* BillAllapotAktualis=&BillAllapot[12];//ebbe olvassa be a program a billenty�k �llapot�t
unsigned char* BillMegnyomva=&BillAllapot[24];//ebbe olvassa be a program a billenty�k �llapot�t
unsigned char Osszehasonlit=0;//�sszehasonl�t� flag 

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
	
	for(unsigned char i=0;i<12;i++){
		BillAllapotElozo[i]=1;//kezdeti �rt�k
	}
    /* Replace with your application code */
	while(1){
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
						if(j==9||j==11){//* vagy # gomb lenyom�sa eset�n 
							Osszehasonlit=1;//be�ll�tja az �sszehasonl�t� jelz�flaget hogy a nyom�gomb ciklus ut�n megt�rt�njen a k�dok �sszehasonl�t�sa
							break;	//majd ki is ugrik a ciklusb�l (az �sszehasonl�t�shoz nem olvasunk be t�bb esetleg k�zben lenyomott gombot)																
						}
						else{
							Digitek[3]=Digitek[2];
							Digitek[2]=Digitek[1];
							Digitek[1]=Digitek[0];//a be�rt sz�mokat eggyel magasabb helyre teszi
							//�s az als� digit hely�re beteszi a legut�tt megnyomott sz�mot
							if(j==10){
								Digitek[0]=0;//ha nulla volt megnyomva akkor az ezzel a megold�ssal egy k�l�n elj�r�s
							}
							else{
								Digitek[0]=j+1;//minden m�s esetben a t�mbben t�rolt sorsz�m +1 egyenl� a lenyomott gombbal 
							}
						}
					}
				}
			}
			_delay_ms(5);//kis k�sleltet�s a sorok k�z�tt		
		}
		for(unsigned char i=0;i<12;i++){
			BillAllapotElozo[i]=BillAllapotAktualis[i];//a legut�bb beolvasott �rt�keket elmenti mint egy el�z� �rt�k hogy  ak�vetkez� beolvas�s az aktu�lis �rt�kek t�mbj�be ker�lj�n
		}
		//a bels� k�d �s a be�rt k�d �sszehasonl�t�sa
		if(Osszehasonlit==1){//ha sz�ks�ges a k�dok �sszehasonl�t�sa
			if(Digitek[0]==KodDigitek[3] && Digitek[1]==KodDigitek[2] && Digitek[2]==KodDigitek[1] && Digitek[3]==KodDigitek[0]){//akkor megn�zi a bels� �s a be�rt sz�mot hogy ugyanazt adj�k-e
				PORTE&=0xF3;//A ledek kinull�z�sa
				PORTE|=0x08;//z�ld led bekapcsol�sa
			}
			else{
				PORTE&=0xF3;//A ledek kinull�z�sa
				PORTE|=0x04;//k�k led bekapcsol�sa				
			}
			Osszehasonlit=0;//kinull�zza az �sszehasonl�t� parancsot
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
	DDRC=0xF8;//portC fels� 5 bit kimenet RGBLED|ROWD|ROWC|ROWB|ROWA|COLC|COLB|COLA|
	DDRE|=0x0C;//portE ledek be�ll�t�sa 
	//7 szegmenses kijelz� inicializ�l�sa
	DDRA=0xFF;//portA kimenet 
	PORTG=0x00;//portG bemenet
}
void Init_Timer0(void){//7 szegmenses kijelz� multiplex�l�s�ra haszn�lt 8 bites timer
	TCCR0|=0B00000101;//128 as el�oszt�s Fclk/128 ez lesz a megszak�t�s �teme
	TIMSK |= (1 << TOIE0);//megszak�t�s enged�lyez�s
}