#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    printf("Hello!!!!\n");
    int fd = open("/usr/share/nomos/nomos", O_RDONLY);

    unsigned char first = 0;
    ssize_t r = read(fd, &first, 1);
    printf("First byte is %x.\n", first);

    return 0;
}
