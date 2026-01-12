#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SHARED_MEM_SIZE 1024
#define SHM_KEY 1234
#define LOCK_FILE "/tmp/sender.lock"

int check_duplicate_instance() {
    int lock_fd = open(LOCK_FILE, O_CREAT | O_RDWR, 0666);
    if (lock_fd < 0) {
        return 1;
    }
    
    struct flock file_lock;
    file_lock.l_type = F_WRLCK;
    file_lock.l_whence = SEEK_SET;
    file_lock.l_start = 0;
    file_lock.l_len = 0;
    
    if (fcntl(lock_fd, F_SETLK, &file_lock) < 0) {
        close(lock_fd);
        return 1;
    }
    
    return 0;
}

int main() {
    if (check_duplicate_instance()) {
        printf("Sender program is already running.\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Shared Memory Sender (PID: %d)\n", getpid());
    
    int shm_id = shmget(SHM_KEY, SHARED_MEM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Failed to create shared memory segment");
        exit(EXIT_FAILURE);
    }
    
    char *shared_data = (char*)shmat(shm_id, NULL, 0);
    if (shared_data == (char*)-1) {
        perror("Failed to attach shared memory");
        exit(EXIT_FAILURE);
    }
    
    shared_data[0] = '\0';
    
    printf("Sender is broadcasting messages...\n");
    
    while (1) {
        time_t current_time;
        struct tm *time_info;
        time(&current_time);
        time_info = localtime(&current_time);
        
        char message[100];
        snprintf(message, sizeof(message), 
                 "Time: %02d:%02d:%02d, Sender PID: %d", 
                 time_info->tm_hour, time_info->tm_min, 
                 time_info->tm_sec, getpid());
        
        strncpy(shared_data, message, SHARED_MEM_SIZE - 1);
        shared_data[SHARED_MEM_SIZE - 1] = '\0';
        
        printf("Broadcast: %s\n", message);
        
        sleep(1);
    }
    
    shmdt(shared_data);
    shmctl(shm_id, IPC_RMID, NULL);
    
    return EXIT_SUCCESS;
}
