#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void cleanup_proc(void) {
    printf("Process %d exiting\n", getpid());
    fflush(stdout);
}

void handle_sig(int s) {
    printf("Signal %d received by process %d\n", s, getpid());
    fflush(stdout);
}

int main(void) {
    atexit(cleanup_proc);

    signal(SIGINT, handle_sig);

    struct sigaction sig_act;
    sig_act.sa_handler = handle_sig;
    sig_act.sa_flags = 0;
    sigemptyset(&sig_act.sa_mask);
    sigaction(SIGTERM, &sig_act, NULL);

    pid_t child_pid = fork();
    if (child_pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (!child_pid) {
        signal(SIGINT, SIG_DFL);
        printf("Child process: PID=%d, PPID=%d\n", getpid(), getppid());
        fflush(stdout);
        sleep(5);
        _exit(42);
    } else {
        printf("Parent process: PID=%d, PPID=%d, Child PID=%d\n", 
               getpid(), getppid(), child_pid);
        fflush(stdout);
        
        int child_status;
        waitpid(child_pid, &child_status, 0);

        if (WIFEXITED(child_status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(child_status));
        } else if (WIFSIGNALED(child_status)) {
            printf("Child terminated by signal %d\n", WTERMSIG(child_status));
        }
    }

    return 0;
}