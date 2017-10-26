/* Scheduler */

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
#include <sys/time.h>		// Time Definitions

#include "errorControl.h"	// Error Control Definitions	
#include "queue.h"		// Queue Definitions	

#define parametersMAX 100

typedef enum queueInform
{
	goback = 1,
	dontgoback = 0
} queueInfo;							// Queue situations

char* program;							// Global variable that receives new programs by shared memory

int* status;							// Global variable that receives scheduler status to continue or end
	
Queue* L1;							// Global queue L1

Queue* L2;							// Global queue L2

Queue* L3;							// Global queue L3

Queue* waitingIO;						// Global queue of processes waiting for I/O

char activeQueue[10];						// Global variable that saves the active queue round robing

PCB* removedProcess;						// Global removed process that represents the actual process running

PCB* processWaiting;						// Global variable to manipulate removed and pushed processes to waiting for I/0 queue
int pidFinished;

void saveProcess (int signal);					// SIGUSR1 - Save Process

void handleIO (int signal);					// SIGUSR2 - Waiting for I/O (w4IO)

void childFinished (int signal);				// SIGCHLD - Verify if child finished or only stopped/continued	

void initializeNewProcess (char* newProcess);			// Create PCB for new process and push it to L1 queue

queueInfo executeProcess (PCB* process, int quantumTime);	// Execute new process or continue process execution

void mask();							// Mask/Block signals

void unmask();							// Unmask/Unblock signals

//void roundRobin(Queue** q, int quantum);			// Round Robin for a queue with some quantum time

/* Main */
int main(int argc, char *argv[])
{
	// Declarations
	int L1_quantum = 1;
	int L2_quantum = 2*L1_quantum;
	int L3_quantum = 4*L1_quantum;
	int errorControl;
	int shmArea_program;
	int shmArea_status;
	key_t key_program = 8180;
	key_t key_status = 8181;
	int shmSize_program = 400*sizeof(char);
	int shmSize_status = sizeof(int);
	PCB* processAux;
	queueInfo situation;
	
	signal(SIGUSR1, saveProcess); // Define SIGUSR1
	signal(SIGUSR2, handleIO); // Define SIGUSR2
	signal(SIGALRM, handleIO); // Define SIGALRM
	signal(SIGCHLD, childFinished); // Define SIGCHLD

	// Initializations
	L1 = newQueue(L1);
	L2 = newQueue(L2);
	L3 = newQueue(L3);
	waitingIO = newQueue(waitingIO);
	// Program - Shared Memory - GET
	shmArea_program = shmget( /* key */ key_program, /* size */ shmSize_program, /* flags */ IPC_CREAT | S_IRUSR | S_IWUSR);
	failVerification(shmArea_program, shm_get);
 	
	// Status - Shared Memory - GET
	shmArea_status = shmget( /* key */ key_status, /* size */ shmSize_status, /* flags */ IPC_CREAT | S_IRUSR | S_IWUSR);
	failVerification(shmArea_status, shm_get);

	// Program - Shared Memory - ATTACH
	program = shmat( /* shared memory identifier */ shmArea_program, /* shared memory adress */ NULL, /* flags */ 0);
	if(program == (char*) -1) { errorControl = -1; }
	failVerification(errorControl , shm_at); // Fail verification

	// Status - Shared Memory - ATTACH
	status = shmat( /* shared memory identifier */ shmArea_status, /* shared memory adress */ NULL, /* flags */ 0);
	if(status == (int*) -1) { errorControl = -1; }
	failVerification(errorControl , shm_at);
	
	// Scheduler Actions
	
	while(*status == 1)
	{

		if(queueLength(L1) > 0) // Check if L1 has processes
		{
			printf("L1\n");
			L1 = queuePull(L1, &removedProcess); // Get next process to execute		
			mask();
			situation = executeProcess (removedProcess, L1_quantum); // Execute actual process
			unmask();
			if(situation == goback)
			{
				if(getPCBState(removedProcess) != terminated) // Assert
				{
					setPCBState(removedProcess, ready); // Set process state to ready
					setPCBQueue(removedProcess, "L2");
					L2 = queuePush(L2, removedProcess); // Process didnt finish, then go back to queue L2
				}
			}
		}
		else if(queueLength(L2) > 0) // Check if L2 has processes
		{
			printf("L2\n");
			L2 = queuePull(L2, &removedProcess); // Get next process to execute		
			mask();
			situation = executeProcess (removedProcess, L2_quantum); // Execute actual process
			unmask();
			if(situation == goback)
			{
				if(getPCBState(removedProcess) != terminated) // Assert
				{
					setPCBQueue(removedProcess, "L3");
					setPCBState(removedProcess, ready); // Set process state to ready
					L3 = queuePush(L3, removedProcess); // Process didnt finish, then go back to queue L3
				}
			}
		}
		else if(queueLength(L3) > 0) // Check if L3 has processes
		{	
			printf("L3\n");
			L3 = queuePull(L3, &removedProcess); // Get next process to execute		
			mask();
			situation = executeProcess (removedProcess, L3_quantum); // Execute actual process
			unmask();
			if(situation == goback)
			{
				if(getPCBState(removedProcess) != terminated)
				{	
					setPCBQueue(removedProcess, "L3");
					setPCBState(removedProcess, ready); // Set process state to ready
					L3 = queuePush(L3, removedProcess); // Process didnt finish, then go back to queue L3
				}
			}
		}

	}
	
	// Finalizations
	queuePrint(L1);
	queuePrint(L2);
	queuePrint(L3);
	queuePrint(waitingIO);
	L1 = queueFreeAll(L1); // L1 - Queue - FREE
	L2 = queueFreeAll(L2); // L1 - Queue - FREE
	L3 = queueFreeAll(L3); // L1 - Queue - FREE
	waitingIO = queueFreeAll(waitingIO);
	
	errorControl = shmdt( /* adress */ program); // Program - Shared Memory - DEATTACH
	failVerification(errorControl, shm_dt);

	errorControl = shmdt( /* adress */ status); // Status - Shared Memory - DEATTACH
	failVerification(errorControl, shm_dt);

	// Program - Shared Memory - REMOVE
	errorControl = shmctl( /* shared memory identifier */ shmArea_program, /* command */ IPC_RMID, /* pointer to data structure */ NULL);
	failVerification(errorControl, shm_ctl);

	// Status - Shared Memory - REMOVE
	errorControl = shmctl( /* shared memory identifier */ shmArea_status, /* command */ IPC_RMID, /* pointer to data structure */ NULL);
	failVerification(errorControl, shm_ctl);

	printf("\n***Scheduler end***\n");

	return 0;
}

