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
#define NUMDISPLAYS 6
void init();
volatile uint8_t selection = 0;
volatile uint8_t tempo[6]={0};
volatile uint8_t revDisplay[6]={0};
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
		printf("%d%d", tempo[0], tempo[1]);
		printf("%d%d", tempo[2], tempo[3]);
		printf("%d%d", tempo[4], tempo[5]);
		printf("\n");
		printf("%d%d", revDisplay[0], revDisplay[1]);
		printf("%d%d", revDisplay[2], revDisplay[3]);
		printf("%d%d", revDisplay[4], revDisplay[5]);
		printf("Hz");
		_delay_ms(60);
		
		/* Reposiciona cursor no inicio */
		cmd_LCD(0x80,0);
	}

	return 0;
}
ISR(TIMER1_COMPA_vect)
{
	for(j=0; j<NUMDISPLAYS; j++)
	{
		revDisplay[j] = tempo[j];
	}
	for(j=0; j<NUMDISPLAYS; j++)
	{
		tempo[j] = 0;
	}
}
//

ISR(TIMER1_CAPT_vect)
{
	tempo[5]++;
	if(tempo[5]>9)
	{
		tempo[5]=0;
		tempo[4]++;
		
		if(tempo[4]>9)
		{
			tempo[4]=0;
			tempo[3]++;
			if(tempo[3]>9)
			{
				tempo[3]=0;
				tempo[2]++;
				if(tempo[2]>9)
				{
					tempo[2]=0;
					tempo[1]++;
					if(tempo[1]>9)
					{
						tempo[0]++;
						tempo[1]=0;
						if(tempo[0]>9)
						{
							tempo[0]=0;
						}
					}
				}
			}
			
		}
	}
}


void init()
{
	
	TCCR1B |= (1 << CS12 | 1 << WGM12 ); // Resolução 256, Modo CTC
	TIMSK1 |= (1 << OCIE1A | 1<<ICIE1); // Interrupção CTC e por Input
	OCR1A = 0xF423;
	
	sei(); //  Ativa interrupção global
	
}