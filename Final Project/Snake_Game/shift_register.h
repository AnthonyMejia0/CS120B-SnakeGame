#include <avr/io.h>
#include "timer.h"

#define SHIFT_DDR DDRB
#define SHIFT_PORT PORTB
#define DATA 0
#define SRCLCK 2
#define RCLCK 1
#define uc unsigned char


void send(unsigned short value)
{
	for (unsigned short i = 0; i < 16; i++)
	{
		if (i < 8) {
			if (value&0x8000)
			{
				SHIFT_PORT = SHIFT_PORT | (0x0001<<DATA);
			}
			else
			{
				SHIFT_PORT = SHIFT_PORT & ~(0x0001<<DATA);
			}
		}
		else {
			if (value&0x8000) {
				SHIFT_PORT = SHIFT_PORT & ~(0x0001<<DATA);
			}
			else {
				SHIFT_PORT = SHIFT_PORT | (0x0001<<DATA);
			}
		}
		value = value << 1;
		// clock pin high
		SHIFT_PORT = SHIFT_PORT | (0x0001<<SRCLCK);
		// clock pin low
		SHIFT_PORT = SHIFT_PORT & ~(0x0001<<SRCLCK);
	}
	// load pin high
	SHIFT_PORT = SHIFT_PORT | (0x0001<<RCLCK);
	// load pin low
	SHIFT_PORT = SHIFT_PORT & ~(0x0001<<RCLCK);
}