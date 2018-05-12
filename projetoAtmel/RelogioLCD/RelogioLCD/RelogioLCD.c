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

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include <LCD.h>
#include <LCD.c>
#include <bits.h>
void init();
volatile uint8_t selection = 0;
volatile uint8_t tempo[6]={0};
//volatile uint8_t PCs[6] = {PC0,PC1,PC2,PC3,PC4,PC5};
volatile uint8_t j = 0;
int main(){
	
	/* Criação do FILE e mapeamento para o stdout */

	GPIO_D->DDR = 0xFF;
	FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);
	stdout = &lcd_str;
	inic_LCD_4bits();
	
	init();
	while(1)
	{
		
		printf("%d%d:", tempo[0], tempo[1]);
		printf("%d%d:", tempo[2], tempo[3]);
		printf("%d%d", tempo[4], tempo[5]);
		_delay_ms(60);
		
		/* Reposiciona cursor no inicio */
		cmd_LCD(0x80,0);
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
	
	TCCR1B |= (1 << CS12 | 1 << WGM12 ); //Modo CTC
	TIMSK1 |= (1 << OCIE1A); // Enable CTC interrupt
	OCR1A = 0xF423;
	sei(); //  Ativa interrupção global
	
}