#include "stm32f10x.h"                  
int durum=0;
int durum2=0;
int eepromdurum=0;
int eepromdurum2=0;
uint8_t bayrak=0;
uint8_t bayrak2=0;

void systick_init(void)
{
	SysTick->CTRL = 0;
	SysTick->LOAD = 72000000/8000;
	SysTick->VAL = 0;
	SysTick->CTRL |= 5;

}

void DelayMillis(void)
{
	SysTick->LOAD = 0x11940;
	SysTick->VAL = 0;
	while((SysTick->CTRL & 0x00010000) == 0);
}

void DelayMs(unsigned long t)
{
	for(;t>0;t--)
		{
			DelayMillis();
		}
}


#define EEPROM_START_ADDRESS ((uint32_t)0x0801F800) //126. page

uint16_t eeprom_oku(uint32_t  adr)
{
	adr=EEPROM_START_ADDRESS+adr;
  uint16_t * Pntr = (uint16_t *)adr;
  return(*Pntr);  
}
///////////////// FLASH KILIDI AÇMA ALT PROGRAMI /////////////////////    
void Unlock_Flash (void)
{
  FLASH->KEYR=0x45670123;  //Flash kilidini açmak için FLASH->KEYR registerine KEY1 ve KEY2 sirayla yazilmalidir
  FLASH->KEYR=0xCDEF89AB;
}
///////////////// FLASH KILITLEME ALT PROGRAMI ///////////////////// 
void Lock_Flash (void)
{
  FLASH->CR=0x00000080;  //FLASH_CR registeri resetlendiginde FLASH kiltlenmis olur
}
//////////////// ISTENILEN ADRESTEKI VERIYI FLASHTAN SILME ALT PROGRAMI //////
void eeprom_sil (uint32_t i)
{
  FLASH->CR|=0x00000002;            //PER enable
  FLASH->AR=EEPROM_START_ADDRESS+i;//FLASH->AR registerine silinmek istenen adres yazilir
  FLASH->CR|=0x00000040;            //STRT anable
  while((FLASH->SR&0x00000001));    //Islem bitene kadar bekle(BUSY kontrol ediliyor)
  FLASH->CR &= ~0x00000042;         //FLASH->CR ilk durumuna aliniyor (kilit hala açik!) 
	Lock_Flash();
}
///////////////// ISTENILEN ADRESE VERI YAZMA ALT PROGRAMI /////////////////////
void Write_Flash (uint32_t adr, uint16_t data)
{
  FLASH->CR|=0x00000001;           //PG enable
  *(__IO uint16_t*)adr = data;     //istenen adrese istenen data yaziliyor
  while((FLASH->SR&0x00000001));   //Islem bitene kadar bekle(BUSY kontrol ediliyor)
}
void eeprom_yaz(int i,int veri)
{
//    Unlock_Flash();
//    eeprom_sil(i); 
    Write_Flash(EEPROM_START_ADDRESS+i,veri);
//    Lock_Flash(); 
}	



void butonanahtar(void){
		if(GPIOA->IDR & (1<<3)) /// Checking status of PIN ! portA  A9 pini
		{
			durum=!durum;	
		}
		while(GPIOA->IDR & (1<<3))	
		{
		 DelayMs(1);
		}
	}

void butonanahtar2(void){
		if(GPIOA->IDR & (1<<6)) /// Checking status of PIN ! portA  A9 pini
		{
			durum2=!durum2;
		}
		while(GPIOA->IDR & (1<<6))	
		{
		 DelayMs(1);
		}
	}

int main(void)
{
 	RCC->APB2ENR |= (1<<2); //// Enabling PORT A
	GPIOA->CRL &= 0xF0FF0FFF; /// A3 pini resetlendi
	GPIOA->CRL |= 0x08008000; /// A3 pini girisi push pull
	GPIOA->CRH &= 0xFFF0FF0F; /// A9 ve A8 pini resetlendi
	GPIOA->CRH |= 0x00030030; /// A9 ve A8 pini girisi push pull
GPIOA->ODR |= 0x1000;
GPIOA->ODR |= 0x0200;
systick_init();	

		
	
	while(1)
	{

    butonanahtar();
		butonanahtar2();		
eepromdurum=eeprom_oku(4);
eepromdurum2=eeprom_oku(6);
				Unlock_Flash();	
		if(durum==1 || eepromdurum==0x02) //a3
		{ 
		 GPIOA->ODR &= ~0x1000;
     eeprom_yaz(4,0x02);
		if(GPIOA->IDR & (1<<3)){
			eeprom_yaz(4,0x00);
		 }			
		}
		else{
		 GPIOA->ODR |= 0x1000;		
		}
		if(durum2==1 || eepromdurum2==0x02) //a6
		{
		 GPIOA->ODR &=~0x0200;	
     eeprom_yaz(6,0x02);
		if(GPIOA->IDR & (1<<6)){
			eeprom_yaz(6,0x00);
		 }
		}
		else{
		 GPIOA->ODR |= 0x0200;
		}
     eeprom_sil(0);
  }
	
}

