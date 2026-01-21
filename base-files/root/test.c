#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/utsname.h>

int main(int argc, char **argv) {
    printf("Hello World!\n");

    int fd = open("testfile", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    char *mapped = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }
    char buffer[29];
    memcpy(buffer, mapped, 28);
    buffer[28] = '\0';

    printf("Mapped content: %s\n", buffer);
    munmap(mapped, 4096);
    close(fd);

    if (uname((volatile void *)0xdeadbeef)) {
        perror("uname");
        return 1;
    }

    return 0;
}