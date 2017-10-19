/* Command Interpreter */

/* Includes */
#include <sys/ipc.h>		// Interprocess Comunication
#include <sys/shm.h>		// Shared Memory
#include <errno.h>		// Error
#include <sys/stat.h>		// Stat Definitions
#include <string.h>		// String
#include <sys/wait.h>		// Waitpid
#include <stdio.h>		// Input and Output
#include <stdlib.h>		// Library Definitions
#include <unistd.h>		// Symbolic Constants and Types
#include <sys/types.h>		// Types Definition

#include "errorControl.h"	// Error Control Definitions	

#define parametersMAX 5		// Define max number of parameters

enum returnConditions 	
{
	exitProgram	        =	0,
	userProgram		=	1,
	parametersExceeded	=	2,
	blank			=	3,
	ok			=       4
};

typedef enum returnConditions returnCond;			// Return conditions

void clearArguments(int* argc, char** argv);			// Clear arguments counter (ARGC) and arguments vector (ARGV)

returnCond interpretCmd (char* cmd, int* argc, char **argv);	// Command interpreter function

/* Main */
int main(int argc, char * argv[])
{
	// Declarations                                                                                                       
	char * scheduler_args[1] = { NULL } ;
	pid_t pid_cmdInterpreter, pid_scheduler;
	int status;

	pid_cmdInterpreter = getpid();
	
	pid_scheduler = fork(); // Create a new process to execute scheduler

	if (pid_scheduler > 0) // Interpreter Process
	{
		// Declarations                                                                                                       
	        int shmArea_programIdentifier;
		int shmSize_programIdentifier = 100*sizeof(char);
	        key_t key_programIdentifier = 8180;
	        char* programIdentifier;

                int shmArea_schedulerStatus;
		int shmSize_schedulerStatus = sizeof(int);
                key_t key_scheduler = 8181;
                int* schedulerStatus;

		int i;
		int errorControl;
		char cmd[30];
		returnCond state = ok;
		int ARGC;
		char **ARGV;

		// Initializations
		ARGV = (char**) malloc (parametersMAX*sizeof(char*));
		if(ARGV == NULL) // Fail verification
		{
			printf("\nmalloc error\n");
			exit(1);
		}

		// Program Identifier - Shared Memory - GET
		shmArea_programIdentifier = shmget( /* key */ key_programIdentifier, /* size */ shmSize_programIdentifier, /* flags */ IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR); 
		failVerification(shmArea_programIdentifier, shm_get);

		// Scheduler - Shared Memory - GET
		shmArea_schedulerStatus = shmget( /* key */ key_scheduler, /* size */ shmSize_schedulerStatus, /* flags */ IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR); 
		failVerification(shmArea_schedulerStatus, shm_get);

		// Program Identifier - Shared Memory - ATTACH
		programIdentifier = (char*) shmat( /* shared memory identifier */ shmArea_programIdentifier, /* shared memory adress */ NULL, /* flags */ 0); 
		if(programIdentifier == (char*) -1) { errorControl = -1; }
		failVerification(errorControl , shm_at); 

		// Scheduler - Shared Memory - ATTACH
		schedulerStatus = (int*) shmat( /* shared memory identifier */ shmArea_schedulerStatus, /* shared memory adress */ NULL, /* flags */ 0); 
		if(schedulerStatus == (int*) -1) { errorControl = -1; }
		failVerification(errorControl , shm_at);

		// Command Interpreter
		printf("Starting Interpreter\t<%d>\n", pid_cmdInterpreter);
                *schedulerStatus = 1; // Set scheduler status to continue

		while(state != exitProgram) // While dont exit
		{
			printf("> "); // Wait for command
			fflush(stdin);
			gets(cmd); // Get command
			state = interpretCmd (cmd, &ARGC, ARGV); // Interpret command
			printf("state = %d\n", state);
                        if(state == userProgram) // Command: Execute user program
			{
				if(strcmp(ARGV[0], "exec") == 0) // Assert
				{
					// Send new program identification to scheduler
					strcpy(cmd, ARGV[1]);
					for(i = 2; i < ARGC; i++)
					{
						strcat(cmd, "#");
						strcat(cmd, ARGV[i]);
					}
					printf("programIdentifier = %s\n", cmd);
					strcpy(programIdentifier, cmd); // New process in shared memory
					kill(pid_scheduler, SIGUSR1); // Warn scheduler
				}
			}
			else 
			{
				if(state == exitProgram) // Command: Exit
				{
					*schedulerStatus = 0; // Set scheduler status to exit
				}
			}
		}

		// Finalizations
                *schedulerStatus = 0; // Set scheduler status to exit
                waitpid(pid_scheduler, NULL, 0); // Wait for scheduler to end

		errorControl = shmdt( /* adress */ programIdentifier); // Program Identifier - Shared Memory - DEATTACH
		failVerification(errorControl, shm_dt);

		errorControl = shmdt( /* adress */ schedulerStatus); // Scheduler Status - Shared Memory - DEATTACH
		failVerification(errorControl, shm_dt);

		printf("Ending Interpreter\n");

		_exit(1); // Leave
	}
	else
	{	
		if (pid_scheduler == 0)	// Scheduler Process
		{
			pid_scheduler = getpid();
			printf("Starting Scheduler\t<%d>\n", pid_scheduler);

			execve( "./scheduler", scheduler_args, NULL); // Execute scheduler process
			perror("execve failed");

		}
		else // Fork failed
		{
			perror("fork failed");
			_exit(1); // Leave
	  	}
	}

	return 0;
}

/* Auxiliar Functions */

void clearArguments(int* argc, char** argv)	// Clear argc and argv
{
	int i;

	*argc = 0; // Reset arguments count

	for(i = 0; i < parametersMAX; i++) // Reset arguments vector
	{
		argv[i] = NULL;
	}	
}


returnCond interpretCmd (char* cmd, int* argc, char **argv)	// Command interpreter function
{
	int i, start = 0, end = 0;

	clearArguments(argc, argv);

	if(strcmp(cmd, "exit") == 0) // Exit command
	{
		return exitProgram;
	}
	
	// User program
	for(i = 0; i <= strlen(cmd); i++) // Read command line
	{

		if(cmd[i] == ' ' || cmd[i] == '(' || cmd[i] == ')' || cmd[i] == ',' || cmd[i] == '\0')
		{
			if(end - start > 0)
			{
				printf("start = %d \t end = %d\n", start, end);

				if(*argc + 1 > parametersMAX)
				{
					// Parameters verification	
					clearArguments(argc, argv);
					printf("\nWARNING: PARAMETERSMAX ERROR\n--- number of arguments exceeded\n\n");
					return parametersExceeded;
				}

				argv[*argc] = (char*) malloc ((end - start + 1)*sizeof(char));
				if(argv[*argc] == NULL)
				{
					printf("\nmalloc error\n");
					exit(1);
				}

				// Save argument in argv
				strncpy ( /* destination */ argv[*argc], /* source + beginIndex */ cmd + start, /* endIndex - beginIndex */ end - start);

				*argc += 1; // Update argc count

				start = i + 1;
			}
			else
			{
				if((i + 1) > strlen(cmd))
				{
					start = i;
				}
				else
				{
					start = i + 1;
				}
			}
		}
		else
		{
			if((i + 1) > strlen(cmd))
			{
				end = i;
			}
			else
			{
				end = i + 1;
			}
		}
	}

	return userProgram;
}

