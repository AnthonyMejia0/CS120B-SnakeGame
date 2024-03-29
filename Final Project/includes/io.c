#ifndef IO_C
#define IO_C

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))
          
/*-------------------------------------------------------------------------*/

#define DATA_BUS PORTD		// port connected to pins 7-14 of LCD display
#define CONTROL_BUS PORTC	// port connected to pins 4 and 6 of LCD disp.
#define RS 1			// pin number of uC connected to pin 4 of LCD disp.
#define E 0			// pin number of uC connected to pin 6 of LCD disp.

/*-------------------------------------------------------------------------*/

void delay_ms(int miliSec) //for 8 Mhz crystal
{
	int i,j;
	for(i=0;i<miliSec;i++)
	for(j=0;j<775;j++)
	{
		asm("nop");
	}
}

void LCD_WriteCommand (unsigned char Command) {
   CLR_BIT(CONTROL_BUS,RS);
   DATA_BUS = Command;
   SET_BIT(CONTROL_BUS,E);
   asm("nop");
   CLR_BIT(CONTROL_BUS,E);
   delay_ms(2); // ClearScreen requires 1.52ms to execute
}

void LCD_init(void) {

	//wait for 100 ms.
	delay_ms(100);
	LCD_WriteCommand(0x38);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0f);
	LCD_WriteCommand(0x01);
	delay_ms(10);
}

void LCD_ClearScreen(void) {
	LCD_WriteCommand(0x01);
}

void LCD_WriteData(unsigned char Data) {
   SET_BIT(CONTROL_BUS,RS);
   DATA_BUS = Data;
   SET_BIT(CONTROL_BUS,E);
   asm("nop");
   CLR_BIT(CONTROL_BUS,E);
   delay_ms(1);
}

void LCD_Cursor(unsigned char column) {
	if ( column < 17 ) { // 16x1 LCD: column < 9
		// 16x2 LCD: column < 17
		LCD_WriteCommand(0x80 + column - 1);
		} else {
		LCD_WriteCommand(0xB8 + column - 9);	// 16x1 LCD: column - 1
		// 16x2 LCD: column - 9
	}
}

void LCD_DisplayString( unsigned char column, char* string) {
   unsigned char c = column;
   
   if (c < 16) {
	   while(*string && c <= 16) {
		  LCD_Cursor(c++);
		  LCD_WriteData(*string++);
	   }
   }
   else {
	   while(*string && c <= 32) {
		   LCD_Cursor(c++);
		   LCD_WriteData(*string++);
	   }
   }
}


//Build char function was used from https://hackprojects.wordpress.com/forum/avr/display-custom-character-on-16x2-lcd-to-avr-microcontroller-atmega-8/
void LCD_BuildChar(unsigned char loc, unsigned char *p)
{
	unsigned char i;
	if(loc<8) //If valid address
	{
		LCD_WriteCommand(0x40+(loc*8)); //Write to CGRAM
		for(i=0;i<8;i++)
			LCD_WriteData(p[i]); //Write the character pattern to CGRAM
	}
	LCD_WriteCommand(0x80); //shift back to DDRAM location 0
}

#endif
