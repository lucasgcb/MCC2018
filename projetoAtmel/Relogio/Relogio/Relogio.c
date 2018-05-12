/*
 * main.c
 *
 *  Created on: Mar 9, 2018
 *      Author: Renan Augusto Starke
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr_gpio.c>
#include <avr/interrupt.h>
#include <avr_gpio.h>
#include <bits.h>
const uint8_t tabela[] PROGMEM = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x18, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};//vari?vel gravada na mem?ria flash
void init();
volatile uint8_t selection = 0;
volatile uint8_t tempo[6]={0};
volatile uint8_t PCs[6] = {PC0,PC1,PC2,PC3,PC4,PC5};
volatile uint8_t j = 0;
int main(){

	init();
	for(;;){
		
	}

	return 0;
}
ISR(PCINT0_vect)
{
	if(!tst_bit(PINB, PB0))
	{
		if(selection == 0)
		{
			selection = 1;
		}
		else
		if(selection == 1)
		{
			selection = 0;
		}
	}
	if(!tst_bit(PINB, PB1))
	{
		if(selection == 0)
		{
			tempo[1]++;
			if(tempo[0]==2 && tempo[1]>3)
			{
				tempo[0]=0;
				tempo[1]=0;
			}
			else
			if(tempo[1]>9)
			{
				tempo[1]=0;
				tempo[0]++;
			}
			
		}
		else
		{
			tempo[3]++;
			if(tempo[3]>9)
			{
				tempo[3]=0;
				tempo[2]++;
				if(tempo[2]>5)
				{
					tempo[2]=0;
				}
			}
		}
	}
}
//

ISR(TIMER0_OVF_vect)
{
	//Multiplexação
	static uint8_t i = 0;

	GPIO_C->PORT = 0x00;
	GPIO_D->PORT = pgm_read_byte(&tabela[tempo[i]]);
	GPIO_SetBit(GPIO_C, i);
	
	i++;
	
	if (i > 5)
	i=0;
	
}

ISR(TIMER1_COMPA_vect)
{
	tempo[5]++;
	if(tempo[5]>9)
	{
		tempo[5]=0;
		tempo[4]++;
		
		if(tempo[4]>5)
		{
			tempo[4]=0;
			tempo[3]++;
			if(tempo[3]>9)
			{
				tempo[3]=0;
				tempo[2]++;
				if(tempo[2]>5)
				{
					tempo[2]=0;
					tempo[1]++;
					if(tempo[0]==2 && tempo[1]>3)
					{
						tempo[0]=0;
						tempo[1]=0;
					}
					else
						if(tempo[1]>9)
						{
							tempo[1]=0;
							tempo[0]++;
						}
					
				}
			}
			
		}
	}
}


void init()
{
	GPIO_B->DDR = ~((1<<PB0) | (1<<PB1));
	GPIO_B->PORT = (1<<PB0) | (1<<PB1);
	
	PCICR |= (1<<PCIE0); //ativa Vetor de interrupção B
	PCMSK0 |= (1<<PCINT0) | (1<< PCINT1); //ativa interrupções em PCINT0_vector -> PCINT0 e PCINT1
	
	TCCR0B |= (1 << CS12); //prescaler de 256
	TIMSK0 |= (1 << TOIE0); //ativa interrupção por overflow do multiplexador
	
	TCCR1B |= (1 << CS12 | 1 << WGM12 ); //Modo CTC
	TIMSK1 |= (1 << OCIE1A); // Enable CTC interrupt
	OCR1A = 0xF000;
	sei(); //  Ativa interrupção global
	
	/* Acesso por strutura  */
	GPIO_C->DDR = (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5);
	GPIO_D->DDR = (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
	
}