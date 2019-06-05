#ifndef SNAKE_H
#define SNAKE_H

#include <avr/io.h>
#include <string.h>
#include "Joystick.h"
#include "time.h"

#define us unsigned short
#define uc unsigned char


struct Coordinates {
	uc row;
	uc col;
};

struct Snake {
	struct Coordinates current;
	struct Coordinates previous;
};

struct Snake body[64];
uc snakeLength = 0;
struct Coordinates Fruit;
uc Loser;
uc HIGH_SCORE;
uc Game_Score;
uc NEW_HIGH_SCORE;

void print_score(uc column, uc value) {
	int tmp = (int)value;
	int firstDigit = tmp / 10;
	int secondDigit = tmp % 10;
	
	if (firstDigit == 0 && secondDigit == 0) {
		LCD_Cursor(column);
		LCD_WriteData(0 + '0');
	}
	else if (firstDigit == 0) {
		LCD_Cursor(column);
		LCD_WriteData(secondDigit + '0');
	}
	else {
		LCD_Cursor(column);
		LCD_WriteData(firstDigit + '0');
		LCD_Cursor(column+1);
		LCD_WriteData(secondDigit + '0');
	}
}

void add_body(uc r, uc c) {
	struct Snake temp;
	temp.current.row = r;
	temp.current.col = c;
	body[snakeLength] = temp;
	snakeLength++;
}

void add_fruit() {
	uc previousR = Fruit.row;
	uc previousC = Fruit.col;
	uc r;
	uc c;
	do {
		r = (rand() % (8 - 1 + 1)) + 1;
	}
	while (r == previousR);
	
	do {
		c = (rand() % (8 - 1 + 1)) + 1;
	}
	while (c == previousC);
	
	Fruit.row = r;
	Fruit.col = c;
}

void check_collision() {
	uc r = body[0].current.row;
	uc c = body[0].current.col;
	
	for (uc i = 1; i < snakeLength; i++) {
		if (r == body[i].current.row && c == body[i].current.col) {
			Loser = 1;
			return;
		}
	}
}

void updatePositions(uc r, uc c) {
	for (uc i = 0; i < snakeLength; i++) {
		body[i].previous.row = body[i].current.row;
		body[i].previous.col = body[i].current.col;
		
		if (i == 0) {
			body[i].current.row = r;
			body[i].current.col = c;
			
			check_collision();
			
			if (r == Fruit.row && c == Fruit.col) {
				add_body(body[snakeLength-1].previous.row, body[snakeLength-1].previous.col);
				add_fruit();
			}
		}
		else {
			body[i].current.row = body[i-1].previous.row;
			body[i].current.col = body[i-1].previous.col;
		}
		
	}
}

us getCoordinates(uc r, uc c) {
	uc upperNibble;
	uc lowerNibble;
	us totalValue = 0x00;
	
	switch(r) {
		case 1:
		upperNibble = 0x01;
		break;
		
		case 2:
		upperNibble = 0x02;
		break;
		
		case 3:
		upperNibble = 0x04;
		break;
		
		case 4:
		upperNibble = 0x08;
		break;
		
		case 5:
		upperNibble = 0x10;
		break;
		
		case 6:
		upperNibble = 0x20;
		break;
		
		case 7:
		upperNibble = 0x40;
		break;
		
		case 8:
		upperNibble = 0x80;
		break;
		
		default:
		upperNibble = 0x01;
		break;
	}
	
	switch(c) {
		case 1:
		lowerNibble = 0x01;
		break;
		
		case 2:
		lowerNibble = 0x02;
		break;
		
		case 3:
		lowerNibble = 0x04;
		break;
		
		case 4:
		lowerNibble = 0x08;
		break;
		
		case 5:
		lowerNibble = 0x10;
		break;
		
		case 6:
		lowerNibble = 0x20;
		break;
		
		case 7:
		lowerNibble = 0x40;
		break;
		
		case 8:
		lowerNibble = 0x80;
		break;
		
		default:
		lowerNibble = 0x01;
		break;
	}
	
	totalValue = totalValue | (short)upperNibble<<8 | ((short)lowerNibble & 0x00FF);
	
	return totalValue;
}


void updateGame(uc r, uc c) {
	if (snakeLength >= 0 && snakeLength <=3) {
		Game_Score = 0;
	}
	else {
		Game_Score = snakeLength-3;
	}
	updatePositions(r, c);
	print_score(8, Game_Score);
	print_score(29, HIGH_SCORE);
}

enum PrintSnake_Tick{Print};

