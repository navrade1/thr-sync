#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "process.h"  // header file from prev project

// Global array to store processes
Process processes[256];
int process_count = 0;

// Thread function to simulate a process
void* process_thread(void* arg) {
    Process* proc = (Process*)arg;
    
    printf("Process %d (Thread ID: %lu) starting execution\n", proc->pid, (unsigned long)pthread_self());
    
    // Simulate the CPU burst time with sleep
    printf("Process %d running for %d time units\n", proc->pid, proc->burst_time);
    sleep(proc->burst_time);  // Simulate the CPU burst time
    
    printf("Process %d completed execution\n", proc->pid);
    
    return NULL;
}

// Function to create and manage threads for all processes
void simulate_processes_with_threads() {
    pthread_t threads[256];
    int i;

    printf("\nStarting process simulation with threads...\n");
    
    for (i = 0; i < process_count; i++) {
        // Sleep for arrival time to simulate process arrivals
        printf("Waiting for process %d to arrive (in %d time units)...\n", 
               processes[i].pid, processes[i].arrival_time);
        sleep(processes[i].arrival_time);
        
        // Create thread for this process
        if (pthread_create(&threads[i], NULL, process_thread, &processes[i]) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
        
        printf("Created thread for process %d\n", processes[i].pid);
    }
    
    // Wait for all threads to complete
    for (i = 0; i < process_count; i++) {
        pthread_join(threads[i], NULL);
        printf("Process %d thread joined\n", processes[i].pid);
    }
    
    printf("All processes completed\n");
}

// Function to parse the process file
void parse_process_file(const char *filename) {
    FILE *processfile = fopen(filename, "r");
    if (processfile == NULL) {
        perror("Error opening file");
        exit(1);
    }

    char line_index[256];

    // Skip the header line
    if (fgets(line_index, sizeof(line_index), processfile) == NULL) {
        perror("Error reading the header of the file\n");
        fclose(processfile);
        exit(1);
    }

    process_count = 0;

    while (fgets(line_index, sizeof(line_index), processfile) != NULL && process_count < 256) {
        int pid, arrivaltime, bursttime, priority;
        if (sscanf(line_index, "%d %d %d %d", &pid, &arrivaltime, &bursttime, &priority) == 4) {
            processes[process_count].pid = pid;
            processes[process_count].arrival_time = arrivaltime;
            processes[process_count].burst_time = bursttime;
            processes[process_count].priority = priority;
            process_count++;
        }
    }

    fclose(processfile);

    // Print the parsed processes
    printf("\nParsed Processes:\n");
    for (int iterate = 0; iterate < process_count; iterate++) {
        printf("Process [%d]: PID=%d, Arrival=%d, Burst=%d, Priority=%d\n", 
               iterate, 
               processes[iterate].pid, 
               processes[iterate].arrival_time, 
               processes[iterate].burst_time, 
               processes[iterate].priority);
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("\nUsage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Parse the process file
    parse_process_file(argv[1]);
    
    // Simulate processes with threads
    simulate_processes_with_threads();

    return 0;
}