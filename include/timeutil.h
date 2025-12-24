#ifndef MDOCK_TIMEUTIL_H
#define MDOCK_TIMEUTIL_H

int mdock_current_timestamp(char *buf, size_t size);

/* Calculate uptime between two timestamps, or from start to now if end is empty/NULL */
int calculate_uptime(const char *start_time, const char *end_time,
                     char *uptime_buf, size_t size);

#endif /* MDOCK_TIMEUTIL_H */
