#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "process.h"

// Global array to store processes
Process processes[256];
int process_count = 0;

// Readers-Writers synchronization variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Protects reader_count
pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;  // Exclusive writer lock
int reader_count = 0;  // Number of active readers

// Thread function to simulate a reader process
void* reader_process(void* arg) {
    Process* proc = (Process*)arg;
    
    printf("Reader Process %d (PID: %d) starting\n", proc->pid, (int)pthread_self());
    
    // Entry section for reader
    pthread_mutex_lock(&mutex);
    reader_count++;
    if (reader_count == 1) {
        // First reader acquires write_lock to prevent writers
        pthread_mutex_lock(&write_lock);
    }
    pthread_mutex_unlock(&mutex);
    
    // Critical section - reading happens here
    printf("Reader Process %d is reading (Burst time: %d)\n", proc->pid, proc->burst_time);
    sleep(proc->burst_time);  // Simulate the CPU burst time
    
    // Exit section for reader
    pthread_mutex_lock(&mutex);
    reader_count--;
    if (reader_count == 0) {
        // Last reader releases write_lock
        pthread_mutex_unlock(&write_lock);
    }
    pthread_mutex_unlock(&mutex);
    
    printf("Reader Process %d finished reading\n", proc->pid);
    
    return NULL;
}

// Thread function to simulate a writer process
void* writer_process(void* arg) {
    Process* proc = (Process*)arg;
    
    printf("Writer Process %d (PID: %d) starting\n", proc->pid, (int)pthread_self());
    
    // Entry section for writer - acquire exclusive lock
    pthread_mutex_lock(&write_lock);
    
    // Critical section - writing happens here
    printf("Writer Process %d is writing (Burst time: %d)\n", proc->pid, proc->burst_time);
    sleep(proc->burst_time);  // Simulate the CPU burst time
    
    // Exit section for writer - release exclusive lock
    pthread_mutex_unlock(&write_lock);
    
    printf("Writer Process %d finished writing\n", proc->pid);
    
    return NULL;
}

// Function to create and manage threads for all processes
void simulate_processes_with_threads() {
    pthread_t threads[256];
    int i;

    printf("\nStarting process simulation with threads...\n");
    
    // We'll assume even PIDs are readers and odd PIDs are writers for demonstration
    for (i = 0; i < process_count; i++) {
        // Sleep for arrival time to simulate process arrivals
        sleep(processes[i].arrival_time);
        
        // Create thread based on process type (reader or writer)
        if (processes[i].pid % 2 == 0) {
            // Even PID - reader process
            if (pthread_create(&threads[i], NULL, reader_process, &processes[i]) != 0) {
                perror("Failed to create reader thread");
                exit(EXIT_FAILURE);
            }
        } else {
            // Odd PID - writer process
            if (pthread_create(&threads[i], NULL, writer_process, &processes[i]) != 0) {
                perror("Failed to create writer thread");
                exit(EXIT_FAILURE);
            }
        }
        
        printf("Created thread for process %d (PID: %d)\n", 
               processes[i].pid, i);
    }
    
    // Wait for all threads to complete
    for (i = 0; i < process_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("All processes completed\n");
}

// Your original parse_process_file function
void parse_process_file(const char *filename) {
    // ... (keep your original function)
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
    
    // Simulate processes with threads and readers-writers synchronization
    simulate_processes_with_threads();

    return 0;
}