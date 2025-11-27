#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "image.h"
#include "fsutil.h"
#include "timeutil.h"
#include "log.h"
#include <linux/limits.h>


static int ensure_file_exists(const char *path)
{
    int fd = open(path, O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("[mdock] open");
        return -1;
    }
    close(fd);
    return 0;
}

int mdock_init_home(char *out_base_dir, size_t size)
{
    char home[PATH_MAX];
    if (mdock_get_home(home, sizeof(home)) != 0) {
        return -1;
    }

    char base_dir[PATH_MAX];
    if (snprintf(base_dir, sizeof(base_dir), "%s/.mdock", home) >= (int)sizeof(base_dir)) {
        fprintf(stderr, "[mdock] base dir path too long\n");
        return -1;
    }

    if (ensure_dir_exists(base_dir, 0755) != 0) {
        return -1;
    }

    char images_dir[PATH_MAX];
    if (snprintf(images_dir, sizeof(images_dir), "%s/images", base_dir) >= (int)sizeof(images_dir)) {
        fprintf(stderr, "[mdock] images dir path too long\n");
        return -1;
    }

    if (ensure_dir_exists(images_dir, 0755) != 0) {
        return -1;
    }

    /* Ensure metadata files exist: images.db, containers.db, log.txt */
    char images_db[PATH_MAX];
    char containers_db[PATH_MAX];
    char log_file[PATH_MAX];

    if (snprintf(images_db, sizeof(images_db), "%s/images.db", base_dir) >= (int)sizeof(images_db) ||
        snprintf(containers_db, sizeof(containers_db), "%s/containers.db", base_dir) >= (int)sizeof(containers_db) ||
        snprintf(log_file, sizeof(log_file), "%s/log.txt", base_dir) >= (int)sizeof(log_file)) {
        fprintf(stderr, "[mdock] metadata path too long\n");
        return -1;
    }

    if (ensure_file_exists(images_db) != 0) return -1;
    if (ensure_file_exists(containers_db) != 0) return -1;
    if (ensure_file_exists(log_file) != 0) return -1;

    /* Return base_dir to caller */
    if (strlen(base_dir) + 1 > size) {
        fprintf(stderr, "[mdock] out_base_dir buffer too small\n");
        return -1;
    }
    strcpy(out_base_dir, base_dir);
    return 0;
}

/* ----- images.db helpers ----- */

static int add_image_record(const char *base_dir,
                            const char *image_name,
                            const char *rootfs_path)
{
    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/images.db", base_dir) >= (int)sizeof(db_path)) {
        fprintf(stderr, "[mdock] images.db path too long\n");
        return -1;
    }

    FILE *f = fopen(db_path, "a");
    if (!f) {
        perror("[mdock] fopen images.db");
        return -1;
    }

    char ts[32];
    if (mdock_current_timestamp(ts, sizeof(ts)) != 0) {
        snprintf(ts, sizeof(ts), "0000-00-00T00:00:00");
    }

    fprintf(f, "%s|%s|%s\n", image_name, rootfs_path, ts);
    fclose(f);
    return 0;
}

int find_image_rootfs(const char *base_dir,
                      const char *image_name,
                      char *out_path,
                      size_t out_size)
{
    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/images.db", base_dir) >= (int)sizeof(db_path)) {
        fprintf(stderr, "[mdock] images.db path too long\n");
        return -1;
    }

    FILE *f = fopen(db_path, "r");
    if (!f) {
        perror("[mdock] fopen images.db");
        return -1;
    }

    char line[4096];
    int found = 0;

    while (fgets(line, sizeof(line), f)) {
        /* Format: image_name|rootfs_path|created_at */
        char *p1 = strchr(line, '|');
        if (!p1) continue;
        *p1 = '\0';
        char *name = line;

        char *p2 = strchr(p1 + 1, '|');
        if (!p2) continue;
        *p2 = '\0';
        char *rootfs = p1 + 1;

        if (strcmp(name, image_name) == 0) {
            if (strlen(rootfs) + 1 > out_size) {
                fprintf(stderr, "[mdock] rootfs path too long for buffer\n");
                fclose(f);
                return -1;
            }
            strcpy(out_path, rootfs);
            found = 1;
            break;
        }
    }

    fclose(f);
    return found ? 0 : -1;
}

/* ----- mdock build command ----- */

int cmd_build(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: mdock build <image_name> <rootfs_dir>\n");
        return 1;
    }

    const char *image_name = argv[1];
    const char *src_rootfs = argv[2];

    struct stat st;
    if (stat(src_rootfs, &st) == -1) {
        perror("[mdock] stat rootfs_dir");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[mdock] build: %s is not a directory\n", src_rootfs);
        return 1;
    }

    char base_dir[PATH_MAX];
    if (mdock_init_home(base_dir, sizeof(base_dir)) != 0) {
        fprintf(stderr, "[mdock] failed to initialize home directory\n");
        return 1;
    }

    char image_dir[PATH_MAX];
    if (snprintf(image_dir, sizeof(image_dir), "%s/images/%s", base_dir, image_name) >= (int)sizeof(image_dir)) {
        fprintf(stderr, "[mdock] image dir path too long\n");
        return 1;
    }
    if (ensure_dir_exists(image_dir, 0755) != 0) {
        return 1;
    }

    char dest_rootfs[PATH_MAX];
    if (snprintf(dest_rootfs, sizeof(dest_rootfs), "%s/rootfs", image_dir) >= (int)sizeof(dest_rootfs)) {
        fprintf(stderr, "[mdock] dest rootfs path too long\n");
        return 1;
    }

    if (copy_dir(src_rootfs, dest_rootfs) != 0) {
        fprintf(stderr, "[mdock] failed to copy rootfs directory\n");
        return 1;
    }

    if (add_image_record(base_dir, image_name, dest_rootfs) != 0) {
        fprintf(stderr, "[mdock] failed to update images.db\n");
        return 1;
    }

    mdock_logf("BUILD image=%s src=%s", image_name, src_rootfs);

    printf("Built image '%s' at %s\n", image_name, dest_rootfs);
    return 0;
}
