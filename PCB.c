// Process Control Block - PCB - Implementation

#include <string.h>	// String
#include <stdio.h>
#include <stdlib.h>

#include "PCB.h"

struct processControlBlock				// PCB structure
{
	pid_t pid;
	char name[30];
	stateType state;
};

PCB* newPCB (char* processName)				// Create new PCB 
{
	PCB* pcb = (PCB*) malloc (sizeof(PCB));
	pcb->pid = 0;
	strcpy(pcb->name, processName);
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
