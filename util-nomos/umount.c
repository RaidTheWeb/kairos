#include <getopt.h>
#include <stdio.h>
#include <sys/mount.h>

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <target>\n", argv[0]);
        return 1;
    }

    const char *target = argv[1];
    if (umount(target) == -1) {
        perror("umount");
        return 1;
    }
    return 0;
}