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

/* Validate image name */
static int is_valid_image_name(const char *name)
{
    if (!name || name[0] == '\0') {
        return 0;
    }
    
    /* Check length */
    size_t len = strlen(name);
    if (len > 64) {
        fprintf(stderr, "[mdock] image name too long (max 64 characters)\n");
        return 0;
    }
    
    /* Check for invalid characters (only allow alphanumeric, dash, underscore) */
    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_')) {
            fprintf(stderr, "[mdock] invalid character in image name: '%c'\n", c);
            fprintf(stderr, "[mdock] image name must contain only alphanumeric, dash, or underscore\n");
            return 0;
        }
    }
    
    /* Don't allow names starting with dash or number */
    if (name[0] == '-' || (name[0] >= '0' && name[0] <= '9')) {
        fprintf(stderr, "[mdock] image name must start with a letter\n");
        return 0;
    }
    
    return 1;
}

/* Check if image already exists */
static int image_exists(const char *base_dir, const char *image_name)
{
    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/images.db", base_dir) >= (int)sizeof(db_path)) {
        return 0;
    }

    FILE *f = fopen(db_path, "r");
    if (!f) {
        return 0;
    }

    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        char *p = strchr(line, '|');
        if (!p) continue;
        *p = '\0';
        if (strcmp(line, image_name) == 0) {
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

int cmd_build(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: mdock build <image_name> <rootfs_dir>\n");
        fprintf(stderr, "\nExamples:\n");
        fprintf(stderr, "  mdock build myimage ./rootfs\n");
        fprintf(stderr, "  mdock build alpine-base /tmp/alpine-rootfs\n");
        return 1;
    }

    const char *image_name = argv[1];
    const char *src_rootfs = argv[2];

    /* Validate image name */
    if (!is_valid_image_name(image_name)) {
        return 1;
    }

    struct stat st;
    if (stat(src_rootfs, &st) == -1) {
        fprintf(stderr, "[mdock] error: rootfs directory '%s' does not exist\n", src_rootfs);
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[mdock] error: '%s' is not a directory\n", src_rootfs);
        return 1;
    }

    char base_dir[PATH_MAX];
    if (mdock_init_home(base_dir, sizeof(base_dir)) != 0) {
        fprintf(stderr, "[mdock] failed to initialize home directory\n");
        return 1;
    }

    /* Check if image already exists */
    if (image_exists(base_dir, image_name)) {
        fprintf(stderr, "[mdock] error: image '%s' already exists\n", image_name);
        fprintf(stderr, "[mdock] hint: choose a different name or remove the existing image\n");
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

int cmd_images(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    char base_dir[PATH_MAX];
    if (mdock_init_home(base_dir, sizeof(base_dir)) != 0) {
        return 1;
    }

    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/images.db", base_dir) >= (int)sizeof(db_path)) {
        fprintf(stderr, "[mdock] db path too long\n");
        return 1;
    }

    FILE *fp = fopen(db_path, "r");
    if (!fp) {
        if (errno == ENOENT) {
            printf("No images found.\n");
            return 0;
        }
        perror("[mdock] fopen images.db");
        return 1;
    }

    // Print header
    printf("%-20s %-15s %-20s\n", "IMAGE", "SIZE", "CREATED");
    printf("%-20s %-15s %-20s\n", "-----", "----", "-------");

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        // Parse: image_name|rootfs_path|timestamp
        char image_name[256];
        char rootfs_path[PATH_MAX];
        char timestamp[64];

        if (sscanf(line, "%255[^|]|%4095[^|]|%63[^\n]", image_name, rootfs_path, timestamp) != 3) {
            continue;  // Skip malformed lines
        }

        // Calculate image size
        char size_str[32] = "N/A";
        char cmd[PATH_MAX + 50];
        if (snprintf(cmd, sizeof(cmd), "du -sh '%s' 2>/dev/null", rootfs_path) < (int)sizeof(cmd)) {
            FILE *du = popen(cmd, "r");
            if (du) {
                char du_output[256];
                if (fgets(du_output, sizeof(du_output), du)) {
                    // Parse size from "123M    /path"
                    sscanf(du_output, "%31s", size_str);
                }
                pclose(du);
            }
        }

        // Format timestamp (just show date part for brevity)
        char created_str[32];
        if (strlen(timestamp) >= 19) {
            // Extract date from "2024-12-24T10:30:15"
            snprintf(created_str, sizeof(created_str), "%.10s %.8s", timestamp, timestamp + 11);
        } else {
            strncpy(created_str, timestamp, sizeof(created_str) - 1);
            created_str[sizeof(created_str) - 1] = '\0';
        }

        printf("%-20s %-15s %-20s\n", image_name, size_str, created_str);
    }

    fclose(fp);
    return 0;
}

