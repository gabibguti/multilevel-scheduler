// Process Control Block - PCB - Definition
// Process Control Block (PCB) is a structure that saves process important information (PID , name, state).
// Example: process A (4937, "A", running)

#include <string.h>	// String
#include <stdio.h>
#include <stdlib.h>

enum processStateType
{
	terminated	=	0,	
	new		=	1,
	ready		=	2,
	running		=	3,
	waiting		=	4
};

typedef enum processStateType stateType;		// Process States

typedef struct processControlBlock PCB;			// PCB structure

PCB* newPCB (char* processName);			// Create new PCB 

pid_t getPCBPid (PCB* pcb);				// Get process PID

pid_t setPCBPid (PCB* pcb, pid_t pid);			// Set process PID

char* getPCBName (PCB* pcb);				// Get process name

void setPCBName (PCB* pcb, char* name);			// Set process name

stateType getPCBState (PCB* pcb);			// Get process state

void setPCBState (PCB* pcb, stateType state);		// Set process state