int PrintSnake_Tick(int state) {
	switch(state) {
		case Print:
			for (int i = 0; i < snakeLength; i++) {
				uc row = body[i].current.row;
				uc col = body[i].current.col;
		
				send(getCoordinates(row, col));
				send(getCoordinates(Fruit.row, Fruit.col));
			}
			break;
		
		default:
			state = Print;
			break;
	}
	
	return state;
}



enum Snake_Direction{initSnake, WaitForStart, SplashScreen, Start, Up, Down, Left, Right, GameOver};

int Snake_Dir_Tick(int state) {
	static uc i;
	static uc j;
	static uc splashCount;
	static uc gameOverCount;
	us splashStates[6] = {0x0000, 0x1818, 0x3C3C, 0x7E7E, 0xFFFF, 0x0000};
	uc StartButton = ~PINA & 0x08;
	uc Reset = ~PINA & 0x10;
	
	switch (state) {
		case initSnake:
			Loser = 0;
			snakeLength = 0;
			splashCount = 0;
			gameOverCount = 0;
			NEW_HIGH_SCORE = 0;
			send(0x0000);
			LCD_DisplayString(1, "Snake Game");
			LCD_Cursor(12);
			LCD_WriteData(2);
			LCD_Cursor(13);
			LCD_WriteData(3);
			LCD_Cursor(14);
			LCD_WriteData(0);
			LCD_DisplayString(16, " Press Start!");
			state = WaitForStart;
			break;
			
		case WaitForStart:
			state = (StartButton && !Reset) ? SplashScreen: WaitForStart;
			break;
			
		case SplashScreen:
			if (splashCount >= 6) {
				state = Start;
			}
			break;
		
		case Start:
			i = 6;
			j = 3;
			add_body(i, j);
			add_body(i+1, j);
			add_body(i+2, j);
			add_fruit();
			LCD_ClearScreen();
			LCD_DisplayString(1, "Score: ");
			LCD_DisplayString(17, "High Score: ");
			updateGame(i, j);
			state = Up;
			break;
		
		case Down:
			if (Reset && !StartButton) {
				snakeLength = 0;
				LCD_ClearScreen();
				state = initSnake;
			}
			else {
				if (Loser == 1) {
					state = GameOver;
				}
				else if (direction == 3) {
					state = Left;
				}
				else if (direction == 4) {
					state = Right;
				}
			}
			break;
		
		case Up:
			if (Reset && !StartButton) {
				state = initSnake;
				LCD_ClearScreen();
				snakeLength = 0;
			}
			else {
				if (Loser == 1) {
					state = GameOver;
				}
				else if (direction == 3) {
					state = Left;
				}
				else if (direction == 4) {
					state = Right;
				}
			}
			break;
		
		case Left:
			if (Reset && !StartButton) {
				state = initSnake;
				LCD_ClearScreen();
				snakeLength = 0;
			}
			else {
				if (Loser == 1) {
					state = GameOver;
				}
				else if (direction == 1) {
					state = Up;
				}
				else if (direction == 2) {
					state = Down;
				}
			}
			break;
		
		case Right:
			if (Reset && !StartButton) {
				state = initSnake;
				LCD_ClearScreen();
				snakeLength = 0;
			}
			else {
				if (Loser == 1) {
					state = GameOver;
				}
				else if (direction == 1) {
					state = Up;
				}
				else if (direction == 2) {
					state = Down;
				}
			}
			break;
		
		case GameOver:
			if (gameOverCount >= 30) {
				LCD_ClearScreen();
				state = initSnake;
			}
			break;
		
		default:
			state = initSnake;
			break;
	}
	
	switch(state) {
		case initSnake:
			break;
			
		case WaitForStart:
			break;
			
		case SplashScreen:
			send(splashStates[splashCount]);
			splashCount++;
			break;
		
		case Start:
			break;
		
		case Up:
			if (i < 1) {
				i = 8;
			}
			else {
				i--;
			}
			updateGame(i, j);
			break;
		
		case Down:
			if (i > 7) {
				i = 0;
			}
			else {
				i++;
			}
			updateGame(i, j);
			break;
		
		case Left:
			if (j < 1) {
				j = 8;
			}
			else {
				j--;
			}
			updateGame(i, j);
			break;
		
		case Right:
			if (j > 7) {
				j = 1;
			}
			else {
				j++;
			}
			updateGame(i, j);
			break;
		
		case GameOver:
			if (gameOverCount == 0) {
				LCD_ClearScreen();
				LCD_DisplayString(1, "Game Over..");
				if (NEW_HIGH_SCORE) {
					LCD_DisplayString(17, "HIGH SCORE!!!");
					LCD_Cursor(31);
					LCD_WriteData(4);
				}
			}
			gameOverCount++;
			break;
	}
	
	return state;
}

#endif
