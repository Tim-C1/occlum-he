#include <stdio.h>
#include <time.h>

void pal_clock(struct timespec *ts, const char *s)
{
    clock_gettime(CLOCK_REALTIME, ts);
    fprintf(stderr, "boot stage: %s at time: %ld.%09ld seconds\n", s, ts->tv_sec, ts->tv_nsec);
}

