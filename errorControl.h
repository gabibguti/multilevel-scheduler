// Error Control - Definition

#include <stdio.h>
#include <stdlib.h>

enum errorControlTypes
{
	shm_get		=	1,
	shm_at		=	2,
	shm_dt		=	3,
	shm_ctl		=	4,
	sig_emptyset 	=	5,
	sig_addset	=	6,
	sig_procmask	=	7
};

typedef enum errorControlTypes errorType;		// Error Type Identifier

void failVerification(int errorControl, errorType e);	// Error Verification