/* Signal Handlers */

void saveProcess (int signal)
{
	printf("Signal %d received! Preparing to save process!\n", signal);
	fflush(stdout);
	if(strlen(program) > 0) // Received new process
	{
		initializeNewProcess(program);
		program[0] = '\0'; // Clear shared memory
	}
}

void handleIO (int signal)
{
	char removedFrom[10]; // Manipulate from which queue process was removed
	struct timeval t1; // Actual process I/0 start time
	struct timeval t2; // Next process I/O start time
	unsigned long long t; // Next alarm time

	switch(signal) {
		case SIGUSR2:
			processWaiting = removedProcess;
			kill(getPCBPid(processWaiting), SIGSTOP);
			setPCBState(processWaiting, waiting);
//			printf("Process waiting for I/O\n");
//			fflush(stdout);
			gettimeofday(&t1, NULL);
			setPCBTimeStructure(processWaiting, t1);		
			if(queueLength(waitingIO) == 0) // Only one process waiting for I/O
			{
				ualarm(3000, 0); // Start Alarm
			}
			waitingIO = queuePush(waitingIO, processWaiting); // Push process to waiting for I/O queue
			break;
		case SIGALRM:
			waitingIO = queuePull(waitingIO, &processWaiting); // Pull process from waiting for I/O queue
			if(queueLength(waitingIO) > 0) // Still have processes waiting for I/O
			{
				getPCBTimeStructure(processWaiting, &t1.tv_sec, &t1.tv_usec);
				queueNextTime(waitingIO, &t2.tv_sec, &t2.tv_usec);
				t = 1000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000;
				if(t == 0)
					t = 100; // Assert to make sure process will finish waiting
				ualarm(t, 0);
//				printf("t = %llu\n", t);
			}
//			printf("I/O received for process %d\n", getPCBPid(processWaiting));
			setPCBState(processWaiting, ready);
			strcpy(removedFrom, getPCBQueue(processWaiting));
			if(strcmp(removedFrom, "L1") == 0 || strcmp(removedFrom, "L2") == 0)
			{
				setPCBQueue(processWaiting, "L1");
				L1 = queuePush(L1, processWaiting); // Put process in level 1 queue
			}
			else if (strcmp(removedFrom, "L3") == 0)
			{
				setPCBQueue(processWaiting, "L2");
				L2 = queuePush(L2, processWaiting); // Put process in level 2 queue
			}
			else
			{
				printf("ERROR: QUEUE\n--- Cant find from which queue process was removed\n");
				printf("To prevent error, go back to level 1 queue\n");
				setPCBQueue(processWaiting, "L1");
				L1 = queuePush(L1, processWaiting);
			}
			processWaiting = NULL;
			break;
		default:
			printf("Something went really really wrong here, signals are going crazy\n");
			*status = 0; // Force program to exit
	}
}

