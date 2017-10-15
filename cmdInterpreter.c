/* Command Interpreter */

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

#define parametersMAX 5

/* Enumerations */
typedef enum {
	// Shared Memory Error Identifier
	shm_get	=	1,
	shm_at	=	2,
	shm_dt	=	3,
	shm_ctl	=	4
} shmError;

typedef enum {
	// Return conditions Enum
	exitProgram	        =	0,
	userProgram		=	1,
	parametersExceeded	=	2,
	blank			=	3,
	ok			=       4
} returnCond;

/* Functions */
void shmVerification(int errorControl, shmError e)	// Shared Memory Error Verification
{
	if(errorControl != -1) {
		// Everything is okay
		return;
	}

	printf("\nERROR: SHARED MEMORY ERROR\n"); // Shared memory error
	switch(e)
	{
		// Specify error
		case shm_get:
			perror("--- shmget: shmget failed");
			break;
		case shm_at:
			perror("--- shmat: shmat failed");
			break;
		case shm_dt:
			perror("--- shmdt: shmdt failed");
			break;
		case shm_ctl:
			perror("--- shmctl: shmctl failed");
			break;
		default:
			perror("--- shm failed");
	}
	printf("\n\n");
	exit(1); // Leave
}

void clearArguments(int* argc, char** argv)	// Clear argc and argv
{
	int i;

	// Reset arguments count
	*argc = 0;

	// Reset arguments vector
	for(i = 0; i < parametersMAX; i++)
	{
		argv[i] = NULL;
	}	
}

returnCond interpretCmd (char* cmd, int* argc, char **argv)	// Command interpreter function
{
	int i, start = 0, end;

	clearArguments(argc, argv);

	if(strcmp(cmd, "exit") == 0) {
		// Exit command
		return exitProgram;
	}
	
	// User program
	for(i = 0; i <= strlen(cmd); i++) // Read command line
	{
		if((cmd[i] == ' ' || cmd[i] == '\0') && cmd[i - 1] != ' '  && i != 0) // Split arguments by spaces
		{
			end = i;

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
			strncpy (
				/* destination */ argv[*argc],
				/* source + beginIndex */ cmd + start,
				/* endIndex - beginIndex */ end - start);

			start = end + 1;
			
			*argc += 1; // Update argc count
		}

		if(cmd[i] == ' ' && cmd[i - 1] == ' ') // Treat sequence of spaces
		{
			start = i + 1;
		}	
	}

	return userProgram;
}

returnCond executeCmd (int argv, char** argc)	// Execute command function
{
	if(argv == 0) {
		// No commands to execute
		return blank;
	}

	if(strcmp(argc[0], "exec") == 0)
	{
		// Call scheduler
	}

	return ok;
}

/* Main */
int main(int argc, char * argv[])
{
	// Declarations                                                                                                       
	char * scheduler_args[1] = { NULL } ;
	pid_t pid_cmdInterpreter, pid_scheduler;
	int status;

	pid_cmdInterpreter = getpid();
	
	// Create a new process to execute scheduler
	pid_scheduler = fork();

	if (pid_scheduler > 0)
	{
		// Interpreter Process
		// Declarations                                                                                                       
	        int shmArea_programIdentifier;
		int shmSize_programIdentifier = 100*sizeof(char);
	        key_t key_programIdentifier = 8180;
                key_t key_scheduler = 8181;
		int i;
		int errorControl;
		char cmd[30];
		returnCond state = ok;
		int ARGC;
		char **ARGV;
                int* schedulerStatus;
                int area_scheduler;
	        char* programIdentifier;

		printf("Starting Interpreter\t<%d>\n", pid_cmdInterpreter);

		ARGV = (char**) malloc (parametersMAX*sizeof(char*));
		if(ARGV == NULL) // Fail verification
		{
			printf("\nmalloc error\n");
			exit(1);
		}

		// Program Identifier - Shared Memory - GET
		shmArea_programIdentifier = shmget(
			/* key */ key_programIdentifier, 
			/* size */ shmSize_programIdentifier, 
			/* flags */ IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
		shmVerification(shmArea_programIdentifier, shm_get); // Fail verification

		// Program Identifier - Shared Memory - ATTACH
		programIdentifier = (char*) shmat( 
			/* shared memory identifier */ shmArea_programIdentifier,
			/* shared memory adress */ NULL,
			/* flags */ 0);
		if(programIdentifier == (char*) -1)
			errorControl = -1;
		shmVerification(errorControl , shm_at); // Fail verification

        	area_scheduler = shmget(key_scheduler, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	        if(area_scheduler == -1) {
        		printf("Erro alocacao memoria compartilhada.\n");
	        	shmctl(area_scheduler, IPC_RMID, 0);
		        exit(1);
	        }
        	schedulerStatus = (int*) shmat (area_scheduler, 0, 0);
                *schedulerStatus = 1;

		while(state != exitProgram) // Command interpreter
		{
			printf("> "); // Wait for command
			fflush(stdin);
			gets(cmd); // Get command
			state = interpretCmd (cmd, &ARGC, ARGV); // Interpret command
                        if(state == userProgram)
			{
				if(ARGV[0] != NULL) // Assert
				{
					// Send new program identification to scheduler
					strcpy(cmd, ARGV[0]);
					for(i = 1; i < ARGC; i++)
					{
						strcat(cmd, ARGV[i]);
					}
					strcpy(programIdentifier, cmd);
				}
			}
			if(state == exitProgram)
				*schedulerStatus = 0; // Set scheduler status to exit
		}

                *schedulerStatus = 0; // Set scheduler status to exit
                waitpid(pid_scheduler, NULL, 0); // Wait for scheduler to end

		// Program Identifier - Shared Memory - DEATTACH
		errorControl = shmdt(
			/* adress */ programIdentifier);
		shmVerification(errorControl, shm_dt); 	// Fail verification

		// Scheduler Status Identifier - Shared Memory - DEATTACH
		errorControl = shmdt(
			/* adress */ schedulerStatus);
		shmVerification(errorControl, shm_dt); // Fail verification

		_exit(1); // Leave
	}
	else
	{	
		if (pid_scheduler == 0)
		{
			// Scheduler Process
			pid_scheduler = getpid();
			printf("Starting Scheduler\t<%d>\n", pid_scheduler);

			execve( "./scheduler", scheduler_args, NULL);
			perror("execve failed");

		}

		else
		{
			// Fail verification
			perror("fork failed");
			_exit(1);
	  	}
	}

	return 0;
}
