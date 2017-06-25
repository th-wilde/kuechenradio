/*
 * Lcd.c
 *
 * Diese Datei regelt die Kommunikation mit dem LCD-Display (KS0070B oder Kompatibel)
 * Das LCD wird im 4-Bit-Modus betrieben
 *
 * Created: 25.05.2013 16:44:01
 *  Author: Thorben Wilde
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <stdbool.h>

#include "Lcd.h"


//Konstante für den PORT an den das LCD angeschlossen ist.
//Pinbelegung (Port = Display):
//0-3 = D4-D7 (Datenbus)
//4 = RS (Register Select)
//5 = E (Enable)
#define LCD_PORT lcd_port //Port-Register
#define LCD_DDR DDRD //Data-Direction-Register

void _delay_ms(int delay){
	usleep(delay*1000);
}

void _delay_us(int delay){
	usleep(delay);
}

//Sendet sowohl Befehle als auch Daten an das LCD
void sendLcdData(int data){
	digitalWrite(18, (data & 0x10) > 0);
	digitalWrite(24, (data & 0x01) > 0);
	digitalWrite(25, (data & 0x02) > 0);
	digitalWrite( 8, (data & 0x04) > 0);
	digitalWrite( 7, (data & 0x08) > 0);
	
	digitalWrite(23, 1);//Enable-Signal
	_delay_us(1); //Kurz warten, damit das Display den Befehl/Daten erkennt.
	digitalWrite(23, 0);//Enable-Signal entfernen
}

//Setzt den Cursor des LCD an die angegebene Position
//Sprint nach dem 16 Zeichen in die 2. Zeile
void setLcdCursorPos(int pos){
	if(pos<16){
		sendLcdData(0x08); //Erste Zeile
		sendLcdData(pos);
	}else{
		sendLcdData(0x0C); //Zweite Zeile
		sendLcdData(pos-16);
	}
	_delay_us(50); //Zeit die das Display zum verarbeiten benötigttop
	
}

//Konfiguriert den PORT und Inizialisiert das LCD-Display
void initLcd(){
	
	//Init wiringPi-Lib
	wiringPiSetupSys();
	
	//Data-Directionregister für Port D Pin 0-6 (LCD)
	//LCD_DDR |= (1<<PD0) | (1<<PD1) | (1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5);
	
	/*pinMode (23, OUTPUT);
	pinMode (18, OUTPUT);
	pinMode (24, OUTPUT);
	pinMode (25, OUTPUT);
	pinMode ( 8, OUTPUT);
	pinMode ( 7, OUTPUT);*/
	
	
	//Inizialisierungs-Sequenz für 4-Bit-Modus
	_delay_ms(50);
	sendLcdData(0x03);
	_delay_ms(5);
	sendLcdData(0x03);
	_delay_ms(1);
	sendLcdData(0x03);
	_delay_ms(1);
	sendLcdData(0x02); //4 Bit-Modus
	_delay_ms(1);
	sendLcdData(0x02); //4 Bit-Modus
	sendLcdData(0x08); //2 Zeilen-Modus 5x7 Punkte
	_delay_ms(1);
	sendLcdData(0x00);
	sendLcdData(0x0C); // Display ein, cursor und blink aus
	_delay_us(50);
	sendLcdData(0x00);
	sendLcdData(0x01); // Lösche anzeige
	_delay_ms(2);
	sendLcdData(0x00);
	sendLcdData(0x06); // Increment-Mode, Shift aus
	_delay_ms(2);
	
}

void clearLcd(){
	sendLcdData(0x00);
	sendLcdData(0x01); // Lösche anzeige
	_delay_ms(2);
}


//Schreibt auf das LCD
// -int start = Start-Position ab der geschrieben werden soll
// -int length = Menge der zu schreibenden Buchstaben
// -char[]/string chars = Buchstaben als Char-Array/String
void setLcd(int start, int length, char chars[]){
	setLcdCursorPos(start); //Startposition setzen
	int i;
	bool endOfString = false;
	for(i = 0; i < length; i++ ){
		char current = chars[i];
		if(current == 0x00 || endOfString){
			current = ' ';
			endOfString = true;
		}
		
		//Zeilenumbruch am Ende der ersten Zeile
		if(start < 16 && start + i == 16){
			setLcdCursorPos(16);
		}
		sendLcdData(0x10 | ((current >> 4) & 0xF)); //RS-Bit & erste hälfte des Bytes
		sendLcdData(0x10 | (current & 0xF));	//RS-Bit & zweite hälfte des Bytes
		_delay_us(50); //Zeit die das Display zum verarbeiten benötigt
	}
}

