#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "input.h"
#include "display.h"
#include "mediactl.h"

enum radio_state {VOLUME, STATION};
static int input[2];

void term(int signum)
{
	input_quit();
	close(input[0]);
	mediactl_quit();
	display_write(1, " ");
	display_write(2, " ");
	display_quit();
    exit(0);
}

int main(void) {
	
	//Register Term-SIgnal
	struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
	
	display_init();
	
	mediactl_init();
	
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
	
	system("sudo halt");
	
} 
