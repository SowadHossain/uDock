#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#include "log.h"
#include "timeutil.h"
#include "fsutil.h"
#include <linux/limits.h>

int mdock_logf(const char *fmt, ...)
{
    char home[PATH_MAX];
    if (mdock_get_home(home, sizeof(home)) != 0) {
        return -1;
    }

    char log_path[PATH_MAX];
    if (snprintf(log_path, sizeof(log_path), "%s/.mdock/log.txt", home) >= (int)sizeof(log_path)) {
        fprintf(stderr, "[mdock] log path too long\n");
        return -1;
    }

    FILE *f = fopen(log_path, "a");
    if (!f) {
        perror("[mdock] fopen log");
        return -1;
    }

    char ts[32];
    if (mdock_current_timestamp(ts, sizeof(ts)) != 0) {
        /* fallback timestamp */
        snprintf(ts, sizeof(ts), "0000-00-00T00:00:00");
    }

    fprintf(f, "[%s] ", ts);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);

    fputc('\n', f);
    fclose(f);
    return 0;
}
