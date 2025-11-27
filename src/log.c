#include <stdio.h>
#include <stdarg.h>

#include "log.h"

int mdock_logf(const char *fmt, ...)
{
    (void)fmt;

    fprintf(stderr, "[mdock] logging not implemented yet\n");
    /* Stub: just ignore logs for now */
    return 0;
}
