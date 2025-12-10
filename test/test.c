#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#include <fcntl.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>

#define MAX_INPUT 256
#define MAX_ARGS 64

#define USER "rtw"

char *strsig(int sig) {
    switch (sig) {
        case SIGHUP: return "SIGHUP";
        case SIGINT: return "SIGINT";
        case SIGQUIT: return "SIGQUIT";
        case SIGILL: return "SIGILL";
        case SIGABRT: return "SIGABRT";
        case SIGFPE: return "SIGFPE";
        case SIGKILL: return "SIGKILL";
        case SIGSEGV: return "SIGSEGV";
        case SIGPIPE: return "SIGPIPE";
        case SIGALRM: return "SIGALRM";
        case SIGTERM: return "SIGTERM";
        case SIGUSR1: return "SIGUSR1";
        case SIGUSR2: return "SIGUSR2";
        case SIGCHLD: return "SIGCHLD";
        case SIGCONT: return "SIGCONT";
        case SIGSTOP: return "SIGSTOP";
        case SIGTSTP: return "SIGTSTP";
        case SIGTTIN: return "SIGTTIN";
        case SIGTTOU: return "SIGTTOU";
        default: return "UNKNOWN SIGNAL";
    }
}

void reaper(int signum) {
    printf("Got SIGCHLD\n");
}

void execute_command(char *cmd) {
    char *args[MAX_ARGS];
    char input[MAX_INPUT];
    strncpy(input, cmd, MAX_INPUT - 1);

    int argc = 0;
    char *token = strtok(input, " ");

    while (token && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }
    args[argc] = NULL;

    if (argc == 0) {
        return;
    }

    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(args[0], "cd") == 0) {
        if (argc < 2) {
            fprintf(stderr, "cd: missing operand\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return;
    } else if (strcmp(args[0], "clear") == 0) {
        printf("\033[H\033[J");
        return;
    }

    // Resolve environment variables:
    char pwdbuf[256];
    getcwd(pwdbuf, sizeof(pwdbuf));
    char pwdenv[300];
    snprintf(pwdenv, sizeof(pwdenv), "PWD=%s", pwdbuf);

    char *envp[] = { "LD_SHOW_AUXV=1", pwdenv, "TERM=linux", "USER=" USER, NULL };

    pid_t pid = fork();
    if (pid == 0) {
        setgid(1001);
        setuid(1001); // Drop privileges to non-root user before executing other commands

        setpgid(0, 0); // Set new process group

        tcsetpgrp(STDIN_FILENO, getpgid(0)); // Give terminal control to child process

        if (execvpe(args[0], args, envp) == -1) {
            perror("execve");
        }
        exit(1);
    } else if (pid > 0) {
        setpgid(pid, pid); // Ensure child is in its own process group

        int status;
        tcsetpgrp(STDIN_FILENO, pid); // Give terminal control to child process
        waitpid(pid, &status, 0);
        tcsetpgrp(STDIN_FILENO, getpgid(0)); // Return terminal control to shell
        if (WIFEXITED(status)) {
            // Child exited normally
        } else if (WIFSIGNALED(status)) {
            printf("Process terminated by signal %d (%s)\n", WTERMSIG(status), strsig(WTERMSIG(status)));
        }
    } else {
        perror("fork");
    }
}

int main() {

    int console = open("/dev/console", O_RDWR);
    if (console == -1) {
        perror("open /dev/console");
        return 1;
    }

    // Redirect standard file descriptors to console.
    dup2(console, STDIN_FILENO);
    dup2(console, STDOUT_FILENO);
    dup2(console, STDERR_FILENO);

    close(console);

    if (ioctl(STDIN_FILENO, TIOCSCTTY, 1) == -1) { // Set controlling terminal.
        perror("ioctl TIOCSCTTY");
        return 1;
    }

    pid_t pgid = getpgid(0);
    if (tcsetpgrp(STDIN_FILENO, pgid) == -1) { // Give terminal control to shell process group.
        perror("tcsetpgrp");
        return 1;
    }

    struct sigaction sa = { 0 };
    sa.sa_handler = reaper;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    char input[MAX_INPUT];

    while (1) {
        char cwdbuf[1024];
        getcwd(cwdbuf, sizeof(cwdbuf));

        char hostname[256];
        gethostname(hostname, sizeof(hostname));

        printf("\033[1;32m%s\033[0m@\033[1;34m%s\033[0m:\033[1;33m%s\033[0m$ ", USER, hostname, cwdbuf);

        fflush(stdout); // Force prompt out, so we don't buffer before newline.

        if (!fgets(input, MAX_INPUT, stdin)) {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strlen(input) > 0) {
            execute_command(input);
        }
    }

    return 0;
}