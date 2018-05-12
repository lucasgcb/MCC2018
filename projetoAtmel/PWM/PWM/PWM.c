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

#include <lcd/LCD.h>
#include <lcd/LCD.c>
#include <bits.h>
#define _Line1StartAddress 0x80
#define _Line2StartAddress 0xC4
#define _BlankLine "                "
#define _bufferSize 4
#define writeLimit 0xC3
#define _BportOffset 3
const char tabela[4][3] PROGMEM = {{"123"},{"456"},{"789"},{"E0E"}};//vari?vel gravada na mem?ria flash
void init();
uint8_t stateCheck();
char matrixSweep();
void printState();
void printState();
void clearDisplay(char *);
enum STATES {CHECK,DEBOUNCE,PRINT};
volatile uint8_t cursor = _Line1StartAddress;
volatile uint8_t state = 0;
volatile uint8_t i = 0;
volatile uint8_t j = 0;
volatile uint8_t selectedColumn = 0;
volatile uint8_t selectedLine = 0;
int main(){
	
	/* Criação do FILE e mapeamento para o stdout */
	
	GPIO_D->DDR = 0xFF;
	FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);
	stdout = &lcd_str;
	inic_LCD_4bits();
	init();
	
	char buffer[_bufferSize]= {'\0'};
	uint8_t currentPos = 0;
	uint8_t pwm = 0x00;
	while(1)
	{
		currentPos = cursor-_Line1StartAddress;
		if(currentPos == _bufferSize)
		{
			clearDisplay(buffer);
		}
		else
		{
			buffer[currentPos] = matrixSweep();
			if(buffer[currentPos] != 0xFF)
				printf("%c", buffer[currentPos]);
			if(buffer[currentPos] == 'E')
			{
				clearDisplay(buffer);	
				OCR1A = pwm;
				cmd_LCD(_Line2StartAddress,0);
				printf("%d", pwm);
				cmd_LCD(cursor,0);
			}
		}
		
		pwm = atoi(buffer);
		
	}

	return 0;
}



void printState()
{	
	cmd_LCD(_Line2StartAddress,0);
	printf("%s%d", "Estado:",state);
	cmd_LCD(cursor,0);
}
void clearDisplay(char* buffer)
{
	cmd_LCD(_Line1StartAddress,0);
		printf("%s", _BlankLine);
	cmd_LCD(_Line2StartAddress,0);
		printf("%s", _BlankLine);
	cmd_LCD(_Line1StartAddress,0);
	
	for(i=0;i<_bufferSize; i++)
		buffer[i]='\0';
	
	cursor=_Line1StartAddress;
}
char matrixSweep()
{
	for(j=0;j<3;j++)
	{
		if(cursor>writeLimit)
		cursor = _Line1StartAddress;
		
		set_bit(PORTB,(j+_BportOffset));
			
		for(i=0;i<4; i++)
		{
			if(stateCheck() == PRINT)
			{
				cursor++;	
				return pgm_read_byte(&tabela[selectedColumn][selectedLine]);
			}
		}
		
		clr_bit(PORTB,(j+_BportOffset));
	}
	return 0xFF;
}
uint8_t stateCheck()
{
	switch(state)
	{
		case CHECK:
			if(tst_bit(PINC,i))
			{
				state = PRINT;
				selectedColumn = i;
				selectedLine = j;
			}
			break;
		case PRINT:
			state = DEBOUNCE;
			break;
		case DEBOUNCE:
			if(selectedLine == j && selectedColumn == i && tst_bit(PINC,i))
			{
				state = DEBOUNCE;
			}
			else
				if(selectedLine == j && selectedColumn == i)
					state = CHECK;
				else
					state = DEBOUNCE;
			break;
		default:
			break;
	}
	return state;
}
//
void init()
{
		GPIO_C->DDR = ~((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4));
		GPIO_B->DDR = 0xff;
		/**
	 * We will be using OCR1A as our PWM output which is the
	 * same pin as PB1.
	 */
	DDRB |= _BV(PB1);

	/**
	 * There are quite a number of PWM modes available but for the
	 * sake of simplicity we'll just use the 8-bit Fast PWM mode.
	 * This is done by setting the WGM10 and WGM12 bits.  We 
	 * Setting COM1A1 tells the microcontroller to set the 
	 * output of the OCR1A pin low when the timer's counter reaches
	 * a compare value (which will be explained below).  CS10 being
	 * set simply turns the timer on without a prescaler (so at full
	 * speed).  The timer is used to determine when the PWM pin should be
	 * on and when it should be off.
	 */
	TCCR1A |= _BV(COM1A1) | _BV(WGM10);
	TCCR1B |= _BV(CS12) | _BV(WGM12);
	OCR1A = 0x00;

	/**
	 *  This loop is used to change the value in the OCR1A register.
	 *  What that means is we're telling the timer waveform generator
	 *  the point when it should change the state of the PWM pin.
	 *  The way we configured it (with _BV(COM1A1) above) tells the
	 *  generator to have the pin be on when the timer is at zero and then
	 *  to turn it off once it reaches the value in the OCR1A register.
	 *
	 *  Given that we are using an 8-bit mode the timer will reset to zero
	 *  after it reaches 0xff, so we have 255 ticks of the timer until it
	 *  resets.  The value stored in OCR1A is the point within those 255
	 *  ticks of the timer when the output pin should be turned off
	 *  (remember, it starts on).
	 *
	 *  Effectively this means that the ratio of pwm / 255 is the percentage
	 *  of time that the pin will be high.  Given this it isn't too hard
	 *  to see what when the pwm value is at 0x00 the LED will be off
	 *  and when it is 0xff the LED will be at its brightest.
	 */
	
}