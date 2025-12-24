#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <linux/limits.h>

#include "container.h"
#include "image.h"
#include "log.h"
#include "timeutil.h"

/* ----- Issue #7: containers.db helpers ----- */

/* Format: id|pid|image|status|start_time|end_time|exit_code */

int add_container_record(const char *base_dir,
                         const char *container_id,
                         int pid,
                         const char *image_name)
{
    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/containers.db", base_dir) >= (int)sizeof(db_path)) {
        fprintf(stderr, "[mdock] containers.db path too long\n");
        return -1;
    }

    FILE *f = fopen(db_path, "a");
    if (!f) {
        perror("[mdock] fopen containers.db");
        return -1;
    }

    char start_time[32];
    if (mdock_current_timestamp(start_time, sizeof(start_time)) != 0) {
        snprintf(start_time, sizeof(start_time), "0000-00-00T00:00:00");
    }

    /* id|pid|image|status|start_time|end_time|exit_code */
    fprintf(f, "%s|%d|%s|running|%s||-1\n", container_id, pid, image_name, start_time);
    fclose(f);
    return 0;
}

int update_container_exit(const char *base_dir,
                          const char *container_id,
                          int exit_code)
{
    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/containers.db", base_dir) >= (int)sizeof(db_path)) {
        fprintf(stderr, "[mdock] containers.db path too long\n");
        return -1;
    }

    char tmp_path[PATH_MAX];
    if (snprintf(tmp_path, sizeof(tmp_path), "%s/containers.db.tmp", base_dir) >= (int)sizeof(tmp_path)) {
        fprintf(stderr, "[mdock] tmp path too long\n");
        return -1;
    }

    FILE *f_in = fopen(db_path, "r");
    if (!f_in) {
        perror("[mdock] fopen containers.db");
        return -1;
    }

    FILE *f_out = fopen(tmp_path, "w");
    if (!f_out) {
        perror("[mdock] fopen containers.db.tmp");
        fclose(f_in);
        return -1;
    }

    char end_time[32];
    if (mdock_current_timestamp(end_time, sizeof(end_time)) != 0) {
        snprintf(end_time, sizeof(end_time), "0000-00-00T00:00:00");
    }

    char line[4096];
    int updated = 0;

    while (fgets(line, sizeof(line), f_in)) {
        /* Parse: id|pid|image|status|start_time|end_time|exit_code */
        char *p1 = strchr(line, '|');
        if (!p1) {
            fputs(line, f_out);
            continue;
        }
        *p1 = '\0';
        char *id = line;

        if (strcmp(id, container_id) == 0) {
            /* Found the container, rebuild the line with updated fields */
            char *p2 = strchr(p1 + 1, '|');
            if (!p2) {
                fputs(line, f_out);
                continue;
            }
            *p2 = '\0';
            char *pid_str = p1 + 1;

            char *p3 = strchr(p2 + 1, '|');
            if (!p3) {
                fputs(line, f_out);
                continue;
            }
            *p3 = '\0';
            char *image = p2 + 1;

            char *p4 = strchr(p3 + 1, '|');
            if (!p4) {
                fputs(line, f_out);
                continue;
            }
            *p4 = '\0';
            /* status field - we'll replace it */

            char *p5 = strchr(p4 + 1, '|');
            if (!p5) {
                fputs(line, f_out);
                continue;
            }
            *p5 = '\0';
            char *start_time = p4 + 1;

            /* Now write updated record */
            fprintf(f_out, "%s|%s|%s|exited|%s|%s|%d\n",
                    id, pid_str, image, start_time, end_time, exit_code);
            updated = 1;
        } else {
            /* Restore the line and write as-is */
            *p1 = '|';
            fputs(line, f_out);
        }
    }

    fclose(f_in);
    fclose(f_out);

    if (!updated) {
        fprintf(stderr, "[mdock] container %s not found in containers.db\n", container_id);
        unlink(tmp_path);
        return -1;
    }

    if (rename(tmp_path, db_path) != 0) {
        perror("[mdock] rename containers.db");
        return -1;
    }

    return 0;
}

