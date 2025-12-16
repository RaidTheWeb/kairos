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
#include <sys/utsname.h>

#define MAX_INPUT 256
#define MAX_ARGS 64

#define USER "rtw"

void reaper(int signum) {
    // Reap all terminated child processes.
    while (waitpid(-1, NULL, WNOHANG) > 0);
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

    // Suppress SIGINT in shell itself.
    signal(SIGINT, SIG_IGN);


    // Load and set hostname from /etc/hostname.
    FILE *hostname_file = fopen("/etc/hostname", "r");
    if (hostname_file) {
        char hostname[256];
        if (fgets(hostname, sizeof(hostname), hostname_file)) {
            hostname[strcspn(hostname, "\n")] = 0; // Remove newline
            sethostname(hostname, strlen(hostname));
        }
        fclose(hostname_file);
    }

    // Get uname and display welcome message.
    struct utsname buf;
    if (uname((struct utsname *)&buf) == 0) {
        printf("Welcome to %s!\n", buf.nodename);
    }

    char *envp[] = { "TERM=linux", NULL };
    pid_t logpid = fork();
    if (logpid == 0) {
        setsid(); // Start new session for bash.

        setpgid(0, 0);

        // Acquire controlling terminal.
        ioctl(STDIN_FILENO, TIOCSCTTY, 1);

        signal(SIGINT, SIG_DFL); // Restore default SIGINT behavior
        signal(SIGCHLD, SIG_DFL); // Restore default SIGCHLD behavior

        setgid(1000);
        setuid(1000);

        tcsetpgrp(STDIN_FILENO, getpgid(0));
        execvpe("/usr/bin/bash", (char *[]) { "bash", NULL }, envp);
        perror("execve bash");
        exit(1);
    } else if (logpid > 0) {
        setpgid(logpid, logpid);
        while (1) {
            pause(); // Wait for signals
        }
    } else {
        perror("fork");
    }
    return 0;
}