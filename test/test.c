#include <errno.h>
#include <fcntl.h>
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv) {
    printf("Hello!!!!\n");

    uint32_t eax, ebx, ecx, edx;
    asm volatile(
        "xgetbv"
        : "=a"(eax), "=d"(edx)
        : "c"(0)
    );
    if ((eax & 6) != 6) {
        printf("CPU doesn't support AVX.\n");
        return 1;
    } else {
        printf("CPU supports AVX.\n");
    }


    char buf[256];
    printf("Password: ");
    fflush(stdout);

    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ECHO; // Disable echo.
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);

    fgets(buf, sizeof(buf), stdin);
    if (buf[strlen(buf) - 1] == '\n') {
        buf[strlen(buf) - 1] = '\0';
    }

    t.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
    printf("\nOutput is `%s` %f.\n", buf, 1.45 / 332.41233);

    asm volatile("mfence" : : : "memory");

    float a[8] __attribute__((aligned(32))) = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
    float b[8] __attribute__((aligned(32))) = { 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f };
    float result[8] __attribute__((aligned(32)));

    __m256 vec_a = _mm256_load_ps(a);
    __m256 vec_b = _mm256_load_ps(b);
    __m256 vec_res = _mm256_add_ps(vec_a, vec_b);
    _mm256_store_ps(result, vec_res);

    for (size_t i = 0; i < 8; i++) {
        if (result[i] != 9.0f) {
            printf("Failed!.\n");
            return 1;
        }
    }
    printf("AVX test passed!.\n");

    return 0;
}
