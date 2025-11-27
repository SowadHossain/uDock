#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include "fsutil.h"
#include <linux/limits.h>

static int copy_file(const char *src, const char *dst)
{
    int in_fd = open(src, O_RDONLY);
    if (in_fd == -1) {
        perror("[mdock] open src");
        return -1;
    }

    struct stat st;
    if (fstat(in_fd, &st) == -1) {
        perror("[mdock] fstat");
        close(in_fd);
        return -1;
    }

    int out_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
    if (out_fd == -1) {
        perror("[mdock] open dst");
        close(in_fd);
        return -1;
    }

    char buf[4096];
    ssize_t n;
    while ((n = read(in_fd, buf, sizeof(buf))) > 0) {
        ssize_t written = 0;
        while (written < n) {
            ssize_t w = write(out_fd, buf + written, n - written);
            if (w == -1) {
                perror("[mdock] write");
                close(in_fd);
                close(out_fd);
                return -1;
            }
            written += w;
        }
    }

    if (n == -1) {
        perror("[mdock] read");
    }

    close(in_fd);
    close(out_fd);
    return (n == -1) ? -1 : 0;
}

int mdock_get_home(char *buf, size_t size)
{
    const char *home = getenv("HOME");
    if (!home || home[0] == '\0') {
        fprintf(stderr, "[mdock] HOME is not set\n");
        return -1;
    }
    if (strlen(home) + 1 > size) {
        fprintf(stderr, "[mdock] home path too long\n");
        return -1;
    }
    strcpy(buf, home);
    return 0;
}

int ensure_dir_exists(const char *path, mode_t mode)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "[mdock] %s exists but is not a directory\n", path);
            return -1;
        }
        return 0; /* already exists */
    }

    if (errno != ENOENT) {
        perror("[mdock] stat");
        return -1;
    }

    if (mkdir(path, mode) == -1) {
        perror("[mdock] mkdir");
        return -1;
    }
    return 0;
}

int copy_dir(const char *src, const char *dst)
{
    struct stat st;
    if (stat(src, &st) == -1) {
        perror("[mdock] stat src");
        return -1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[mdock] copy_dir: %s is not a directory\n", src);
        return -1;
    }

    if (ensure_dir_exists(dst, 0755) != 0) {
        return -1;
    }

    DIR *dir = opendir(src);
    if (!dir) {
        perror("[mdock] opendir");
        return -1;
    }

    struct dirent *ent;
    char src_path[PATH_MAX];
    char dst_path[PATH_MAX];

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        if (snprintf(src_path, sizeof(src_path), "%s/%s", src, ent->d_name) >= (int)sizeof(src_path) ||
            snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, ent->d_name) >= (int)sizeof(dst_path)) {
            fprintf(stderr, "[mdock] path too long\n");
            closedir(dir);
            return -1;
        }

        if (stat(src_path, &st) == -1) {
            perror("[mdock] stat entry");
            closedir(dir);
            return -1;
        }

        if (S_ISDIR(st.st_mode)) {
            if (copy_dir(src_path, dst_path) != 0) {
                closedir(dir);
                return -1;
            }
        } else if (S_ISREG(st.st_mode)) {
            if (copy_file(src_path, dst_path) != 0) {
                closedir(dir);
                return -1;
            }
        } else {
            /* ignore other types (symlinks, devices) for simplicity */
            fprintf(stderr, "[mdock] skipping non-regular file: %s\n", src_path);
        }
    }

    closedir(dir);
    return 0;
}
