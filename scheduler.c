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

/* Enumerations */
typedef enum processStateType {
	// Process States Type
	terminated = 0,	
	new = 1,
	ready = 2,
	running = 3,
	waiting = 4
} stateType;

typedef enum {
	// Shared Memory Error Identifier
	shm_get	=	1,
	shm_at	=	2,
	shm_dt	=	3,
	shm_ctl	=	4
} shmError;

/* Structures */
typedef struct processControlBlock {
	// Process Control Block
	pid_t pid;
	char name[30];
	stateType state;
} PCB;

typedef struct queue
{
	// Process Queue
	//char process[30];
	PCB* pcb;
	struct queue * next;
} Queue;

/* Shared Memory Functions */
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

/* Process Block Control Functions */

PCB* newPCB (char* processName)	// Create a new process control block
{
	PCB* pcb = (PCB*) malloc (sizeof(PCB));
	pcb->pid = 0;
	strcpy(pcb->name, processName);
	pcb->state = new;
	return pcb;
}

char* getPCBName (PCB* pcb)	// Get process name
{
	return pcb->name;
}

void setPCBName (PCB* pcb, char* name)	// Set process name
{
	strcpy(pcb->name, name);
}

stateType getPCBState (PCB* pcb)	// Get process state
{
	return pcb->state;
}

void setPCBState (PCB* pcb, stateType state)	// Set process state
{
	pcb->state = state;
}

pid_t getPCBPid (PCB* pcb)	// Get process pid
{
	return pcb->pid;
}

pid_t setPCBPid (PCB* pcb, pid_t pid)	// Set process pid
{
	pcb->pid = pid;
}

/* Queue Functions */

Queue* newQueue(Queue* q)	// Create new processes queue
{
	q = (Queue*) malloc (sizeof(Queue));
	q = NULL;
	return q;
}

int queueLength (Queue* q)	// Calculate the length of the queue
{
	int i = 0;
	Queue* p = q;
	for(p = q; p != NULL; p = p->next)
		i++;
	return i;	
}

Queue* queuePush(Queue* q, PCB* newProcess)	// Push process to queue
{
	Queue * p = q;
	Queue* new = (Queue*) malloc (sizeof(Queue));
	//strcpy(new->process, newProcess);
	new->pcb = newProcess;
	new->next = NULL;

	if(q == NULL)
		q = new;
	else {
		while(p->next != NULL)
		{
			p = p->next;
		}
		p->next = new;
	}

	return q;
}

Queue* queuePull(Queue* q, PCB* removedProcess)	// Pull process from queue
{
	//char process[30];
	PCB* process;
	Queue* p = q;

	if(q != NULL)
	{
		process = p->pcb;
		//strcpy(removedProcess, p->process);
		printf("OK\n");
		if(q->next != NULL)
			q = q->next;
		else
			q = NULL;
		free(p);
	}

	return q;
}

Queue* queueFreeAll(Queue* q)	// Free all queue processes
{
	Queue* last = NULL;
	Queue* p;

	if(q != NULL)
	{
		for(p = q; p->next != NULL; p = p->next)
		{
			free(last);
			last = p;
		}
	}
	
	return NULL;
}

void queuePrint(Queue* q)	// Print queue processes
{
	Queue* p;
	PCB* aux;

	for(p = q; p != NULL; p = p->next)
	{
		aux = p->pcb;
		printf("Process Name: %s\n", getPCBName(aux));
		printf("Process PID: %d\n", getPCBPid(aux));
		printf("Process State: ");
		switch(getPCBState(aux))
		{
			case new:
				printf("new\n");
				break;
			case terminated:
				printf("terminated\n");
				break;
			case running:
				printf("running\n");
				break;
			case waiting:
				printf("waiting\n");
				break;
			case ready:
				printf("ready\n");
				break;
			default:
				printf("not avaiable\n");
		}
	}
}

/* Process Manipulation Functions */

