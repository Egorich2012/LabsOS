#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

int main() {
    int fifo_fd = open("myfifo", O_RDONLY);
    if (fifo_fd == -1) {
        perror("Failed to open FIFO");
        exit(EXIT_FAILURE);
    }
    
    char buffer[100];
    
    ssize_t bytes_read = read(fifo_fd, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        perror("Failed to read from FIFO");
        close(fifo_fd);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_read] = '\0'; 
    close(fifo_fd);
    
    time_t current_time;
    struct tm *time_info;
    
    time(&current_time);
    time_info = localtime(&current_time);
    
    printf("FIFO Receiver - Current PID: %d\n", getpid());
    printf("Current time: %02d:%02d:%02d\n", 
           time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
    printf("Message from FIFO: %s\n", buffer);
    
    unlink("myfifo");
    
    return EXIT_SUCCESS;
}
