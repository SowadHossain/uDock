#include <stdio.h>
#include <time.h>

#include "timeutil.h"

int mdock_current_timestamp(char *buf, size_t size)
{
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        perror("[mdock] time");
        return -1;
    }

    struct tm tm_now;
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    if (localtime_r(&now, &tm_now) == NULL) {
        perror("[mdock] localtime_r");
        return -1;
    }
#else
    struct tm *ptm = localtime(&now);
    if (!ptm) {
        perror("[mdock] localtime");
        return -1;
    }
    tm_now = *ptm;
#endif

    if (strftime(buf, size, "%Y-%m-%dT%H:%M:%S", &tm_now) == 0) {
        fprintf(stderr, "[mdock] strftime buffer too small\n");
        return -1;
    }

    return 0;
}
