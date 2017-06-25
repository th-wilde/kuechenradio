#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>

#define POWEROFFPIN 10

int main(int argc, char **argv){

	if(argc >= 2 && (strcmp(argv[1], "halt") == 0 || strcmp(argv[1], "poweroff") == 0)){
		wiringPiSetupGpio();
		pinMode(POWEROFFPIN, OUTPUT);
		sync();
		sleep(3);
		digitalWrite(POWEROFFPIN, HIGH);
		sleep(1);
	}
}
