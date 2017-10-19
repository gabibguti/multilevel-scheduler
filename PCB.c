// Process Control Block - PCB - Implementation

#include <string.h>	// String
#include <stdio.h>
#include <stdlib.h>

#include "PCB.h"

#define parametersMAX 5

struct processControlBlock				// PCB structure
{
	pid_t pid;
	char name[30];
	char** argv;
	int argc;
	stateType state;
};

PCB* newPCB (char* processName, int argc, char** argv)	// Create new PCB 
{
	PCB* pcb = (PCB*) malloc (sizeof(PCB));
	pcb->pid = 0;
	strcpy(pcb->name, processName);
	pcb->argc = argc;
	pcb->argv = argv;
	pcb->state = new;
	return pcb;
}

pid_t getPCBPid (PCB* pcb)				// Get process PID
{
	return pcb->pid;
}

pid_t setPCBPid (PCB* pcb, pid_t pid)			// Set process PID
{
	pcb->pid = pid;
}

char* getPCBName (PCB* pcb)				// Get process name
{
	return pcb->name;
}

void setPCBName (PCB* pcb, char* name)			// Set process name
{
	strcpy(pcb->name, name);
}

stateType getPCBState (PCB* pcb)			// Get process state
{
	return pcb->state;
}

void setPCBState (PCB* pcb, stateType state)		// Set process state
{
	pcb->state = state;
}

void setPCBArgc (PCB* pcb, int argc)			// Set process arguments counter
{
	pcb->argc = argc;
}

int getPCBArgc (PCB* pcb)				// Get process arguments counter
{
	return pcb->argc;
}

void setPCBArgv (PCB* pcb, char** argv)			// Set process arguments vector
{
	pcb->argv = argv;
}

char** getPCBArgv (PCB* pcb)				// Get process arguments vector
{
	return pcb->argv;
}
