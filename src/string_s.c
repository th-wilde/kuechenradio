#include <stdio.h>
#include <string.h>
#include "string_s.h"

void strcpy_s(char* str, int length, char *cpy){
	if(strlen(cpy) + 1 >= length){
		char truncate = cpy[length];
		cpy[length] = 0x00;
		strcpy(str, cpy);
		cpy[length] = truncate;
	}else{
		strcpy(str, cpy);
	}
}

void strcat_s(char* str, int length, char *cpy){
	strcpy_s(&str[strlen(str)], length, cpy);
}