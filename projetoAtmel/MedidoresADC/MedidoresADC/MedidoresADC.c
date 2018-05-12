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
#include <stdlib.h>
#include <string.h>

#include <LCD/LCD.h>
#include <LCD/LCD.c>
#include <bits.h>

#define num_devices 6
#define lineStart 0x80
#define line2 0xC0
#define blankLine "                      "
void init();
uint16_t adc_read(uint8_t ch);
void clearDisplay();
volatile uint8_t selection = 0;
volatile char measurements_buffer[num_devices]={0};
//volatile uint8_t PCs[6] = {PC0,PC1,PC2,PC3,PC4,PC5};
volatile uint8_t j = 0;
volatile uint8_t cursor = lineStart;
int main(){
	
	/* Criação do FILE e mapeamento para o stdout */
	
	GPIO_D->DDR = 0xFF;
	FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);
	stdout = &lcd_str;
	inic_LCD_4bits();
	
	init();
	while(1)
	{
		clearDisplay();
		for(j=0; j<num_devices/2;j++)
		{
			if(j==2)
				cmd_LCD(line2,0);
			printf("M%d:%dC ", j, measurements_buffer[j]);	
		}
		_delay_ms(2000);
		
		clearDisplay();
		for(j=num_devices/2; j<num_devices;j++)
		{
			if(j==(num_devices/2 + 2))
				cmd_LCD(line2,0);
			printf("M%d:%dC ", j, measurements_buffer[j]);
		}
		
		_delay_ms(2000);
	}

	return 0;
}

void clearDisplay()
{
	cmd_LCD(lineStart,0);
	printf("%s", blankLine);
	cmd_LCD(line2,0);
	printf("%s", blankLine);
	cmd_LCD(lineStart,0);
}
uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with ’7? will always keep the value of ‘ch’ between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF0)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write ’1? to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes ’0? again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}


ISR(TIMER0_OVF_vect)
{
	measurements_buffer[selection]=adc_read(selection)/2;
	selection < 5? (selection++) : (selection=0);
}

void init()
{
	
	
	GPIO_B->DDR = ~((1<<PB0) | (1<<PB1));
	GPIO_B->PORT = (1<<PB0) | (1<<PB1);
	
	ADMUX |= (1<<REFS0);
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	/*TCCR1B |= (1 << CS12 | 1 << WGM12 ); //Modo CTC
	TIMSK1 |= (1 << OCIE1A); // Enable CTC interrupt
	OCR1A = 0xF423;*/
	
	TCCR0B |= (1 << CS12); //prescaler de 256
	TIMSK0 |= (1 << TOIE0); //ativa interrupção por overflow do multiplexador
	
	sei(); //  Ativa interrupção global
	
}