// Troca processo
void trocaProcesso (PCB* process)
{
	char* args[1] = { NULL };
	pid_t pid_pai, pid_filho;
	char aux[30];

	if(getPCBState(process) != new)
	{
		printf("Continuar processo %d \n", process->pid);
		kill(getPCBPid(process), SIGCONT);
		setPCBState(process, running);
		sleep(1);
		kill(getPCBPid(process), SIGSTOP);
		setPCBState(process, waiting);
	}
	else
	{
		pid_filho = fork();
		if(pid_filho != 0)
		{
			// Processo pai
			pid_pai = getpid();
			printf("PROCESSO PAI\t<%d>\n", pid_pai);
			sleep(1);
			setPCBPid(process, pid_filho);
			kill(pid_filho, SIGSTOP);
			process->state = waiting;
		}
		else
		{
			// Processo filho
			pid_filho = getpid();
			printf("PROCESSO FILHO\t<%d>\n", pid_filho);
			strcpy(aux, "./");
			strcat(aux, getPCBName(process));		
			execve( aux, args, NULL);
			perror("execve failed");
		}
	}
	return;
}

/* Main */
int main(int argc, char *argv[])
{
	// Declarations
	int errorControl;
	char* program;
	int* status;
	int shmArea_program;
	int shmArea_status;
	key_t key_program = 8180;
	key_t key_status = 8181;
	int shmSize_program = 100*sizeof(char);
	int shmSize_status = sizeof(int);
	Queue* L1 = newQueue(L1);

	// Program - Shared Memory - GET
	shmArea_program = shmget(
		/* key */ key_program, 
		/* size */ shmSize_program, 
		/* flags */ IPC_CREAT | S_IRUSR | S_IWUSR);
	shmVerification(shmArea_program, shm_get); // Fail verification
 	
	// Program - Shared Memory - ATTACH
	program = shmat( 
		/* shared memory identifier */ shmArea_program,
		/* shared memory adress */ NULL,
		/* flags */ 0);
	if(program == (char*) -1)
		errorControl = -1;
	shmVerification(errorControl , shm_at); // Fail verification

	// Status - Shared Memory - GET
	shmArea_status = shmget(
		/* key */ key_status, 
		/* size */ shmSize_status, 
		/* flags */ IPC_CREAT | S_IRUSR | S_IWUSR);
	shmVerification(shmArea_status, shm_get);// Fail verification

	// Status - Shared Memory - ATTACH
	status = shmat( 
		/* shared memory identifier */ shmArea_status,
		/* shared memory adress */ NULL,
		/* flags */ 0);
	if(status == (int*) -1)
		errorControl = -1;
	shmVerification(errorControl , shm_at); // Fail verification
	
	// Scheduler Actions
	while(*status == 1)
	{
		if(strlen(program) > 0) // Wait for new process (changes in shared memory)
		{
			PCB* process = newPCB(program); // Create new PCB for process
			L1 = queuePush(L1, process); // Put process in level 1 queue
			program[0] = '\0'; // Clear shared memory
		}	
	}

	queuePrint(L1); // L1 - Queue - PRINT
	L1 = queueFreeAll(L1); // L1 - Queue - FREE
	
	// Program - Shared Memory - DEATTACH
	errorControl = shmdt(
		/* adress */ program);
	shmVerification(errorControl, shm_dt); // Fail verification

	// Status - Shared Memory - DEATTACH
	errorControl = shmdt(
		/* adress */ status);
	shmVerification(errorControl, shm_dt); // Fail verification

	// Program - Shared Memory - REMOVE
	errorControl = shmctl(
		/* shared memory identifier */ shmArea_program,
		/* command */ IPC_RMID, 
		/* pointer to data structure */ NULL);
	shmVerification(errorControl, shm_ctl); // Fail verification

	// Status - Shared Memory - REMOVE
	errorControl = shmctl(
		/* shared memory identifier */ shmArea_status,
		/* command */ IPC_RMID, 
		/* pointer to data structure */ NULL);
	shmVerification(errorControl, shm_ctl); // Fail verification

	return 0;
}
