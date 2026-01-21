#include <nomos/syscall.h>

#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main(int argc, const char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <new_root> <put_old>\n", argv[0]);
        return 1;
    }

    const char *new_root = argv[1];
    const char *put_old = argv[2];

    int ret = 0;
    if ((ret = syscall(SYSCALL_PIVOTROOT, (uint64_t)new_root, strlen(new_root), (uint64_t)put_old, strlen(put_old), 0, 0)) < 0) {
        errno = -ret;
        perror("pivot_root");
        return 1;
    }
    return 0;
}