/* Scheduler */

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

#include "errorControl.h"
#include "queue.h"

char* program;							// Global variable that receives new programs by shared memory

int* status;							// Global variable that receives scheduler status to continue or end
	
Queue* L1;							// Global queue L1

Queue* L2;							// Global queue L2

Queue* L3;							// Global queue L3

char activeQueue[10];						// Global variable that saves the active queue round robing

PCB* removedProcess;						// Global removed process that represents the actual process running

void saveProcess (int signal);					// SIGUSR1 - Save Process

void childFinished (int signal);				// SIGCHLD - Verify if child finished or only stopped/continued	

void initializeNewProcess (char* newProcess);			// Create PCB for new process and push it to L1 queue

void executeProcess (PCB* process, int quantumTime);		// Execute new process or continue process execution

void mask();							// Mask/Block signals

void unmask();							// Unmask/Unblock signals

void roundRobin(Queue** q, int quantum, char* queueName);	// Round Robin for a queue with some quantum time

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
	int shmSize_program = 100*sizeof(char);
	int shmSize_status = sizeof(int);
	PCB* processAux;
	
	signal(SIGUSR1, saveProcess); // Define SIGUSR1
	signal(SIGCHLD, childFinished); // Define SIGCHLD

	// Initializations
	L1 = newQueue(L1);
	L2 = newQueue(L2);
	L3 = newQueue(L3);
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
		roundRobin(&L1, L1_quantum, "L1");
		roundRobin(&L2, L2_quantum, "L2");
		roundRobin(&L3, L3_quantum, "L3");
	}
	
	// Finalizations
	L1 = queueFreeAll(L1); // L1 - Queue - FREE
	L2 = queueFreeAll(L2); // L1 - Queue - FREE
	L3 = queueFreeAll(L3); // L1 - Queue - FREE
	
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

	printf("Scheduler end\n");

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

void childFinished (int signal)
{
	int pidEncerrado = waitpid(-1, NULL, WNOHANG);
	if (pidEncerrado == 0) // Process received a SIGSTOP/SIGCONT signal
	{
		return; // Then do nothing
	}
	else if (pidEncerrado == -1) // Something went wrong
	{
		printf("ERROR: PID ERROR\n");
		fflush(stdout);
		exit(1);
	}
	// Else child finished
	printf("Child %d finished!\n", pidEncerrado);
	fflush(stdout);
	if(getPCBPid(removedProcess) == pidEncerrado) // Assert
	{
		setPCBState (removedProcess, terminated); // Set process state to terminated
		free(removedProcess); // Destroy process
	}
	else // Something went wrong
	{
		printf("ERROR: LOST PROCESS\n--- Actual process running is not the one that finished.\n");
		fflush(stdout);
		exit(1);
	}
}

/* Auxiliar Functions */

void initializeNewProcess (char* newProcess)
{
	PCB* process = newPCB(newProcess);
	printf("newProcess: %s\n", getPCBName(process));
	L1 = queuePush(L1, process); // Put process in level 1 queue

	// TO DO: Treat when received a new process and L1 is empty
	/*	
	if(strcmp(activeQueue, "L1") != 0) // If queue round robing isnt L1
	{
		printf("Stopping actual process to start new");
		kill(getPCBPid(removedProcess), SIGSTOP); // Stop current process
		setPCBState(removedProcess, ready); // Set process state to ready
		if(strcmp(activeQueue, "L2") == 0)
	}
	*/
}

void executeProcess (PCB* process, int quantumTime)	// Execute new process or continue process execution
{
	char* args[1] = { NULL };
	pid_t pid_scheduler, pid_userProcess;
	char aux[30];

	if(getPCBState(process) == ready) // Continue user process
	{
		kill(getPCBPid(process), SIGCONT); // Continue process
		setPCBState(process, running); // Set process state to running
		sleep(quantumTime); // Let process run for a quantum time
		kill(getPCBPid(process), SIGSTOP); // Stop process
		setPCBState(process, ready); // Set process state to ready
	}
	else if(getPCBState(process) == new) // New process, start new execution
	{
		pid_userProcess = fork();
		if(pid_userProcess != 0) // Scheduler
		{
			setPCBPid(process, pid_userProcess); // Set new pid for process
			sleep(quantumTime); // Let process run for a quantum time
			kill(pid_userProcess, SIGSTOP); // Stop process execution
			setPCBState(process, ready); // Set process state to ready
		}
		else // New user program
		{
			pid_userProcess = getpid(); // Get new pid
			strcpy(aux, "./");
			strcat(aux, getPCBName(process));
			execve(aux, args, NULL); // Execute new process
			perror("execve failed");
			fflush(stdout);
		}
	}
	return;
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

void roundRobin(Queue** q, int quantum, char* queueName)	// Round Robin for a queue with some quantum time
{
	while(queueLength (*q) > 0 && *status == 1) // Check if queue has processes
	{
		strcpy(activeQueue, queueName);
		*q = queuePull(*q, &removedProcess); // Get next process to execute		
		mask();
		executeProcess (removedProcess, quantum); // Execute actual process
		unmask();
		*q = queuePush(*q, removedProcess); // Process didnt finish, then go back to queue
	}
	activeQueue[0] = '\0';
}

