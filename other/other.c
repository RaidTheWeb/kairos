#include <dirent.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/poll.h>

#include <pthread.h>

void handler(int signum) {
    printf("Caught signal %d\n", signum);
}

void *thread_func(void *arg) {
    printf("Hello from thread %s!\n", (char *)arg);
    return NULL;
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
        nanosleep((const struct timespec[]){{5, 100000000L}}, NULL);
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
        pause(); // Wait for signals
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
    } else if (argc > 1 && strcmp(argv[1], "threadtest") == 0) {

        pthread_t thread;
        if (pthread_create(&thread, NULL, thread_func, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }

        pthread_join(thread, NULL);
    } else if (argc > 1 && strcmp(argv[1], "fifotest") == 0) {
        const char *fifo_path = "my_fifo";
        mkfifo(fifo_path, 0666);

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            // Child process - Writer
            int fd = open(fifo_path, O_WRONLY);
            const char *msg = "Hello from FIFO!\n";
            write(fd, msg, strlen(msg));
            close(fd);
            exit(0);
        } else {
            // Parent process - Reader
            char buffer[100];
            memset(buffer, 0, sizeof(buffer));
            int fd = open(fifo_path, O_RDONLY);
            read(fd, buffer, sizeof(buffer));
            printf("Received from FIFO: %s", buffer);
            close(fd);
            wait(NULL); // Wait for child to finish
            unlink(fifo_path); // Remove FIFO
        }
    } else if (argc > 1 && strcmp(argv[1], "bigthreadtest") == 0) {
        #define NUM_THREADS 10
        pthread_t threads[NUM_THREADS];
        for (int i = 0; i < NUM_THREADS; i++) {
            if (pthread_create(&threads[i], NULL, thread_func, i % 2 ? "A" : "B") != 0) {
                perror("pthread_create");
                return 1;
            }
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }
    } else if (argc > 1 && strcmp(argv[1], "timetest") == 0) {
        #include <time.h>
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char buf[100];
        strftime(buf, sizeof(buf), "Current time: %Y-%m-%d %H:%M:%S\n", t);
        printf("%s", buf);
    } else if (argc > 2 && strcmp(argv[1], "fg") == 0) {
        // Bring a background job to the foreground
        pid_t job_pid = (pid_t)atoi(argv[2]);
        if (kill(job_pid, SIGCONT) == -1) {
            perror("kill");
            return 1;
        }
        int status;
        waitpid(job_pid, &status, 0);
        printf("Job %d brought to foreground with status %d\n", job_pid, status);
    } else if (argc > 1 && strcmp(argv[1], "loop") == 0) {
        while (1) {
            printf("Looping...\n");
            sleep(1);
        }
    } else {
        printf("Unknown command or insufficient arguments.\n");
    }
    return 0;
}