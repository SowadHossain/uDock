#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fsutil.h"

int mdock_get_home(char *buf, size_t size)
{
    (void)buf;
    (void)size;

    fprintf(stderr, "[mdock] mdock_get_home: not implemented yet\n");
    return -1;
}

int ensure_dir_exists(const char *path, mode_t mode)
{
    (void)path;
    (void)mode;

    fprintf(stderr, "[mdock] ensure_dir_exists: not implemented yet\n");
    return -1;
}

int copy_dir(const char *src, const char *dst)
{
    (void)src;
    (void)dst;

    fprintf(stderr, "[mdock] copy_dir: not implemented yet (Milestone M1)\n");
    return -1;
}
