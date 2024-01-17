
#include <stdio.h>
#include <time.h>

int main (int argc, char *argv[])
{
    struct timespec first, later;
    long long int f, l;

    clock_gettime(CLOCK_MONOTONIC, &first);
    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &later);
        sleep(1);
#if 0
        if (later.tv_sec != first.tv_sec) break;
        if (later.tv_nsec != first.tv_nsec) break;
#endif
        printf("sec %d nsec %lld\n", later.tv_sec, later.tv_nsec);
    }
    f = first.tv_sec + first.tv_nsec * 1000000000LL;
    l = later.tvsec + later.tv_nsec * 1000000000LL;
    printf("resolution is %lld nano seconds\n", l - f);
}


