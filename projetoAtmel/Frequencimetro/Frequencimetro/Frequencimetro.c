/*
 * main.c
 *
 *  Created on: Mar 9, 2018
 *      Author: Lucas Bandeira
 *  Frequencimetro com precisão até 117kHz (Proteus?)
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr_gpio.c>
#include <avr/interrupt.h>
#include <avr_gpio.h>
#include <bits.h>

#define NUMDISPLAYS 6
#define MSB 0
#define LSB 5
#define TEMPOCOMP 0xF423
//(x+1) * 256 = 16000000 = F423
const uint8_t tabela[] PROGMEM = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x18, 0x08, 0x03, 0x46, 0x21, 0x06, 0x0E};//vari?vel gravada na mem?ria flash
volatile uint8_t j=0;
volatile uint8_t tempo[NUMDISPLAYS]={0}; // Contador, atualiza através da captura do timer
volatile uint8_t tempoDisplay[NUMDISPLAYS]={0}; //Atualiza com o valor no contador a cada 1 seg
void init();

int main(){
	
	init();
	for(;;){
		//Aguardar interrupções
	}

	return 0;
}
void init()
{
	/////////CFG Temporizador 0 
	TCCR0B |= (1 << CS12 ); // Resolução 256
	TIMSK0 |= (1 << TOIE0); // Interrupção Overflow p/ Multiplexador
	/////////CFG Temporizador 1
	TCCR1B |= (1 << CS12 | 1 << WGM12 ); // Resolução 256, Modo CTC
	TIMSK1 |= (1 << OCIE1A | 1<<ICIE1); // Interrupção CTC e por Input
	OCR1A = TEMPOCOMP; //Tempo para 1seg sobre o Timer na Resolução 256 @ 16MHz
	
	/* Acesso por strutura  */
	GPIO_C->DDR = (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5); //Saida Multiplexador
	GPIO_D->DDR = (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7); //Saida 7 Segmentos
	sei(); //  Ativa Interrupções
	
}

ISR(TIMER0_OVF_vect)
{
	static uint8_t i = 0; //Mantém display atual

	GPIO_C->PORT = 0x00; //Desliga todos os displays
	GPIO_D->PORT = pgm_read_byte(&tabela[tempoDisplay[i]]);
	GPIO_SetBit(GPIO_C, i); //Acende o display atual
	
	i++; //Prepara o proximo display
	
	if (i > NUMDISPLAYS) //Se for o ultimo display, retorne ao primeiro
		i=0;

}

ISR(TIMER1_COMPA_vect)
{
	for(j=0; j<NUMDISPLAYS; j++)
	{
		tempoDisplay[j] = tempo[j];
	}
	for(j=0; j<NUMDISPLAYS; j++)
	{
		tempo[j] = 0;
	}
}
ISR(TIMER1_CAPT_vect)
{
	//6 variaveis 8 bits contando de 0 a 9, incrementando a sucessora no reset
	tempo[LSB]++;
	if(tempo[LSB]>9)
	{
		tempo[LSB]=0;
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
						tempo[1]=0;
						tempo[MSB]++;
					}
					if(tempo[MSB]>9)
					{
						tempo[MSB]=14; //ERR
					}
				}
			}
		}
	}
}

