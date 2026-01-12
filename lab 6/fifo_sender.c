#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    printf("FIFO SENDER PROCESS\n");
    
    if (mkfifo("myfifo", 0666) == -1) {
        perror("Failed to create FIFO");
        exit(EXIT_FAILURE);
    }
    
    time_t current_time;
    struct tm *time_info;
    time(&current_time);
    time_info = localtime(&current_time);
    
    char message[100];
    snprintf(message, sizeof(message), 
             "Sender time: %02d:%02d:%02d, PID: %d", 
             time_info->tm_hour, time_info->tm_min, time_info->tm_sec, 
             getpid());
    
    printf("Sender waiting for 10 seconds...\n");
    sleep(10);
    
    int fifo_fd = open("myfifo", O_WRONLY);
    if (fifo_fd == -1) {
        perror("Failed to open FIFO for writing");
        exit(EXIT_FAILURE);
    }
    
    ssize_t bytes_written = write(fifo_fd, message, strlen(message) + 1);
    if (bytes_written == -1) {
        perror("Failed to write to FIFO");
        close(fifo_fd);
        exit(EXIT_FAILURE);
    }
    
    close(fifo_fd);
    
    printf("Data transmitted via FIFO\n");
    
    return EXIT_SUCCESS;
}
