#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "popen2.h"

pid_t popen2(char * command[], int *infp, int *outfp)
{
    int p_stdin[2], p_stdout[2];
	//int p_stderr = open("/dev/null", O_WRONLY); 
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
        return -1;

    pid = fork();

    if (pid < 0)
        return pid;
    else if (pid == 0)
    {
        close(p_stdin[1]);
        dup2(p_stdin[0], STDIN_FILENO);
        close(p_stdout[0]);
        dup2(p_stdout[1], STDOUT_FILENO);
		
		dup2(p_stdout[1], STDERR_FILENO);

        execvp(command[0], command);
        perror("execvp");
        exit(1);
    }

    if (infp == NULL)
        close(p_stdin[1]);
    else
        *infp = p_stdin[1];

    if (outfp == NULL)
        close(p_stdout[0]);
    else
        *outfp = p_stdout[0];
	
	//close(p_stderr);

    return pid;
}