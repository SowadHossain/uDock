#include <stdio.h>
#include <time.h>
#include <string.h>

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

int calculate_uptime(const char *start_time, const char *end_time,
                     char *uptime_buf, size_t size)
{
    struct tm tm_start = {0};
    struct tm tm_end = {0};
    
    /* Parse start time */
    if (!start_time || start_time[0] == '\0') {
        snprintf(uptime_buf, size, "00:00:00");
        return 0;
    }
    
    if (strptime(start_time, "%Y-%m-%dT%H:%M:%S", &tm_start) == NULL) {
        snprintf(uptime_buf, size, "??:??:??");
        return -1;
    }
    
    time_t start = mktime(&tm_start);
    time_t end;
    
    /* If end_time is empty or NULL, use current time */
    if (!end_time || end_time[0] == '\0' || end_time[0] == '|') {
        end = time(NULL);
    } else {
        if (strptime(end_time, "%Y-%m-%dT%H:%M:%S", &tm_end) == NULL) {
            snprintf(uptime_buf, size, "??:??:??");
            return -1;
        }
        end = mktime(&tm_end);
    }
    
    /* Calculate difference */
    long diff = (long)difftime(end, start);
    if (diff < 0) diff = 0;
    
    int hours = diff / 3600;
    int minutes = (diff % 3600) / 60;
    int seconds = diff % 60;
    
    snprintf(uptime_buf, size, "%02d:%02d:%02d", hours, minutes, seconds);
    return 0;
}
