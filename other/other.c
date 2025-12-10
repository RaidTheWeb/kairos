#include <dirent.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/stat.h>
#include <sys/poll.h>

void handler(int signum) {
    printf("Caught signal %d\n", signum);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "suidtest") == 0) {
        setuid(0); // Attempt to change UID to 0 (root), should only work if binary is setuid root.
        if (getuid() == 0) {
            printf("suidtest successful, current UID: %d\n", getuid());
        } else {
            printf("suidtest failed, current UID: %d\n", getuid());
        }
    } else if (argc > 1 && strcmp(argv[1], "ncursestest") == 0) {
        #include <ncurses.h>
        initscr();
        printw("Hello, ncurses!");
        refresh();
        getch();
        endwin();
    } else if (argc > 1 && strcmp(argv[1], "pipetest") == 0) {
        // Open a pipe and demonstrate inter-process communication
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe");
            return 1;
        }
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            // Child process
            close(fd[0]); // Close unused read end
            const char *msg = "Hello from child process!\n";
            write(fd[1], msg, strlen(msg));
            close(fd[1]);
            exit(0);
        } else {
            // Parent process
            close(fd[1]); // Close unused write end
            char buffer[100];
            read(fd[0], buffer, sizeof(buffer));
            printf("Received message: %s", buffer);
            close(fd[0]);
            wait(NULL); // Wait for child to finish
        }
    } else if (argc > 1 && strcmp(argv[1], "faulttest") == 0) {
        poll(NULL, 0, 5000); // Wait for 5 seconds.
        int *p = NULL;
        *p = 42;
    } else if (argc > 1 && strcmp(argv[1], "sigactiontest") == 0) {
        struct sigaction sa = { 0 };
        sa.sa_handler = handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        if (sigaction(SIGINT, &sa, NULL) == -1) {
            perror("sigaction");
            return 1;
        }

        printf("Press Ctrl+C to trigger SIGINT...\n");
        while (1) {
            pause(); // Wait for signals
        }
    } else if (argc > 1 && strcmp(argv[1], "orphantest") == 0) {

        pid_t pid = fork();
        if (pid < 0 ){
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            while (true) {
                asm ("pause");
            }
        } else {
            printf("Parent process exiting, child will become orphan.\n");
            exit(0);
        }
    } else {
        printf("Unknown command or insufficient arguments.\n");
    }
    return 0;
}