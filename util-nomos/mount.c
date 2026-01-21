#include <getopt.h>
#include <stdio.h>
#include <sys/mount.h>

int main(int argc, const char **argv) {
    const char *type = "auto"; // Default to auto-detect.
    int opt;
    while ((opt = getopt(argc, (char * const *)argv, "t:")) != -1) {
        switch (opt) {
            case 't':
                type = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-t fstype] <source> <target>\n", argv[0]);
                return 1;
        }
    }

    if (optind + 2 != argc) {
        fprintf(stderr, "Usage: %s [-t fstype] <source> <target>\n", argv[0]);
        return 1;
    }

    const char *source = argv[optind];
    const char *target = argv[optind + 1];
    if (mount(source, target, type, 0, NULL) == -1) {
        perror("mount");
        return 1;
    }

    return 0;
}