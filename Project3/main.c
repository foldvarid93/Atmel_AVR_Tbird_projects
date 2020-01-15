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

//a feladatok bemeneti paraméterei
unsigned char KodDigitek[4]={0,1,3,4};//elõre beállított kód digitjei

//billentyûzet 
unsigned char AktualisSor=0;//az aktuálisan engedélyezett sort tartalmazza kezdeti érték legyen 0
unsigned char BillAllapot[36];//ebbe olvassa be a program a billentyûk állapotát  
unsigned char* BillAllapotElozo=&BillAllapot[0];//ebbe olvassa be a program a billentyûk állapotát
unsigned char* BillAllapotAktualis=&BillAllapot[12];//ebbe olvassa be a program a billentyûk állapotát
unsigned char* BillMegnyomva=&BillAllapot[24];//ebbe olvassa be a program a billentyûk állapotát
unsigned char Osszehasonlit=0;//összehasonlító flag 

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
	
	for(unsigned char i=0;i<12;i++){
		BillAllapotElozo[i]=1;//kezdeti érték
	}
    /* Replace with your application code */
	while(1){
		for(AktualisSor=0;AktualisSor<4;AktualisSor++){//végigmegy a 4 soron egy for ciklussal
			if(AktualisSor==0){
				PORTC=(PINC & 0B10000111) | 0B00001000;//elsõ sor aktív
			}
			if(AktualisSor==1){
				PORTC=(PINC & 0B10000111) | 0B00010000;//második sor aktív
			}
			if(AktualisSor==2){
				PORTC=(PINC & 0B10000111) | 0B00100000;//harmadik sor aktív
			}
			if(AktualisSor==3){
				PORTC=(PINC & 0B10000111) | 0B01000000;//negyedik sor aktív
			}
			_delay_ms(10);//minimális várakozás az esetelges tranziensek (nyomógomb kapacitások stb...) lezajlásához
			unsigned char tmp=PINC;//felsõ bitek kimaszkolása
			if(tmp&0x01){//ha az elsõ oszlop 1-es
				BillAllapotAktualis[(AktualisSor*3)+0]=1;//elsõ oszlop nincs megnyomva
			}
			else{
				BillAllapotAktualis[(AktualisSor*3)+0]=0;//elsõ oszlop megnyomva 
			}
			if(tmp&0x02){//ha az második oszlop 1-es
				BillAllapotAktualis[(AktualisSor*3)+1]=1;//második oszlop nincs megnyomva
			}
			else{
				BillAllapotAktualis[(AktualisSor*3)+1]=0;//második oszlop megnyomva
			}
			if(tmp&0x04){//ha a harmadik oszlop 1-es
				BillAllapotAktualis[(AktualisSor*3)+2]=1;//harmadik oszlop nincs megnyomva
			}
			else{//egyébként 0
				BillAllapotAktualis[(AktualisSor*3)+2]=0;//harmadik oszlop megnyomva 
			}
			//végigellenzõrzi hogy történt-e billentyû leütés illetve pontosabban elengedés
			for(unsigned char i=0;i<12;i++){
				if(BillAllapotElozo[i]==0 && BillAllapotAktualis[i]==1){//elengedést vizsgál
					BillMegnyomva[i]=1;//beírja hogy a gomb meg volt nyomva
				}
				else {
					BillMegnyomva[i]=0;//ellenkezõ esetben nullát ír be az adott flag helyére
				}
			}
			//kiértékeli a billentyûzet teljes állapotát
			if(BillMegnyomva[(AktualisSor*3)+2]==1 || BillMegnyomva[(AktualisSor*3)+1]==1 || BillMegnyomva[(AktualisSor*3)+0]==1){//ha az aktuális sorban történt gombnyomás már be is avatkozik
				for(unsigned char j=0;j<12;j++){//végignézi a teljes tömböt	
					if(BillMegnyomva[j]==1){//az elsõ helyen ahol billentyû lenyomás van bejegyezve ott megáll
						if(j==9||j==11){//* vagy # gomb lenyomása esetén 
							Osszehasonlit=1;//beállítja az összehasonlító jelzõflaget hogy a nyomógomb ciklus után megtörténjen a kódok összehasonlítása
							break;	//majd ki is ugrik a ciklusból (az összehasonlításhoz nem olvasunk be több esetleg közben lenyomott gombot)																
						}
						else{
							Digitek[3]=Digitek[2];
							Digitek[2]=Digitek[1];
							Digitek[1]=Digitek[0];//a beírt számokat eggyel magasabb helyre teszi
							//és az alsó digit helyére beteszi a legutótt megnyomott számot
							if(j==10){
								Digitek[0]=0;//ha nulla volt megnyomva akkor az ezzel a megoldással egy külön eljárás
							}
							else{
								Digitek[0]=j+1;//minden más esetben a tömbben tárolt sorszám +1 egyenlõ a lenyomott gombbal 
							}
						}
					}
				}
			}
			_delay_ms(5);//kis késleltetés a sorok között		
		}
		for(unsigned char i=0;i<12;i++){
			BillAllapotElozo[i]=BillAllapotAktualis[i];//a legutóbb beolvasott értékeket elmenti mint egy elõzõ érték hogy  akövetkezõ beolvasás az aktuális értékek tömbjébe kerüljön
		}
		//a belsõ kód és a beírt kód összehasonlítása
		if(Osszehasonlit==1){//ha szükséges a kódok összehasonlítása
			if(Digitek[0]==KodDigitek[3] && Digitek[1]==KodDigitek[2] && Digitek[2]==KodDigitek[1] && Digitek[3]==KodDigitek[0]){//akkor megnézi a belsõ és a beírt számot hogy ugyanazt adják-e
				PORTE&=0xF3;//A ledek kinullázása
				PORTE|=0x08;//zöld led bekapcsolása
			}
			else{
				PORTE&=0xF3;//A ledek kinullázása
				PORTE|=0x04;//kék led bekapcsolása				
			}
			Osszehasonlit=0;//kinullázza az összehasonlító parancsot
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
	DDRC=0xF8;//portC felsõ 5 bit kimenet RGBLED|ROWD|ROWC|ROWB|ROWA|COLC|COLB|COLA|
	DDRE|=0x0C;//portE ledek beállítása 
	//7 szegmenses kijelzõ inicializálása
	DDRA=0xFF;//portA kimenet 
	PORTG=0x00;//portG bemenet
}
void Init_Timer0(void){//7 szegmenses kijelzõ multiplexálására használt 8 bites timer
	TCCR0|=0B00000101;//128 as elõosztás Fclk/128 ez lesz a megszakítás üteme
	TIMSK |= (1 << TOIE0);//megszakítás engedélyezés
}