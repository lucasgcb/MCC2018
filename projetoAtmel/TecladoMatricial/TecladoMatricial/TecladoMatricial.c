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

#include <lcd/LCD.h>
#include <lcd/LCD.c>
#include <bits.h>
const char tabela[4][3] PROGMEM = {{"123"},{"456"},{"789"},{"E0E"}};//vari?vel gravada na mem?ria flash
void init();

enum STATES {CHECK,DEBOUNCE};
volatile uint8_t cursor = 0x80;
volatile uint8_t state = 0;
volatile uint8_t i = 0;
volatile uint8_t j = 0;
volatile uint8_t ic = 0;
volatile uint8_t jc = 0;
int main(){
	
	/* Criação do FILE e mapeamento para o stdout */

	GPIO_D->DDR = 0xFF;
	FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);
	stdout = &lcd_str;
	inic_LCD_4bits();
	init();
	while(1)
	{
		for(j=0;j<3;j++)
		{
			cmd_LCD(0xC4,0);
			printf("%s%d", "Estado:",state);
			cmd_LCD(cursor,0);
			if(cursor>0xC3) 
				cursor = 0x80;
			if(state != DEBOUNCE)
			{
				set_bit(PORTB,j);	
			}
			for(i=0;i<4; i++)
			{	
				switch(state)
				{
					case CHECK: 
						if(tst_bit(PINC,i))
						{
							state = DEBOUNCE;
							ic = i;
							jc = j;
							printf("%c", pgm_read_byte(&tabela[i][j]));
							cursor++;
						}
						break;
					case DEBOUNCE:
						if(jc == j && ic == i && ~tst_bit(PINC,i))
						{
							state = CHECK;
						}
						else 
							state = DEBOUNCE;
						break;
					default:
						break;
				}
			}
			if(state != DEBOUNCE)
			{
				clr_bit(PORTB,j);
			}
		}
	}

	return 0;
}
//
void init()
{
		GPIO_C->DDR = ~((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4));
		GPIO_B->DDR = 0xff;
		/*PCICR |= (1<<PCIE1); //ativa Vetor de interrupção C
		PCMSK1 |= (1<< PCINT8) | (1<< PCINT9) | (1<< PCINT10) | (1<< PCINT11); //ativa interrupções em PCINT0_vector -> PCINT0 e PCINT1*/
		
		/*TCCR1B |= (1 << CS12 | 1 << WGM12 ); //Modo CTC
		TIMSK1 |= (1 << OCIE1A); // Enable CTC interrupt
		OCR1A = 0xF423;*/
		//sei();
	
}