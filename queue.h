/* Queue - Definition */

#include <string.h>	// String
#include <stdio.h>
#include <stdlib.h>

#include "PCB.h"

typedef struct queue Queue;

Queue* newQueue(Queue* q);				// Create new processes queue

int queueLength (Queue* q);				// Calculate the length of the queue

Queue* queuePush(Queue* q, PCB* newProcess);		// Push process to queue

Queue* queuePull(Queue* q, PCB** removedProcess);	// Pull process from queue

Queue* queueFreeAll(Queue* q);				// Free all queue processes

void queuePrint(Queue* q);				// Print queue processes


