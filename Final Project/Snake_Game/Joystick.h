#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <avr/io.h>
#include "io.h"

const short MAX = 190;
const short MIN = 15;
unsigned char y_direction, x_direction;
unsigned char direction = 0;

void ADC_init()
{
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADMUX |= (1 << REFS0);
	ADCSRA |= (1 << ADSC);
}

//adc_read function found on http://maxembedded.com/2011/06/the-adc-of-the-avr/
uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	ch &= 0x07;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write ’1? to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes ’0? again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}


enum GetADC {ReadADC, SetDirection};

int GetADC_Tick(int state) {
	switch (state) {
		case ReadADC:
			y_direction = adc_read(0);
			x_direction = adc_read(1);
			state = SetDirection;
			break;
			
		case SetDirection:
			state = ReadADC;

		default:
			state = ReadADC;
			break;
	}

	switch (state) {
		case ReadADC:
			break;
			
		case SetDirection:
			if (y_direction >= MAX) { 
				direction = 1; 
			} 
			else if (y_direction <= MIN) { 
				direction = 2; 
				} 
			else if (x_direction <= MIN) { 
				direction = 3; 
			} 
			else if (x_direction >= MAX) { 
				direction = 4; 
			}
			else { 
				direction = 0; 
			}
			break;
	}
	
	return state;
}

#endif