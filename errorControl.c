// Error Control - Definition

#include <stdio.h>
#include <stdlib.h>
#include "errorControl.h"

void failVerification(int errorControl, errorType e)	// Error Verification
{
	if(errorControl != -1) // Everything is okay
	{
		return;
	}

	switch(e) // Specify error
	{
		case shm_get:
			printf("\nERROR: SHARED MEMORY ERROR\n");
			perror("--- shmget: shmget failed");
			break;
		case shm_at:
			printf("\nERROR: SHARED MEMORY ERROR\n");
			perror("--- shmat: shmat failed");
			break;
		case shm_dt:
			printf("\nERROR: SHARED MEMORY ERROR\n");
			perror("--- shmdt: shmdt failed");
			break;
		case shm_ctl:
			printf("\nERROR: SHARED MEMORY ERROR\n");
			perror("--- shmctl: shmctl failed");
			break;
		case sig_emptyset:
			printf("\nERROR: SIGNAL SET ERROR\n");
			perror("--- sigemptyset: sigemptyset failed");
			break;
		case sig_addset:
			printf("\nERROR: SIGNAL SET ERROR\n");
			perror("--- sigaddset: sigaddset failed");
			break;
		case sig_procmask:
			printf("\nERROR: SIGNAL SET ERROR\n");
			perror("--- sigprocmask: sigprocmask failed");
			break;	
		default:
			perror("ERROR: VERIFICATION FAILED\n");
	}
	printf("\n\n");
	exit(1); // Leave
}

