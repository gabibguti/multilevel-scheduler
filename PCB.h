/* Process Control Block - PCB - Definition */

// Process Control Block (PCB) is a structure that saves process important information (PID , name, state).
// Example: process A (4937, "A", running)

#include <string.h>		// String
#include <stdio.h>		// Input and Output
#include <stdlib.h>		// Library Definitions

enum processStateType
{
	terminated	=	0,	
	new		=	1,
	ready		=	2,
	running		=	3,
	waiting		=	4
};

typedef enum processStateType stateType;				// Process States

typedef struct processControlBlock PCB;					// PCB structure

PCB* newPCB (char* processName, int argc, char** argv);			// Create new PCB 

pid_t getPCBPid (PCB* pcb);						// Get process PID

pid_t setPCBPid (PCB* pcb, pid_t pid);					// Set process PID

char* getPCBName (PCB* pcb);						// Get process name

void setPCBName (PCB* pcb, char* name);					// Set process name

int getPCBArgc (PCB* pcb);						// Get process arguments counter

void setPCBArgc (PCB* pcb, int argc);					// Set process arguments counter

char** getPCBArgv (PCB* pcb);						// Get process arguments vector

void setPCBArgv (PCB* pcb, char** argv);				// Set process arguments vector

stateType getPCBState (PCB* pcb);					// Get process state

void setPCBState (PCB* pcb, stateType state);				// Set process state

void setPCBState (PCB* pcb, stateType state);				// Set process state

void getPCBTimeStructure (PCB* pcb, time_t* t_s, suseconds_t* t_us);	// Get process time controller structure

void setPCBTimeStructure (PCB* pcb, struct timeval t);			// Set process time controller structure

char* getPCBQueue (PCB* pcb);						// Get process last/actual queue

void setPCBQueue (PCB* pcb, char* q);					// Set process last/actual queue

