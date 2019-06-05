#include <avr/io.h>
#include "shift_register.h"
#include "io.c"
#include "Snake.h"

#define us unsigned short
#define uc unsigned char

const uc EEPROM_Address = 0x1A; //Set location to write to EEPROM

//EEPROM write and read based on functions given in Atmega documentaion
void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE));
	
	EECR = (0<<EEPM1) | (0<<EEPM0);
	
	/* Set up address and Data Registers */
	EEAR = uiAddress;
	EEDR = ucData;
	
	EECR |= (1<<EEMPE);
	EECR |= (1<<EEPE);
}

unsigned char EEPROM_read(unsigned int uiAddress)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE));
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}

/////////////////////////////////////////////////////////
/*State machine to Read, Set, and Erase High Score*/
/////////////////////////////////////////////////////////
enum Set_High_Score{Set_To_Zero, init, Wait, Set_New_Score, Read_New_Score, RESET,};
	
int Set_HighScore_Tick(int state) {
	uc reset = ~PINA & 0x18;
	static uc i;
	
	switch (state) {
		case Set_To_Zero:
			EEPROM_write(EEPROM_Address, 0x00);
			state = init;
			break;
			
		case init:
			HIGH_SCORE = EEPROM_read(EEPROM_Address);
			state = Wait;
			break;
		
		case Wait:
			if (reset) {
				state = RESET;
				i = 0;
			}
			else {
				if (Game_Score > HIGH_SCORE) {
					NEW_HIGH_SCORE = 1;
					state = Set_New_Score;
				}
			}
			break;
			
		case Set_New_Score:
			state = Read_New_Score;
			break;
			
		case Read_New_Score:
			state = Wait;
			break;
			
		case RESET:
			if (i >= 200){
				state = Set_To_Zero;
			}
			else {
				state = (reset) ? RESET: Wait;
			}
			break;
			
		default:
			state = init;
			break;
	}
	
	switch (state) {
		case Set_To_Zero:
			break;
			
		case init:
			break;
			
		case Wait:
			break;
			
		case Set_New_Score:
			EEPROM_write(EEPROM_Address, Game_Score);
			break;
			
		case Read_New_Score:
			HIGH_SCORE = EEPROM_read(EEPROM_Address);
			break;
			
		case RESET:
			i++;
			break;
	}
	
	return state;
}
///////////////////////////////////////////////////////////


typedef struct _task {
	/*Tasks should have members that include: state, period,
		a measurement of elapsed time, and a function pointer.*/
	signed char state; //Task's current state
	unsigned long int period; //Task period
	unsigned long int elapsedTime; //Time elapsed since last task tick
	int (*TickFct)(int); //Task tick function
} task;

unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}


int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	LCD_init();	
	ADC_init();
	srand(time(NULL));
		
	/*Hex values for each custom character to be stored in memory*/
	uc snake_character[8] = {0x11,0x0A,0x1F,0x15,0x1F,0x11,0x0E, 0x00};
	uc diamond_character[8] = {0x04,0x0E,0x1F,0x1F,0x0E,0x04,0x00, 0x00};
	uc controller_character1[8] = {0x00,0x1F,0x1B,0x15,0x1B,0x1C,0x1C,0x00};
	uc controller_character2[8] = {0x00,0x1F,0x1B,0x15,0x1B,0x07,0x07,0x00};
	uc crown[8] = {0x00,0x00,0x00,0x15,0x15,0x1F,0x1F,0x00};
		
	/*Save each custom character to sequential locations in memory*/
	LCD_BuildChar(0, snake_character);
	LCD_BuildChar(1, diamond_character);
	LCD_BuildChar(2, controller_character1);
	LCD_BuildChar(3, controller_character2);
	LCD_BuildChar(4, crown);
	
	/*Define periods for each state machine*/	
	unsigned long int Set_HighScore_Tick_period = 5;
	unsigned long int Snake_Dir_Tick_period = 100;
	unsigned long int PrintSnake_Tick_period = 1;
	unsigned long int GetADC_Tick_period = 50;

	//Declare an array of tasks
	static task Snake_dir_task;
	static task Print_Snake_Task;
	static task Get_ADC_Task;
	static task Set_High_Score_Task;
	
	task *tasks[] = { &Set_High_Score_Task, &Get_ADC_Task, &Snake_dir_task, &Print_Snake_Task };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	Snake_dir_task.state = -1;//Task initial state.
	Snake_dir_task.period = Snake_Dir_Tick_period;//Task Period.
	Snake_dir_task.elapsedTime = Snake_Dir_Tick_period;//Task current elapsed time.
	Snake_dir_task.TickFct = &Snake_Dir_Tick;//Function pointer for the tick.
	
	Print_Snake_Task.state = -1;//Task initial state.
	Print_Snake_Task.period = PrintSnake_Tick_period;//Task Period.
	Print_Snake_Task.elapsedTime = PrintSnake_Tick_period;//Task current elapsed time.
	Print_Snake_Task.TickFct = &PrintSnake_Tick;//Function pointer for the tick.
	
	Get_ADC_Task.state = -1;//Task initial state.
	Get_ADC_Task.period = GetADC_Tick_period;//Task Period.
	Get_ADC_Task.elapsedTime = GetADC_Tick_period;//Task current elapsed time.
	Get_ADC_Task.TickFct = &GetADC_Tick;//Function pointer for the tick.
	
	Set_High_Score_Task.state = -1;//Task initial state.
	Set_High_Score_Task.period = Set_HighScore_Tick_period;//Task Period.
	Set_High_Score_Task.elapsedTime = Set_HighScore_Tick_period;//Task current elapsed time.
	Set_High_Score_Task.TickFct = &Set_HighScore_Tick;//Function pointer for the tick.

	// Set the timer and turn it on
	TimerSet(1); //System period must be 1 in order to light up each individual LED in Matrix without too much noticeable stuttering
	TimerOn();

	unsigned short i; // Scheduler for-loop iterator
	while(1) {
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
}