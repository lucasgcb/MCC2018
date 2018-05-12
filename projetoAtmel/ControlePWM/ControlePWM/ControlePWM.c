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

#define tensaoNominalMotor 12
#define resistenciaMotor 12
#define rpmNominal 110
#define correnteNominal 100 //centiamperes
#define lineStart 0x80
#define line2 0xC0
#define blankLine "                      "
void init();
uint16_t adc_read(uint8_t ch);
void clearDisplay();
void measure_current();
void checkCurrent();
void printVitalsMonitor();
void checkDutyCycle();
volatile uint8_t strike = 0;
volatile uint8_t panic = 0;
volatile uint16_t deb = 0;
volatile uint16_t measured_current = 0;
volatile uint8_t checkFlag = 0;
volatile uint16_t duty= 0;
enum State {
	CHECKING,
	PANICKING,
};
enum State state = CHECKING;

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
		
		switch(state)
		{
			case CHECKING:
				TCCR1A |= _BV(COM2A1); //Retoma PWM
				checkCurrent();
				checkDutyCycle();
				checkFlag=0;
				OCR1A = duty;

				break;
			case PANICKING:
				TCCR1A = 0; //Para PWM
				checkCurrent();
				checkDutyCycle();
				checkFlag=0;
				break;
			default:
				break;
		}
		printVitalsMonitor();
		_delay_ms(60);
	}

	return 0;
}

void printVitalsMonitor()
{
	uint16_t dutyPercentual = ((duty)*100)/204;
	uint16_t tensao = ((measured_current * resistenciaMotor) / 100);
	uint16_t rpm = (((measured_current * resistenciaMotor)/100)*rpmNominal)/tensaoNominalMotor;
	clearDisplay();
	printf("DUTY:%d%% ", dutyPercentual );
	printf("%dV ", tensao );
	printf("%dms", state);
	cmd_LCD(line2,0);
	printf("%d.%.02dA", measured_current/100, measured_current%100 );
	if(strike>0)
		printf("!");
	if(strike>1)
		printf("!");
	printf(" %dRPM", rpm );
	if(panic>0)
		printf(" !P%d!", panic);
}

void measure_current()
{
	uint16_t measured = 0;
	for(j=0;j<100; j++)
	{
		
		measured += adc_read(0) /2;
		_delay_us(200);
	}
	measured_current = ((measured / 100) )/2;
	if(measured_current == 25) //25 centiamps é o limite inferior do medidor
		measured_current=0;
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
	ADMUX = (ADMUX & 0xF0)|ch; // limpa os 3 ultimos bits antes do or
	
	// inicia conversao
	ADCSRA |= (1<<ADSC);
	
	//Aguardar conversão
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

void checkCurrent()
{
	if(checkFlag)
	{
		measure_current();
	}
}
void checkDutyCycle()
{
	if(checkFlag)
	{
		duty=adc_read(6)/5;
	}
}

ISR(TIMER0_COMPA_vect)
{
	checkFlag = 1;
}


ISR(TIMER2_COMPA_vect)
{
	deb++;
	if(deb==485)
	{
		switch(state)
		{
			case CHECKING:
				if(measured_current>correnteNominal && panic == 0)
				{
					strike++;
				}
				else
				{
					strike=0;
				}
				if(strike>2)
				{
					strike = 0;
					panic = 10;
					state=PANICKING;
				}
				break;
			case PANICKING:
				if(!panic)
				{
					state=CHECKING;
				}
				else
				{
					panic--;
					state=PANICKING;
				}
				break;
			default:
				break;
		}
		deb = 0;
	}
	
}

void init()
{
	
	ADMUX |= (1<<REFS0);
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	/*TCCR1B |= (1 << CS12 | 1 << WGM12 ); //Modo CTC
	TIMSK1 |= (1 << OCIE1A); // Enable CTC interrupt
	OCR1A = 0xF423;*/
	DDRB |= _BV(PB1); // Saida PWM
	
	TCCR1A |= _BV(COM2A1); //Comutar OC1A
	TCCR1B |=  _BV(CS11) | _BV(WGM13); //prescaler 8 + Fast PWM com TOP em ICR1 + 
	ICR1 = 0xC7; // para 5kHz
	OCR1A = 0x00;
	
	TCCR0B |= (1 << CS12 | 1 << CS10 | 1 << WGM02 );
	TIMSK0 |= (1 << OCIE0A); //ativa interrupção por ctc do multiplexador
	OCR0A = 0x1;
	
	TCCR2B |= (1 << CS12 | 1 << CS10 | 1 << WGM02 );
	TIMSK2 |= (1 << OCIE1A); //ativa interrupção por ctc do multiplexador
	OCR2A = 0xFF;
	
	sei(); //  Ativa interrupção global
	
}