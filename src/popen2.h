#ifndef _POPEN2_H_
#define _POPEN2_H_
#include <unistd.h>
#include <sys/types.h>

extern pid_t popen2(char * command[], int *infp, int *outfp);

#endif