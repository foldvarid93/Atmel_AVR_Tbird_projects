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

//
unsigned char Allapot=1;
unsigned char Muvelet=0;
unsigned char SzamTomb[4];
unsigned char Op1;
unsigned char Op2;
int Eredmeny;

//billentyûzet 
unsigned char AktualisSor=0;//az aktuálisan engedélyezett sort tartalmazza kezdeti érték legyen 0
unsigned char BillAllapot[36];//ebbe olvassa be a program a billentyûk állapotát  
unsigned char* BillAllapotElozo=&BillAllapot[0];//ebbe olvassa be a program a billentyûk állapotát
unsigned char* BillAllapotAktualis=&BillAllapot[12];//ebbe olvassa be a program a billentyûk állapotát
unsigned char* BillMegnyomva=&BillAllapot[24];//ebbe olvassa be a program a billentyûk állapotát
unsigned char Osszehasonlit=0;//összehasonlító flag 

//gombokhoz tartozó változók
unsigned char gombaktualis=0;//aktuális gombállást tárolja
unsigned char gombelozo=0;//az elõzõ beolvasást tárolja
unsigned char gombmegnyomva=0;//a gombnyomások flagjeit tároló változó

//a függvények prototipusai felsorolásszerûen
void Init(void);
void led_out(unsigned char ertek);
void Init_Timer0(void);
void BillentyuBeolvasas(void);
void GombBeolvasas(void);
void felbont(int szam);

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
		//Állapotgép
		if(Allapot==1){//elsõ operandus beolvasása
			led_out(0x01);//1. operandusnál az 1. LED világít			
			BillentyuBeolvasas();//ciklikusan olvassa be a billentyûzetet
			Digitek[0]=SzamTomb[0];
			Digitek[1]=SzamTomb[1];
			Digitek[2]=0;
			Digitek[3]=0;//a számokat átadja a kijelzõnek mivel az operandusok 2 digitesek így a felsõ 2 digit 0 
			Op1=(Digitek[1]*10)+Digitek[0];//a beírt számokat átakalítja egy kétjegyû számmá
			GombBeolvasas();//beolvassa a gombokat (+,-,*) és lép új állapotba						
		}
		if(Allapot==2){//második operandus beolvasása
			led_out(0x02);//2. operandusnál a 2. LED világít			
			BillentyuBeolvasas();//ciklikusan olvassa be a billentyûzetet
			Digitek[0]=SzamTomb[0];
			Digitek[1]=SzamTomb[1];
			Digitek[2]=0;
			Digitek[3]=0;
			Op2=(Digitek[1]*10)+Digitek[0];//a számokat átadja a kijelzõnek mivel az operandusok 2 digitesek így a felsõ 2 digit 0 
			GombBeolvasas();//beolvassa a gombokat (=) és lép új állapotba				
		}
		if(Allapot==3){
			led_out(0x04);//eredményeknél a 3. LED világít
			if(Muvelet==0){//összeadás
				Eredmeny=Op1+Op2;
			}
			if(Muvelet==1){//kivonás
				if(Op1>Op2){
					Eredmeny=Op1-Op2;	
				}
				else{
					Eredmeny=Op2-Op1;// a kivonás különleges bánásmódot igényel, mindig a nagyobból vonja ki a kisebbet
					led_out(0x04|0x80);//de ha negatív lenne az eredmény akkor a LED8-at bekapcsolja 
				}
			}
			if(Muvelet==2){//szorzás
				Eredmeny=Op1*Op2;
			}
			felbont(Eredmeny);//az eredményt átalakítja 4 digitté 
			while(Allapot==3){
				_delay_ms(10);
				BillentyuBeolvasas();//ameddig a 3. állapot áll fenn, addig ciklikusan olvassa be billentyûket
			}
			for(unsigned char j=0;j<4;j++){
				SzamTomb[j]=0;
				Digitek[j]=0;//ha felhasználói beavatkozás történt, kinullázza az átmeneti tárolókat és a kijelzõt egy újabb mûvelet elvégzéséhez
			}																								
		}
	}
}
ISR (TIMER0_OVF_vect){//timer0 megszakítás
	//PORTA=0;//rövid idõre kikapcsolás hogy ne legyen "ghost effekt"
	unsigned char ertek=0;
	ertek=AktualisDigit<<4;//a megfelelõ helyre shifteli a digitek jelét
	ertek=ertek|Digitek[AktualisDigit];
	ertek=ertek|0b10000000;//enable jel hozzávagyolása 
	PORTA=ertek;//érték kiírása
	AktualisDigit++;//aktuális digit növelése
	if(Allapot==1||Allapot==2){//ha az operandusokat beolvasó állapotok vannak akkor csak 2 digitet használjunk
		if(AktualisDigit>1){
			AktualisDigit=0;//ilyenkor csak a 0. és az 1. digitet használjuk
		}
	}
	else{
		if(AktualisDigit>3){//egyébként pedig kiteszi a 4 digitet
			AktualisDigit=0;//akkor következõleg a nullással folytatja, így lesz 0,1,2,3,0,1,2,3 ciklikus mûködés
		}		
	}
	TCNT0=0xB1;//31250Hz -> 400Hz: 31250Hz/400Hz=78(0x4E) 0xFF-0x4E=0xB1 kerül a számlálóba												 
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
void Init_Timer0(void){//7 szegmenses kijelzõ multiplexálására használt 8 bites timer 400Hz-es megszakítás 8000000Hz/400Hz=20000 count -> 20000/256(elõosztó)=78
	TCCR0|=(1<<CS02);//256 os elõosztás 8000000Hz/256=31250Hz
	TCNT0=0xB1;//31250Hz -> 400Hz: 31250Hz/400Hz=78(0x4E) 0xFF-0x4E=0xB1 kerül a számlálóba
	TIMSK |= (1 << TOIE0);//megszakítás engedélyezés
	TIFR|=(1<<TOIE0);//Timer 0 Overflow Interrupt enable
}
void BillentyuBeolvasas(void){
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
						if(Allapot==3){
							Allapot=1;	//ha a 3. állapotban gombnyomás történt, akkor újra az elsõ állapotba ugrik				
						}
						else{
							if(j==9||j==11){//* vagy # gomb lenyomása esetén 																
							
							}
							else{
								SzamTomb[3]=SzamTomb[2];
								SzamTomb[2]=SzamTomb[1];
								SzamTomb[1]=SzamTomb[0];//a beírt számokat eggyel magasabb helyre teszi
								//és az alsó digit helyére beteszi a legutótt megnyomott számot
								if(j==10){
									SzamTomb[0]=0;//ha nulla volt megnyomva akkor az ezzel a megoldással egy külön eljárás
								}
								else{
									SzamTomb[0]=j+1;//minden más esetben a tömbben tárolt sorszám +1 egyenlõ a lenyomott gombbal 
								}						
							}
						}
					}
				}
			}
			_delay_ms(2);//kis késleltetés a sorok között		
		}
		for(unsigned char i=0;i<12;i++){
			BillAllapotElozo[i]=BillAllapotAktualis[i];//a legutóbb beolvasott értékeket elmenti mint egy elõzõ érték hogy  akövetkezõ beolvasás az aktuális értékek tömbjébe kerüljön
		}	
}
void GombBeolvasas(void){
	gombaktualis=PING & 0x1F;//gombok beolvasása
	//szorzás
	if((gombelozo & 0x01)==0 && (gombaktualis & 0x01) == 1){
		gombmegnyomva |= 0b00000001;//ha meg lett nyomva a gomb 0->1 akkor beállít egy flaget egy regiszerben
	}
	if((gombmegnyomva & 0x01)==1 && (gombaktualis & 0x01)==0){
		gombmegnyomva &= 0b11111110;//ha el lett engedve 1->0 akkor kinullázza a flaget
		if(Allapot==1){
			Allapot=2;//a két operandus bevitele közötti állapot (*)
			for(unsigned char j=0;j<4;j++){
				SzamTomb[j]=0;//kinullázza a tömböt, az elõzõ érték maradékai eltávolítása érdekében
			}
			Muvelet=2;//szorzás lesz a mûvelet
		}
	}
	//kivonás
	if((gombelozo & 0x02)==0 && (gombaktualis & 0x02) == 0x02){
		gombmegnyomva |= 0b00000010;//ha meg lett nyomva a gomb 0->1 akkor beállít egy flaget egy regiszerben
	}
	if((gombmegnyomva & 0x02)==0x02 && (gombaktualis & 0x02)==0){
		gombmegnyomva &= 0b11111101;//ha el lett engedve 1->0 akkor kinullázza a flaget
		if(Allapot==1){
			Allapot=2;//a két operandus bevitele közötti állapot (-)
			for(unsigned char j=0;j<4;j++){
				SzamTomb[j]=0;//kinullázza a tömböt, az elõzõ érték maradékai eltávolítása érdekében
			}			
			Muvelet=1;//kivonás
		}		
	}
	//összeadás
	if((gombelozo & 0x04)==0 && (gombaktualis & 0x04) == 0x04){
		gombmegnyomva |= 0b00000100;//ha meg lett nyomva a gomb 0->1 akkor beállít egy flaget egy regiszerben
	}
	if((gombmegnyomva & 0x04)==0x04 && (gombaktualis & 0x04)==0){
		gombmegnyomva &= 0b11111011;//ha el lett engedve 1->0 akkor kinullázza a flaget
		if(Allapot==1){
			Allapot=2;//a két operandus bevitele közötti állapot (+)
			for(unsigned char j=0;j<4;j++){
				SzamTomb[j]=0;//kinullázza a tömböt, az elõzõ érték maradékai eltávolítása érdekében
			}			
			Muvelet=0;//összeadás
		}
	}
	//egyenlõ
	if((gombelozo & 0x08)==0 && (gombaktualis & 0x08) == 0x08){
		gombmegnyomva |= 0b00001000;//ha meg lett nyomva a gomb 0->1 akkor beállít egy flaget egy regiszerben
	}
	if((gombmegnyomva & 0x08)==0x08 && (gombaktualis & 0x08)==0){
		gombmegnyomva &= 0b11110111;//ha el lett engedve 1->0 akkor kinullázza a flaget
		if(Allapot==2){
			Allapot=3;//ha 2. állapotban bevittük az Op2-t akkor az egyenlõ megnyomására a 3. állapotba ugrik.
		}		
	}
	gombelozo=gombaktualis;//a jenenlegi beolvasást elmenti a a következõ olvasás régi értékeként
}
void felbont(int szam){
	Digitek[3]=szam/1000;//ezresek
	szam%=1000;//ami az ezresek után marad
	Digitek[2]=szam/100;//százasok
	szam%=100;//ami a százasok után marad	
	Digitek[1]=szam/10;//tizesek
	szam%=10;// ami a tizesek után marad	
	Digitek[0]=szam;//egyesek
}
void led_out(unsigned char ertek){
	PORTD=ertek&0xF0;
	PORTB=ertek<<4;
}