//Wandelt eine Int-Zahl in die entsprechenden Char-Ziffern um
// -int number = Umzuwandelne Int-Zahl
// -char[]/string = Char-Array/String in dem die umgewandelte Int-Zahl gespeichert werden soll (länge muss >= der Int-Zahl-Stellen sein)
// -int length = Länge des Char-Array/String
void numberToChars (int number, char chars[], int length){
	length--;
	long dezPos = 1;
	int pos;
	for(pos = length; pos >= 0; pos--){
		chars[pos] = 48 + ( number % (dezPos*10) ) / (dezPos);
		if(chars[pos] == 48 && pos != length && number / (dezPos*10) == 0){
			chars[pos] = 32;
		}
		dezPos *= 10;
	}
}

void setCustomChar (int charIndex, int dotLine1, int dotLine2, int dotLine3, int dotLine4, int dotLine5, int dotLine6, int dotLine7, int dotLine8){

	if(charIndex >= 0 && charIndex<=7){
	
		//CGRAM-Address
		
		int charIndexCGRAMPos = 0x04 | ((charIndex >> 1) & 0xF);
		int baseCGRAMPos = (charIndex << 3) & 0xF;
		
		sendLcdData(charIndexCGRAMPos);
		sendLcdData(baseCGRAMPos);
		usleep(50);
		
		sendLcdData(0x10 | ((dotLine1 >> 4) & 0xF)); //RS-Bit & erste hälfte des Bytes
		sendLcdData(0x10 | (dotLine1 & 0xF));	//RS-Bit & zweite hälfte des Bytes
		
		usleep(50);
		sendLcdData(charIndexCGRAMPos);
		sendLcdData(baseCGRAMPos | 1);
		usleep(50);
		
		sendLcdData(0x10 | ((dotLine2 >> 4) & 0xF)); //RS-Bit & erste hälfte des Bytes
		sendLcdData(0x10 | (dotLine2 & 0xF));	//RS-Bit & zweite hälfte des Bytes
		
		usleep(50);
		sendLcdData(charIndexCGRAMPos);
		sendLcdData(baseCGRAMPos | 2);
		usleep(50);
		
		sendLcdData(0x10 | ((dotLine3 >> 4) & 0xF)); //RS-Bit & erste hälfte des Bytes
		sendLcdData(0x10 | (dotLine3 & 0xF));	//RS-Bit & zweite hälfte des Bytes
		
		usleep(50);
		sendLcdData(charIndexCGRAMPos);
		sendLcdData(baseCGRAMPos | 3);
		usleep(50);
		
		sendLcdData(0x10 | ((dotLine4 >> 4) & 0xF)); //RS-Bit & erste hälfte des Bytes
		sendLcdData(0x10 | (dotLine4 & 0xF));	//RS-Bit & zweite hälfte des Bytes
		
		usleep(50);
		sendLcdData(charIndexCGRAMPos);
		sendLcdData(baseCGRAMPos | 4);
		usleep(50);
		
		sendLcdData(0x10 | ((dotLine5 >> 4) & 0xF)); //RS-Bit & erste hälfte des Bytes
		sendLcdData(0x10 | (dotLine5 & 0xF));	//RS-Bit & zweite hälfte des Bytes
		
		usleep(50);
		sendLcdData(charIndexCGRAMPos);
		sendLcdData(baseCGRAMPos | 5);
		usleep(50);
		
		sendLcdData(0x10 | ((dotLine6 >> 4) & 0xF)); //RS-Bit & erste hälfte des Bytes
		sendLcdData(0x10 | (dotLine6 & 0xF));	//RS-Bit & zweite hälfte des Bytes
		
		usleep(50);
		sendLcdData(charIndexCGRAMPos);
		sendLcdData(baseCGRAMPos | 6);
		usleep(50);
		
		sendLcdData(0x10 | ((dotLine7 >> 4) & 0xF)); //RS-Bit & erste hälfte des Bytes
		sendLcdData(0x10 | (dotLine7 & 0xF));	//RS-Bit & zweite hälfte des Bytes
		
		usleep(50);
		sendLcdData(charIndexCGRAMPos);
		sendLcdData(baseCGRAMPos | 7);
		usleep(50);
		
		sendLcdData(0x10 | ((dotLine8 >> 4) & 0xF)); //RS-Bit & erste hälfte des Bytes
		sendLcdData(0x10 | (dotLine8 & 0xF));	//RS-Bit & zweite hälfte des Bytes
		
		usleep(50);
		
	}
}