int image_in_use(const char *base_dir, const char *image_name)
{
    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/containers.db", base_dir) >= (int)sizeof(db_path)) {
        return 0;  // Assume not in use if path too long
    }

    FILE *fp = fopen(db_path, "r");
    if (!fp) {
        return 0;  // No containers exist
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        // Parse: container_id|pid|image|status|start_time|end_time|exit_code
        char container_image[256];
        if (sscanf(line, "%*[^|]|%*[^|]|%255[^|]", container_image) == 1) {
            if (strcmp(container_image, image_name) == 0) {
                fclose(fp);
                return 1;  // Image is in use
            }
        }
    }

    fclose(fp);
    return 0;  // Not in use
}

int cmd_rmi(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: mdock rmi <image_name>\n");
        return 1;
    }

    const char *image_name = argv[1];

    char base_dir[PATH_MAX];
    if (mdock_init_home(base_dir, sizeof(base_dir)) != 0) {
        return 1;
    }

    // Check if image is in use
    if (image_in_use(base_dir, image_name)) {
        fprintf(stderr, "Error: Image '%s' is in use by one or more containers.\n", image_name);
        fprintf(stderr, "Hint: Remove containers first with 'mdock rm <container_id>'\n");
        return 1;
    }

    // Find image in database
    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/images.db", base_dir) >= (int)sizeof(db_path)) {
        fprintf(stderr, "[mdock] db path too long\n");
        return 1;
    }

    FILE *fp = fopen(db_path, "r");
    if (!fp) {
        fprintf(stderr, "Error: No images database found.\n");
        return 1;
    }

    // Read all lines, find and remove the target image
    char lines[1000][1024];
    int line_count = 0;
    int found = 0;
    char rootfs_to_delete[PATH_MAX] = "";

    while (fgets(lines[line_count], sizeof(lines[line_count]), fp) && line_count < 1000) {
        char current_name[256];
        char current_rootfs[PATH_MAX];
        if (sscanf(lines[line_count], "%255[^|]|%4095[^|]", current_name, current_rootfs) == 2) {
            if (strcmp(current_name, image_name) == 0) {
                found = 1;
                strncpy(rootfs_to_delete, current_rootfs, sizeof(rootfs_to_delete) - 1);
                rootfs_to_delete[sizeof(rootfs_to_delete) - 1] = '\0';
                continue;  // Skip this line (delete it)
            }
        }
        line_count++;
    }
    fclose(fp);

    if (!found) {
        fprintf(stderr, "Error: Image '%s' not found.\n", image_name);
        return 1;
    }

    // Write back without the deleted image
    fp = fopen(db_path, "w");
    if (!fp) {
        perror("[mdock] fopen for writing");
        return 1;
    }

    for (int i = 0; i < line_count; i++) {
        fputs(lines[i], fp);
    }
    fclose(fp);

    // Delete the rootfs directory
    if (strlen(rootfs_to_delete) > 0) {
        // Get parent directory (image dir)
        char image_dir[PATH_MAX];
        strncpy(image_dir, rootfs_to_delete, sizeof(image_dir) - 1);
        image_dir[sizeof(image_dir) - 1] = '\0';
        
        // Remove "/rootfs" suffix to get image directory
        char *last_slash = strrchr(image_dir, '/');
        if (last_slash) {
            *last_slash = '\0';
        }

        char cmd[PATH_MAX + 50];
        if (snprintf(cmd, sizeof(cmd), "rm -rf '%s'", image_dir) < (int)sizeof(cmd)) {
            int ret = system(cmd);
            if (ret != 0) {
                fprintf(stderr, "Warning: Failed to delete image directory\n");
            }
        }
    }

    mdock_logf("RMI image=%s", image_name);
    printf("Removed image '%s'\n", image_name);

    return 0;
}
