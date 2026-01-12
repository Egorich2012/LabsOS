#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHARED_MEM_SIZE 1024
#define SHM_KEY 1234

int main() {
    printf("Shared Memory Receiver (PID: %d)\n", getpid());
    
    int shm_id = shmget(SHM_KEY, SHARED_MEM_SIZE, 0666);
    if (shm_id == -1) {
        printf("Shared memory segment not found.\n");
        printf("Please run the sender program first.\n");
        exit(EXIT_FAILURE);
    }
    
    char *shared_data = (char*)shmat(shm_id, NULL, 0);
    if (shared_data == (char*)-1) {
        perror("Failed to attach shared memory");
        exit(EXIT_FAILURE);
    }
    
    char previous_message[100] = "";
    
    printf("Receiver is listening for messages...\n\n");
    
    while (1) {
        if (strlen(shared_data) > 0 && strcmp(shared_data, previous_message) != 0) {
            time_t current_time;
            struct tm *time_info;
            time(&current_time);
            time_info = localtime(&current_time);
            
            printf("Receiver - PID: %d\n", getpid());
            printf("Time: %02d:%02d:%02d\n", 
                   time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
            printf("Message: %s\n\n", shared_data);
            
            strncpy(previous_message, shared_data, sizeof(previous_message) - 1);
            previous_message[sizeof(previous_message) - 1] = '\0';
        }
        
        usleep(100000);
    }
    
    shmdt(shared_data);
    
    return EXIT_SUCCESS;
}
