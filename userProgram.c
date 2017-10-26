/* User program */

/* Includes */
#include <sys/wait.h>		// Waitpid
#include <stdio.h>		// Input and Output
#include <stdlib.h>		// Library Definitions
#include <unistd.h>		// Symbolic Constants and Types

int main (int argc, char** argv)
{
	int i, j;
	pid_t pid_user, pid_parent;

	pid_user = getpid(); // User process pid
	pid_parent = getppid(); // Scheduler process pid

	for(i = 0; i < argc; i++) // For each parameter
	{
		for(j = 0; j < atoi(argv[i]); j++) // Execute
		{
			printf("%d\n", pid_user);
			fflush(stdout);
			sleep(1);
		}
		if(i < argc - 1) // Between executions request I/0
		{
			kill(pid_parent, SIGUSR2); // Ask scheduler for I/O event
			kill(pid_user, SIGSTOP); // Assert 
		}
	}

	return 0;
}
