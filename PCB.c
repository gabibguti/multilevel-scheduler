// Process Control Block - PCB - Implementation

#include <string.h>		// String
#include <stdio.h>		// Input and Output
#include <stdlib.h>		// Library Definitions

#include "PCB.h" 		// PCB Definitions	

#define parametersMAX 5

struct processControlBlock						// PCB structure
{
	pid_t pid;		// Pid
	char name[30];		// Name
	int argc;		// Arguments counter
	char** argv;		// Arguments vector
	stateType state;	// State
	struct timeval start;	// Time controller
	char queue[10];		// Actual/last queue
};

PCB* newPCB (char* processName, int argc, char** argv)			// Create new PCB 
{
	PCB* pcb = (PCB*) malloc (sizeof(PCB));
	pcb->pid = 0;
	strcpy(pcb->name, processName);
	pcb->argc = argc;
	pcb->argv = argv;
	pcb->state = new;
	pcb->start.tv_sec = 0;
	pcb->start.tv_usec = 0;
	pcb->queue[0] = '\0';
	return pcb;
}

pid_t getPCBPid (PCB* pcb)						// Get process PID
{
	return pcb->pid;
}

pid_t setPCBPid (PCB* pcb, pid_t pid)					// Set process PID
{
	pcb->pid = pid;
}

char* getPCBName (PCB* pcb)						// Get process name
{
	return pcb->name;
}

void setPCBName (PCB* pcb, char* name)					// Set process name
{
	strcpy(pcb->name, name);
}

int getPCBArgc (PCB* pcb)						// Get process arguments counter
{
	return pcb->argc;
}

void setPCBArgc (PCB* pcb, int argc)					// Set process arguments counter
{
	pcb->argc = argc;
}

char** getPCBArgv (PCB* pcb)						// Get process arguments vector
{
	return pcb->argv;
}

void setPCBArgv (PCB* pcb, char** argv)					// Set process arguments vector
{
	pcb->argv = argv;
}

stateType getPCBState (PCB* pcb)					// Get process state
{
	return pcb->state;
}

void setPCBState (PCB* pcb, stateType state)				// Set process state
{
	pcb->state = state;
}

void getPCBTimeStructure (PCB* pcb, time_t* t_s, suseconds_t* t_us)	// Get process time controller structure
{
	*t_s = pcb->start.tv_sec;
	*t_us = pcb->start.tv_usec;
}

void setPCBTimeStructure (PCB* pcb, struct timeval t)			// Set process time controller structure
{
	pcb->start.tv_sec = t.tv_sec;
	pcb->start.tv_usec = t.tv_usec;
}

char* getPCBQueue (PCB* pcb)						// Get process last/actual queue
{
	return pcb->queue;
}

void setPCBQueue (PCB* pcb, char* q)					// Set process last/actual queue
{
	strcpy(pcb->queue, q);
}
