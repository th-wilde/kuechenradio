#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include "Lcd.h"
#include "string_s.h"
#include "display.h"

static pthread_mutex_t lcdUsage;
static char display_line1[DISPLAY_LINE_BUFFER];
static int display_offset1 = 0;
static char display_line2[DISPLAY_LINE_BUFFER];
static int display_offset2 = 0;
static pthread_t displayloop_thread;

void display_update(){
	pthread_mutex_lock(&lcdUsage);
	clearLcd();
	
	char lcdline[33];
	if(strlen(display_line1)<16){
		int i;
		for(i = strlen(display_line1); i<16; i++){
			display_line1[i+1] = '\0';
			display_line1[i] = ' ';
		}
	}
	
	int display_offset_fix1 = display_offset1;
	if(display_offset_fix1 < 0)
		display_offset_fix1 = 0;
	else if(display_offset_fix1 + 16 > strlen(display_line1))
		display_offset_fix1 = strlen(display_line1) -16;
	int display_offset_fix2 = display_offset2;
	if(display_offset_fix2 < 0)
		display_offset_fix2 = 0;
	else if(display_offset_fix2 + 16 > strlen(display_line2))
		display_offset_fix2 = strlen(display_line2) -16;
	
	strcpy_s(lcdline, 16, &display_line1[display_offset_fix1]);
	strcat_s(lcdline, 16, &display_line2[display_offset_fix2]);
	setLcd(0, 32, lcdline);
	
	pthread_mutex_unlock(&lcdUsage);
}

void *displayloop(void *arg)
{
	while(1) {
		usleep(500000);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		display_update();
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		
		display_offset1++;
		display_offset2++;
		
		if(display_offset1+16 > strlen(display_line1)+1){
			display_offset1=-1;
		}
			
		if(display_offset2+16 > strlen(display_line2)+1){
			display_offset2=-1;
		}
	}
}


void display_init() {

	initLcd();
	pthread_create(&displayloop_thread, NULL, displayloop, NULL);
}

void display_quit() {
	
	pthread_cancel(displayloop_thread);
	pthread_join(displayloop_thread, NULL);
}

void display_write(int line, char text[]){
	pthread_mutex_lock(&lcdUsage);
	switch(line){
		case 1:
			strcpy_s(display_line1, DISPLAY_LINE_BUFFER, text);
			initLcd();
			display_offset1 = -1;
		break;
		case 2:
			strcpy_s(display_line2, DISPLAY_LINE_BUFFER, text);
			initLcd();
			display_offset2 = -1;
		break;
	}
	pthread_mutex_unlock(&lcdUsage);
	display_update();
}