void childFinished (int signal)
{
	pidFinished = waitpid(-1, NULL, WNOHANG);
	if(pidFinished > 0)
	{
		// Else child finished
		if(getPCBPid(removedProcess) == pidFinished) // Assert
		{
			setPCBState (removedProcess, terminated); // Set process state to terminated
			free(removedProcess); // Destroy process
			printf("Child %d finished!\n", pidFinished);
			fflush(stdout);
		}
		else // Something went wrong
		{
			printf("ERROR: LOST PROCESS\n--- Actual process running %d is not the one that finished.\n", pidFinished);
			fflush(stdout);
			*status = 0; // Force program to exit
		}
	}
	else if (pidFinished == 0) // Process received a SIGSTOP/SIGCONT signal
	{
		return; // Then do nothing
	}
	else if (pidFinished == -1) // Something went wrong
	{
		printf("ERROR: PID ERROR\n");
		fflush(stdout);
		exit(1);
	}
}

/* Auxiliar Functions */

void clearArguments(int* argc, char** argv)	// Clear argc and argv
{
	int i;

	*argc = 0; // Reset arguments count

	for(i = 0; i < parametersMAX - 1; i++) // Reset arguments vector
	{
		argv[i] = NULL;
	}	
}

void breakNewProcess(char* newProcess, int* argc, char** argv, char* processName)
{
	int i, start = 0, end = 0;

	clearArguments(argc, argv);

	for(i = 0; i <= strlen(newProcess); i++) // Read command line
	{

		if(newProcess[i] == '#' || newProcess[i] == '\0')
		{
			if(end - start > 0)
			{
//				printf("start = %d \t end = %d\n", start, end);

				if(start == 0)
				{
					strncpy ( /* destination */ processName, /* source + beginIndex */ newProcess + start, /* endIndex - beginIndex */ end - start);
				}
				else
				{
					argv[*argc] = (char*) malloc ((end - start + 1)*sizeof(char));
					if(argv[*argc] == NULL)
					{
						printf("\nmalloc error\n");
						exit(1);
					}

					// Save argument in argv
					strncpy ( /* destination */ argv[*argc], /* source + beginIndex */ newProcess + start, /* endIndex - beginIndex */ end - start);

					*argc += 1; // Update argc count
				}
				start = i + 1;
			}
			else
			{
				if((i + 1) > strlen(newProcess))
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
			if((i + 1) > strlen(newProcess))
			{
				end = i;
			}
			else
			{
				end = i + 1;
			}
		}
	}

//	printf("name = %s\n", processName);
//	for(i = 0; i < *argc; i++)
//		printf("argv[%d] = %s\n", i, argv[i]);
}

void initializeNewProcess (char* newProcess)
{
	PCB* process;
	int argc;
	char* processName = (char*) malloc (strlen(newProcess)*sizeof(char));
	char** argv = (char**) malloc ((parametersMAX - 1)*sizeof(char*));
	if(argv == NULL) // Fail verification
	{
		printf("\nmalloc error\n");
		exit(1);
	}
	breakNewProcess(newProcess, &argc, argv, processName);
	process = newPCB(processName, argc, argv);
	setPCBQueue(process, "L1");
	L1 = queuePush(L1, process); // Put process in level 1 queue
//	printf("newProcess: %s\n", getPCBName(process));
}

queueInfo executeProcess (PCB* process, int quantumTime)	// Execute new process or continue process execution
{
	//char* args[2] = { "hello", NULL };
	int execError;
	pid_t pid_scheduler, pid_userProcess;
	char aux[30];

	if(pidFinished > 0 && getPCBPid(process) == pidFinished) // Assert
	{
		setPCBState(process, terminated);
	}

	if(getPCBState(process) == ready) // Continue user process
	{
		kill(getPCBPid(process), SIGCONT); // Continue process
		setPCBState(process, running); // Set process state to running
		printf("Running process\n");
		sleep(quantumTime); // Let process run for a quantum time
		if(getPCBState(process) == running)
		{
			kill(getPCBPid(process), SIGSTOP); // Stop process
			printf("Quantum time's up! Stopping process\n");		
			return goback;
		}
		else if (getPCBState(process) == waiting)
		{
			printf("Process asked for IO\n");
			return dontgoback;
		}
		else if (getPCBState(process) == terminated)
		{
			printf("Process terminated\n");
			free(process);
			return dontgoback;
		}
		else
		{
			printf("Something went really really wrong here, process was running but got confused with finishing or IO event\n");
			*status = 0; // Force program to exit
			free(process);
			return dontgoback;
		}
	}
	else if(getPCBState(process) == new) // New process, start new execution
	{
		pid_userProcess = fork();
		if(pid_userProcess != 0) // Scheduler
		{
			setPCBPid(process, pid_userProcess); // Set new pid for process
			sleep(quantumTime); // Let process run for a quantum time
			if(getPCBState(process) == new)
			{
				kill(pid_userProcess, SIGSTOP); // Stop process execution
				printf("Quantum time's up! Stopping new process\n");
				return goback;
			}
			else if (getPCBState(process) == waiting)
			{
				printf("Process asked for IO\n");
				return dontgoback;
			}
			else if (getPCBState(process) == terminated)
			{
				printf("Process terminated\n");
				free(process);
				return dontgoback;
			}
			else
			{
				printf("Something went really really wrong here, process was running but got confused with finishing or IO event\n");
				*status = 0; // Force program to exit
				free(process);
				return dontgoback;
			}
		}
		else // New user program
		{
			pid_userProcess = getpid(); // Get new pid
			strcpy(aux, "./");
			strcat(aux, getPCBName(process));
			execve(aux, getPCBArgv(process), NULL); // Execute new process
			perror("execve failed");
			fflush(stdout);
			printf("Erro ao executar programa, liberando...\n");
			free(process);
			return dontgoback;
		}
	}
	return goback;
}

void mask()	// Mask/Block signals
{
	sigset_t signal_set;
	int errorControl;

	errorControl = sigemptyset(&signal_set); // Create empty signals set
	failVerification(errorControl, sig_emptyset);	
	
	errorControl = sigaddset(&signal_set, SIGCHLD); // Add signal SIGCHLD to set
	failVerification(errorControl, sig_addset);
	
	errorControl = sigprocmask(SIG_BLOCK, &signal_set, NULL); // Block all signals at signals set
	failVerification(errorControl, sig_procmask);
}

void unmask()	// Unmask/Unblock signals
{
	sigset_t signal_set;
	int errorControl;

	errorControl = sigemptyset(&signal_set); // Create empty signals set
	failVerification(errorControl, sig_emptyset);
	
	errorControl = sigaddset(&signal_set, SIGCHLD); // Add signal SIGCHLD to set
	failVerification(errorControl, sig_addset);
	
	errorControl = sigprocmask(SIG_UNBLOCK, &signal_set, NULL); // Unblock all signals at signals set
	failVerification(errorControl, sig_procmask);
}


/*void roundRobin(Queue** q, int quantum)	// Round Robin for a queue with some quantum time
{
	while(queueLength (*q) > 0 && *status == 1) // Check if queue has processes
	{
		*q = queuePull(*q, &removedProcess); // Get next process to execute		
		mask();
		executeProcess (removedProcess, quantum); // Execute actual process
		unmask();
		*q = queuePush(*q, removedProcess); // Process didnt finish, then go back to queue
	}
}
*/
