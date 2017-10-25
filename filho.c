/* Includes */
#include <sys/ipc.h>	// Interprocess Comunication
#include <sys/shm.h>	// Shared Memory
#include <errno.h>	// Error
#include <sys/stat.h>	// Stat Definitions
#include <string.h>	// String
#include <sys/wait.h>	// Waitpid
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main (int argc, char** argv)
{
	int i, j;
	pid_t pid_user, pid_parent;

	// User Process
	pid_user = getpid();
	pid_parent = getppid();

//	printf("User Process %d starting!\n", pid_user);
//	fflush(stdout);

	for(i = 0; i < argc; i++)
	{
		for(j = 0; j < atoi(argv[i]); j++)
		{
			//printf("Executing here!\t<%d>\n", pid_user);
			printf("%d\n", pid_user);
			fflush(stdout);
			sleep(1);
		}
		if(i < argc - 1) // Between executions request I/0
		{
			kill(pid_parent, SIGUSR2); // I/O event
			kill(pid_user, SIGSTOP); // I/O event
		}
	}

//	printf("User Process %d ended!\n", pid_user);
//	fflush(stdout);

	return 0;
}
