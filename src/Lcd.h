/*
 * Lcd.h
 *
 * Header-Datei für Lcd.c
 * Für Details siehe Lcd.c
 *
 * Created: 25.05.2013 16:44:40
 *  Author: Thorben Wilde
 */ 


#ifndef _LCD_H_
#define _LCD_H_

extern void initLcd();
extern void setLcd(int start, int length, char chars[]);
extern void numberToChars (int number, char chars[], int length);
extern void clearLcd();
extern void setCustomChar (int charIndex, int dotLine1, int dotLine2, int dotLine3, int dotLine4, int dotLine5, int dotLine6, int dotLine7, int dotLine8);

#endif /* LCD_H_ */