#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "input.h"
#include "display.h"
#include "mediactl.h"

enum radio_state {VOLUME, STATION};

int main(void) {
	
	display_init();
	
	mediactl_init();
	
	int input[2];
	pipe(input);
	
	input_init(input[1]);
	
	char readedData[] = "_";
	
	enum radio_state currentState;
	currentState = VOLUME;
	
	char line_vol[17];
	char line_station[40];
	
	do{
		switch(currentState){
			case VOLUME:
				switch(readedData[0]){
					case 'P':
						input_quit();
						close(input[0]);
					break;
					case 'M':
						currentState = STATION;
					break;
					case 'L':
						mediactl_vol_down();
					break;
					case 'R':
						mediactl_vol_up();
					break;
				}
			break;
			case STATION:
				switch(readedData[0]){
					case 'P':
						input_quit();
						close(input[0]);
					break;
					case 'M':
						currentState = VOLUME;
					break;
					case 'L':
						mediactl_station_previous();
					break;
					case 'R':
						mediactl_station_next();
					break;
				}
			break;
		}
		switch(currentState){
			case VOLUME:
				sprintf(line_vol, "Lautst\xE1rke: %3d%%", mediactl_vol());
				display_write(2, line_vol);
			break;
			case STATION:
				sprintf(line_station, "Sender: %s", station_name);
				display_write(2, line_station);
			break;
		}
	}while(read(input[0], readedData, 1)>0);
	
	mediactl_quit();
	
	display_write(1, "Tsch\xF5ss!        ");
	display_write(2, "Schalte aus...  ");
	
	display_quit();
	
	//system("sudo halt");
	
} 
