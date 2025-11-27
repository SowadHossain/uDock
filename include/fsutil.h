#ifndef MDOCK_FSUTIL_H
#define MDOCK_FSUTIL_H

#include <sys/types.h>
#include <stddef.h>

int mdock_get_home(char *buf, size_t size);
int ensure_dir_exists(const char *path, mode_t mode);
int copy_dir(const char *src, const char *dst);

#endif /* MDOCK_FSUTIL_H */
