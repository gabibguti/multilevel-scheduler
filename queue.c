/* Queue - Implementation */

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

#include "queue.h"

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
	//printf("Trying to push process to queue\n");
	//fflush(stdout);
	Queue * p = q;
	Queue* new = (Queue*) malloc (sizeof(Queue));
	if(new == NULL)
	{
		printf("ERROR: MEMORY ALLOCATION\n");
		exit(1);
	}
	if(strlen(getPCBName(newProcess)) <= 0) // Assert
	{
		printf("Process has no name, wont push!\n");
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

	//printf("Push succeeded\n");
	//fflush(stdout);

	return q;
}

Queue* queuePull(Queue* q, PCB** removedProcess)			// Pull process from queue
{
	Queue* p = q;
	PCB* pcbCopy;

	if(p != NULL)
	{
		//removedProcess = p->pcb;
		//removedProcess = newPCB (getPCBName(p->pcb));
		//setPCBPid(removedProcess, getPCBPid(p->pcb));
		//setPCBState(removedProcess, getPCBState(p->pcb));
		pcbCopy = newPCB(getPCBName(p->pcb), getPCBArgc(p->pcb), getPCBArgv(p->pcb));
		if(pcbCopy == NULL)
		{
			printf("ERROR: MEMORY ALLOCATION\n");
			exit(1);
		}
		setPCBPid(pcbCopy, getPCBPid(p->pcb));
		setPCBState(pcbCopy, getPCBState(p->pcb)); 
		*removedProcess = pcbCopy;	
		//printf("Process %s removed from queue.\n", getPCBName(pcbCopy));
		if(q->next != NULL)
			q = q->next;
		else
			q = NULL;
	}
	else
	{
		printf("Empty queue\n");
	}	
	
	return q;
}

Queue* queueFreeAll(Queue* q)					// Free all queue processes
{
	Queue* last = NULL;
	Queue* p;

	if(q != NULL)
	{
		printf("Freeing all processes\n");
		fflush(stdout);
		for(p = q; p->next != NULL; p = p->next)
		{
			free(last);
			last = p;
		}
	}

	printf("Freeing suceeded\n");
	fflush(stdout);
	
	return NULL;
}

void queuePrint(Queue* q)					// Print queue processes
{
	int i;
	Queue* p = q;
	PCB* aux;
	char** arguments;

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
		p = p->next;
	}
}

