#include <fcntl.h>

int main(int argc, char **argv) {
    int fd = open("/test", O_RDONLY);

    return 0;
}
