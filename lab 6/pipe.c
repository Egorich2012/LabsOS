#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

int main() {
    printf("PIPE COMMUNICATION EXAMPLE\n");
    
    int pipe_fd[2];
    pid_t child_pid;
    
    if (pipe(pipe_fd) == -1) {
        perror("Failed to create pipe");
        exit(EXIT_FAILURE);
    }
    
    child_pid = fork();
    
    if (child_pid == -1) {
        perror("Failed to fork process");
        exit(EXIT_FAILURE);
    }
    
    if (child_pid > 0) {
        close(pipe_fd[0]); 
        
        time_t parent_time;
        struct tm *parent_timeinfo;
        time(&parent_time);
        parent_timeinfo = localtime(&parent_time);
        
        char parent_message[100];
        snprintf(parent_message, sizeof(parent_message),
                 "Parent time: %02d:%02d:%02d, PID: %d",
                 parent_timeinfo->tm_hour, parent_timeinfo->tm_min,
                 parent_timeinfo->tm_sec, getpid());
        
        printf("Parent process waiting for 5 seconds...\n");
        sleep(5);
        
        ssize_t bytes_written = write(pipe_fd[1], parent_message,
                                      strlen(parent_message) + 1);
        if (bytes_written == -1) {
            perror("Failed to write to pipe");
            close(pipe_fd[1]);
            exit(EXIT_FAILURE);
        }
        
        close(pipe_fd[1]);
        
        wait(NULL);
        printf("Parent process execution completed\n");
    }
    else {
        close(pipe_fd[1]); 
        
        char received_data[100];
        
        ssize_t bytes_read = read(pipe_fd[0], received_data,
                                  sizeof(received_data) - 1);
        if (bytes_read == -1) {
            perror("Failed to read from pipe");
            close(pipe_fd[0]);
            exit(EXIT_FAILURE);
        }
        received_data[bytes_read] = '\0';
        
        time_t child_time;
        struct tm *child_timeinfo;
        time(&child_time);
        child_timeinfo = localtime(&child_time);
        
        printf("Child process - PID: %d\n", getpid());
        printf("Current time: %02d:%02d:%02d\n",
               child_timeinfo->tm_hour, child_timeinfo->tm_min,
               child_timeinfo->tm_sec);
        printf("Message received: %s\n", received_data);
        
        close(pipe_fd[0]);
    }
    
    return EXIT_SUCCESS;
}
