/* Queue - Implementation */

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

#include "queue.h"		// Queue Definitions

struct queue							// Process Queue
{
	PCB* pcb;
	struct queue * next;
};

Queue* newQueue(Queue* q)					// Create new processes queue
{
	q = (Queue*) malloc (sizeof(Queue));
	q = NULL;
	return q;
}

int queueLength (Queue* q)					// Calculate the length of the queue
{
	int i = 0;
	Queue* p = q;
	for(p = q; p != NULL; p = p->next)
		i++;
	return i;	
}

Queue* queuePush(Queue* q, PCB* newProcess)			// Push process to queue
{
	struct timeval t;
	Queue * p = q;
	Queue* new = (Queue*) malloc (sizeof(Queue));
	if(new == NULL)
	{
		printf("ERROR: MEMORY ALLOCATION\n");
		exit(1);
	}
	if(strlen(getPCBName(newProcess)) <= 0) // Assert
	{
		return q;
	}
	PCB* pcbCopy = newPCB(getPCBName(newProcess), getPCBArgc(newProcess), getPCBArgv(newProcess));
	if(pcbCopy == NULL)
	{
		printf("ERROR: MEMORY ALLOCATION\n");
		exit(1);
	}
	setPCBPid(pcbCopy, getPCBPid(newProcess));
	setPCBState(pcbCopy, getPCBState(newProcess));
	getPCBTimeStructure(newProcess, &t.tv_sec, &t.tv_usec);
	setPCBTimeStructure(pcbCopy, t); 
	setPCBQueue(pcbCopy, getPCBQueue(newProcess));
	new->pcb = pcbCopy;
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

Queue* queuePull(Queue* q, PCB** removedProcess)			// Pull process from queue
{
	Queue* p = q;
	PCB* pcbCopy;
	struct timeval t;

	if(p != NULL)
	{
		pcbCopy = newPCB(getPCBName(p->pcb), getPCBArgc(p->pcb), getPCBArgv(p->pcb));
		if(pcbCopy == NULL)
		{
			printf("ERROR: MEMORY ALLOCATION\n");
			exit(1);
		}
		setPCBPid(pcbCopy, getPCBPid(p->pcb));
		setPCBState(pcbCopy, getPCBState(p->pcb)); 
		getPCBTimeStructure(p->pcb, &t.tv_sec, &t.tv_usec);
		setPCBTimeStructure(pcbCopy, t);
		setPCBQueue(pcbCopy, getPCBQueue(p->pcb));
		*removedProcess = pcbCopy;	
		if(q->next != NULL)
			q = q->next;
		else
			q = NULL;
	}
	
	return q;
}

Queue* queueFreeAll(Queue* q)					// Free all queue processes
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
	printf("Freeing Processes\n");	
	return NULL;
}

void queuePrint(Queue* q)					// Print queue processes
{
	int i;
	Queue* p = q;
	PCB* aux;
	char** arguments;
	struct timeval t;	

	while(p != NULL)
	{
		aux = p->pcb;
		printf("Process Name: %s\n", getPCBName(aux));
		fflush(stdout);
		printf("Process PID: %d\n", getPCBPid(aux));
		fflush(stdout);
		printf("Process State: ");
		fflush(stdout);
		switch(getPCBState(aux))
		{
			case new:
				printf("new\n");
				fflush(stdout);
				break;
			case terminated:
				printf("terminated\n");
				fflush(stdout);
				break;
			case running:
				printf("running\n");
				fflush(stdout);
				break;
			case waiting:
				printf("waiting\n");
				fflush(stdout);
				break;
			case ready:
				printf("ready\n");
				fflush(stdout);
				break;
			default:
				printf("not avaiable\n");
				fflush(stdout);
		}
		printf("Process Arguments: ");
		fflush(stdout);
		arguments = getPCBArgv(aux);
		for(i = 0; i < getPCBArgc(aux); i++)
		{
			printf("%s ", arguments[i]);
		}
		printf("\n");
		getPCBTimeStructure(aux, &t.tv_sec, &t.tv_usec);
		printf("Time Controller: %lu\n", t.tv_sec + t.tv_usec);
		printf("Actual/Last queue: %s\n", getPCBQueue(aux));
		p = p->next;
	}
}

void queueNextTime(Queue* q, time_t* t_s, suseconds_t* t_us)		// Get first process time controller from queue
{
	Queue* p = q;

	if(p != NULL)
	{
		getPCBTimeStructure(p->pcb, t_s, t_us);
	}

	fflush(stdout);
}