int generate_container_id(const char *base_dir,
                          char *out_id,
                          size_t out_size)
{
    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/containers.db", base_dir) >= (int)sizeof(db_path)) {
        fprintf(stderr, "[mdock] containers.db path too long\n");
        return -1;
    }

    FILE *f = fopen(db_path, "r");
    if (!f) {
        /* If file doesn't exist, start with c1 */
        if (snprintf(out_id, out_size, "c1") >= (int)out_size) {
            fprintf(stderr, "[mdock] container id buffer too small\n");
            return -1;
        }
        return 0;
    }

    int max_num = 0;
    char line[4096];

    while (fgets(line, sizeof(line), f)) {
        /* Parse id field (format: cN) */
        char *p = strchr(line, '|');
        if (!p) continue;
        *p = '\0';

        if (line[0] == 'c') {
            int num = atoi(&line[1]);
            if (num > max_num) {
                max_num = num;
            }
        }
    }

    fclose(f);

    if (snprintf(out_id, out_size, "c%d", max_num + 1) >= (int)out_size) {
        fprintf(stderr, "[mdock] container id buffer too small\n");
        return -1;
    }

    return 0;
}

/* ----- Issue #8 & #9: mdock run command ----- */

int cmd_run(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: mdock run <image_name>\n");
        return 1;
    }

    const char *image_name = argv[1];

    /* Initialize ~/.mdock */
    char base_dir[PATH_MAX];
    if (mdock_init_home(base_dir, sizeof(base_dir)) != 0) {
        fprintf(stderr, "[mdock] failed to initialize home directory\n");
        return 1;
    }

    /* Lookup image rootfs */
    char rootfs_path[PATH_MAX];
    if (find_image_rootfs(base_dir, image_name, rootfs_path, sizeof(rootfs_path)) != 0) {
        fprintf(stderr, "[mdock] image '%s' not found\n", image_name);
        return 1;
    }

    /* Generate unique container ID */
    char container_id[64];
    if (generate_container_id(base_dir, container_id, sizeof(container_id)) != 0) {
        fprintf(stderr, "[mdock] failed to generate container ID\n");
        return 1;
    }

    /* Fork child process */
    pid_t pid = fork();
    if (pid == -1) {
        perror("[mdock] fork");
        return 1;
    }

    if (pid == 0) {
        /* ===== Child process ===== */
        
        /* Change directory to rootfs */
        if (chdir(rootfs_path) != 0) {
            perror("[mdock] chdir");
            exit(1);
        }

        /* Optional: chroot to rootfs (requires root privileges) */
        /* Uncomment if running as root:
         * if (chroot(rootfs_path) != 0) {
         *     perror("[mdock] chroot");
         *     exit(1);
         * }
         */

        /* Execute /bin/sh inside the container */
        char *args[] = {"/bin/sh", NULL};
        char *envp[] = {NULL};
        
        execve("/bin/sh", args, envp);
        
        /* If execve fails */
        perror("[mdock] execve");
        exit(1);
    }

    /* ===== Parent process ===== */

    /* Record container in containers.db with status=running */
    if (add_container_record(base_dir, container_id, pid, image_name) != 0) {
        fprintf(stderr, "[mdock] failed to add container record\n");
        /* Continue anyway, we'll try to wait for the child */
    }

    /* Log RUN event */
    mdock_logf("RUN container_id=%s pid=%d image=%s", container_id, pid, image_name);

    printf("[mdock] Container %s started (PID %d)\n", container_id, pid);

    /* ===== Issue #9: Wait for container exit ===== */

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("[mdock] waitpid");
        return 1;
    }

    /* Extract exit code */
    int exit_code = 0;
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        exit_code = 128 + WTERMSIG(status);
    }

    /* Update container record with exit info */
    if (update_container_exit(base_dir, container_id, exit_code) != 0) {
        fprintf(stderr, "[mdock] failed to update container exit status\n");
    }

    /* Log EXIT event */
    mdock_logf("EXIT container_id=%s pid=%d exit_code=%d", container_id, pid, exit_code);

    printf("[mdock] Container %s exited with code %d\n", container_id, exit_code);

    return 0;
}

int cmd_ps(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    fprintf(stderr, "[mdock] ps: not implemented yet (Milestone M3)\n");
    return 1;
}

int cmd_stop(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    fprintf(stderr, "[mdock] stop: not implemented yet (Milestone M3)\n");
    return 1;
}
