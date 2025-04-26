#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> /* Utilized for pthread creation, join, and for mutex (mutual exclusion) */
#include <unistd.h>
#include <semaphore.h> /* Used for the full and empty attribute functionality */
#include "process.h" /* From our previous project for the typedef struct of process details */

#define Numberof 100 /* Defines a non-modifiable value that stays the same through out the program, used for the bounded buffer also */

Process boundedbuffer[Numberof]; /* Used for both Producer and Consumer, this should be Process type and not char because each Process has 4 attributes and storing the entire process which includes those attirbutes would make the boundedbuffer a process array of struct defined in process.h */
int in = 0;
int out = 0;

/* The mutex and the semaphore functions to utlize */
pthread_mutex_t mutex;
sem_t full, empty;


void* producer(void* arg)
{
	Process* p = (Process*)arg;
	printf("[Producer %d] Thread Started \n", p->pid);
	printf("[Producer %d] Waiting for Empty Slot...\n", p->pid);
	sem_wait(&empty); /* Waiting for empty slot */
	printf("[Producer %d] Waiting for Locks...\n", p->pid);
	pthread_mutex_lock(&mutex); /* locks the process so that other process consumer cannot interfere */
	printf("[Producer %d] Acquired Lock...\n", p->pid);
	boundedbuffer[in] = *p;
	printf("[Producer %d] created process at BufferIndex %d\n", p->pid, in);
	in = (in + 1) % Numberof;

	pthread_mutex_unlock(&mutex); /* Unlocks the process so that other process consumer can utlize the process */
	printf("[Producer %d] Released Lock...\n", p->pid);
	sem_post(&full); /* Indicate that a full slot is available */
	printf("[Producer %d] Finished \n", p->pid);
	pthread_exit(NULL);
}

void* consumer(void* arg)
{
	printf("[Consumer] Thread Started \n");
	printf("[Consumer] Waiting for Full Slot...\n");
	sem_wait(&full); /* Wait for a full slot */
	printf("[Consumer] Waiting for Lock...\n");
	pthread_mutex_lock(&mutex); /* locks the process so that the other process producer cannot interfere */
	printf("[Consumer] Acquired Lock...\n");

	Process p = boundedbuffer[out];
	printf("[Consumer] picked process %d from BufferIndex %d\n", p.pid, out);
	out = (out + 1) % Numberof;

	pthread_mutex_unlock(&mutex); /* Unlocks the process so that other process producer can utlize the process */
	printf("[Consumer] Released Lock...\n");
	sem_post(&empty); /* Indicate an empty available slot */
	printf("[Consumer] Thread %d started (Arrival: %d, Burst: %d, Priority: %d)\n", p.pid, p.arrival_time, p.burst_time, p.priority);
    	sleep(p.burst_time);
    	printf("[Consumer] Thread %d completed\n", p.pid);
	printf("[Consumer] Finished \n");
    	pthread_exit(NULL);
}


int main()
{
	/* File pointer used to open the file and read it */
	FILE* file = fopen("processes.txt", "r");
	if (file == NULL)
	{
		perror("Error opening file");
		return 1;
	}

	Process processes[Numberof];
	int count = 0;

	/* This line was added to read the titles of process details of pid to burst_time due to the next function fscanf only processing numerical values */
	char process_detail_title[256];
	fgets(process_detail_title, sizeof(process_detail_title), file); /* Uses fgets to read first line up to a \n newline character and increments pointer reference to second line */

	/* To read each line and process the processes' details */
	while (fscanf(file, "%d %d %d %d", &processes[count].pid, &processes[count].arrival_time, &processes[count].burst_time, &processes[count].priority) == 4)
	{
		count++;
	}

	fclose(file);

	/* printf("Total processes read: %d\n", count);  Debug Purposes */


	/* Producer-Consumer Synchronization initialization */
	pthread_mutex_init(&mutex, NULL);
	sem_init(&empty, 0, Numberof);
	sem_init(&full, 0, 0);

	pthread_t producers[Numberof]; /* creating Producer threads with pthread_t data type */
    	pthread_t consumers[Numberof]; /* creating Consumer threads with pthread_t data type */

    	for (int p = 0; p < count; p++) {
        	pthread_create(&producers[p], NULL, producer, &processes[p]);
    	}

    	for (int c = 0; c < count; c++) {
        	pthread_create(&consumers[c], NULL, consumer, NULL);
    	}

    	for (int i = 0; i < count; i++) {
        	pthread_join(producers[i], NULL);
        	pthread_join(consumers[i], NULL);
    	}

    	// Cleanup
    	pthread_mutex_destroy(&mutex);
    	sem_destroy(&empty);
    	sem_destroy(&full);

    return 0;

}
