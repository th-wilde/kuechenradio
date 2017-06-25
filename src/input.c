#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <wiringPi.h>

#include "input.h"

static int output_pipe;

void processPwrBtn(void)
{
	write(output_pipe, "P", 1);
}

void processModeBtn(void)
{
	write(output_pipe, "M", 1);
}

void processRightTurn(void)
{
	if(digitalRead(27)==digitalRead(17)==0){
		write(output_pipe, "R", 1);
	}
}

void processLeftTurn(void)
{
	if(digitalRead(27)==digitalRead(17)==0){
		write(output_pipe, "L", 1);
	}
}

void input_init(int init_output_pipe) {

	output_pipe = init_output_pipe;

	wiringPiSetupSys();
	
	wiringPiISR(27, INT_EDGE_SETUP, processRightTurn);
	wiringPiISR(17, INT_EDGE_SETUP, processLeftTurn);
	
	wiringPiISR( 4, INT_EDGE_SETUP, processPwrBtn);
	wiringPiISR(22, INT_EDGE_SETUP, processModeBtn);
}

void input_quit() {
	close(output_pipe);
}