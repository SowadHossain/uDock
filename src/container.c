#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <linux/limits.h>

#include "container.h"
#include "image.h"
#include "log.h"
#include "timeutil.h"

/* ----- Issue #10 & #11: Helper functions ----- */

int is_pid_alive(pid_t pid)
{
    /* Use kill with signal 0 to check if process exists */
    if (kill(pid, 0) == 0) {
        return 1; /* Process exists */
    }
    return 0; /* Process doesn't exist or we don't have permission */
}

int find_container_by_id(const char *base_dir,
                         const char *container_id,
                         int *out_pid,
                         char *out_status,
                         size_t status_size)
{
    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/containers.db", base_dir) >= (int)sizeof(db_path)) {
        fprintf(stderr, "[mdock] containers.db path too long\n");
        return -1;
    }

    FILE *f = fopen(db_path, "r");
    if (!f) {
        perror("[mdock] fopen containers.db");
        return -1;
    }

    char line[4096];
    int found = 0;

    while (fgets(line, sizeof(line), f)) {
        /* Parse: id|pid|image|status|start_time|end_time|exit_code */
        char *p1 = strchr(line, '|');
        if (!p1) continue;
        *p1 = '\0';
        char *id = line;

        if (strcmp(id, container_id) == 0) {
            char *p2 = strchr(p1 + 1, '|');
            if (!p2) continue;
            *p2 = '\0';
            char *pid_str = p1 + 1;

            char *p3 = strchr(p2 + 1, '|');
            if (!p3) continue;
            *p3 = '\0';

            char *p4 = strchr(p3 + 1, '|');
            if (!p4) continue;
            *p4 = '\0';
            char *status = p3 + 1;

            *out_pid = atoi(pid_str);
            if (strlen(status) + 1 > status_size) {
                fprintf(stderr, "[mdock] status buffer too small\n");
                fclose(f);
                return -1;
            }
            strcpy(out_status, status);
            found = 1;
            break;
        }
    }

    fclose(f);
    return found ? 0 : -1;
}

int update_container_status(const char *base_dir,
                            const char *container_id,
                            const char *new_status)
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
            /* Found the container, rebuild the line with new status */
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
            /* old status - we'll replace it */

            char *p5 = strchr(p4 + 1, '|');
            if (!p5) {
                fputs(line, f_out);
                continue;
            }
            *p5 = '\0';
            char *start_time = p4 + 1;

            char *p6 = strchr(p5 + 1, '|');
            if (!p6) {
                fputs(line, f_out);
                continue;
            }
            *p6 = '\0';
            char *end_time = p5 + 1;

            /* Remove trailing newline from exit_code */
            char *exit_code = p6 + 1;
            char *newline = strchr(exit_code, '\n');
            if (newline) *newline = '\0';

            /* Write updated record with new status */
            fprintf(f_out, "%s|%s|%s|%s|%s|%s|%s\n",
                    id, pid_str, image, new_status, start_time, end_time, exit_code);
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

/* ----- Issue #13: Resource limit parsing ----- */

static long parse_memory_limit(const char *str)
{
    char *endptr;
    long value = strtol(str, &endptr, 10);
    
    if (value <= 0) {
        return -1;
    }
    
    /* Check for unit suffix */
    if (*endptr == 'M' || *endptr == 'm') {
        return value * 1024 * 1024;
    } else if (*endptr == 'G' || *endptr == 'g') {
        return value * 1024 * 1024 * 1024;
    } else if (*endptr == 'K' || *endptr == 'k') {
        return value * 1024;
    } else if (*endptr == '\0') {
        /* No unit, assume bytes */
        return value;
    }
    
    return -1;
}

static long parse_cpu_limit(const char *str)
{
    char *endptr;
    long value = strtol(str, &endptr, 10);
    
    if (value <= 0) {
        return -1;
    }
    
    /* CPU limit is in seconds */
    return value;
}

/* ----- Issue #8 & #9: mdock run command ----- */

int cmd_run(int argc, char **argv)
{
    const char *image_name = NULL;
    long mem_limit = -1;
    long cpu_limit = -1;
    
    /* Parse arguments */
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mem") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[mdock] error: --mem requires a value\n");
                fprintf(stderr, "Usage: mdock run [--mem <size>] [--cpu <seconds>] <image_name>\n");
                return 1;
            }
            mem_limit = parse_memory_limit(argv[++i]);
            if (mem_limit < 0) {
                fprintf(stderr, "[mdock] error: invalid memory limit '%s'\n", argv[i]);
                fprintf(stderr, "[mdock] hint: use format like 128M, 1G, or 512M\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--cpu") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[mdock] error: --cpu requires a value\n");
                fprintf(stderr, "Usage: mdock run [--mem <size>] [--cpu <seconds>] <image_name>\n");
                return 1;
            }
            cpu_limit = parse_cpu_limit(argv[++i]);
            if (cpu_limit < 0) {
                fprintf(stderr, "[mdock] error: invalid CPU limit '%s'\n", argv[i]);
                fprintf(stderr, "[mdock] hint: specify seconds (e.g., 10 for 10 seconds)\n");
                return 1;
            }
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "[mdock] error: unknown option '%s'\n", argv[i]);
            fprintf(stderr, "Usage: mdock run [--mem <size>] [--cpu <seconds>] <image_name>\n");
            return 1;
        } else {
            /* This is the image name */
            image_name = argv[i];
            break;
        }
    }
    
    if (!image_name) {
        fprintf(stderr, "Usage: mdock run [--mem <size>] [--cpu <seconds>] <image_name>\n");
        fprintf(stderr, "\nOptions:\n");
        fprintf(stderr, "  --mem <size>      Memory limit (e.g., 128M, 1G)\n");
        fprintf(stderr, "  --cpu <seconds>   CPU time limit in seconds\n");
        fprintf(stderr, "\nExamples:\n");
        fprintf(stderr, "  mdock run myimage\n");
        fprintf(stderr, "  mdock run --mem 128M myimage\n");
        fprintf(stderr, "  mdock run --mem 256M --cpu 10 myimage\n");
        return 1;
    }

    /* Initialize ~/.mdock */
    char base_dir[PATH_MAX];
    if (mdock_init_home(base_dir, sizeof(base_dir)) != 0) {
        fprintf(stderr, "[mdock] failed to initialize home directory\n");
        return 1;
    }

    /* Lookup image rootfs */
    char rootfs_path[PATH_MAX];
    if (find_image_rootfs(base_dir, image_name, rootfs_path, sizeof(rootfs_path)) != 0) {
        fprintf(stderr, "[mdock] error: image '%s' not found\n", image_name);
        fprintf(stderr, "[mdock] hint: use 'mdock build %s <rootfs_dir>' to create it\n", image_name);
        return 1;
    }

    /* Verify rootfs directory still exists */
    struct stat st;
    if (stat(rootfs_path, &st) == -1) {
        fprintf(stderr, "[mdock] error: rootfs directory '%s' no longer exists\n", rootfs_path);
        fprintf(stderr, "[mdock] hint: the image may be corrupted, try rebuilding it\n");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[mdock] error: rootfs path '%s' is not a directory\n", rootfs_path);
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
        
        /* Set resource limits if specified */
        if (mem_limit > 0) {
            struct rlimit mem_rlimit;
            mem_rlimit.rlim_cur = mem_limit;
            mem_rlimit.rlim_max = mem_limit;
            if (setrlimit(RLIMIT_AS, &mem_rlimit) != 0) {
                perror("[mdock] setrlimit RLIMIT_AS");
                fprintf(stderr, "[mdock] warning: failed to set memory limit, continuing anyway\n");
            }
        }
        
        if (cpu_limit > 0) {
            struct rlimit cpu_rlimit;
            cpu_rlimit.rlim_cur = cpu_limit;
            cpu_rlimit.rlim_max = cpu_limit;
            if (setrlimit(RLIMIT_CPU, &cpu_rlimit) != 0) {
                perror("[mdock] setrlimit RLIMIT_CPU");
                fprintf(stderr, "[mdock] warning: failed to set CPU limit, continuing anyway\n");
            }
        }
        
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

    /* Log RUN event with resource limits if set */
    if (mem_limit > 0 && cpu_limit > 0) {
        mdock_logf("RUN container_id=%s pid=%d image=%s mem_limit=%ldM cpu_limit=%lds", 
                   container_id, pid, image_name, mem_limit / (1024*1024), cpu_limit);
    } else if (mem_limit > 0) {
        mdock_logf("RUN container_id=%s pid=%d image=%s mem_limit=%ldM", 
                   container_id, pid, image_name, mem_limit / (1024*1024));
    } else if (cpu_limit > 0) {
        mdock_logf("RUN container_id=%s pid=%d image=%s cpu_limit=%lds", 
                   container_id, pid, image_name, cpu_limit);
    } else {
        mdock_logf("RUN container_id=%s pid=%d image=%s", container_id, pid, image_name);
    }

    if (mem_limit > 0 || cpu_limit > 0) {
        printf("[mdock] Container %s started (PID %d)", container_id, pid);
        if (mem_limit > 0) {
            printf(" [mem: %ldM]", mem_limit / (1024*1024));
        }
        if (cpu_limit > 0) {
            printf(" [cpu: %lds]", cpu_limit);
        }
        printf("\n");
    } else {
        printf("[mdock] Container %s started (PID %d)\n", container_id, pid);
    }

    /* ===== Issue #9: Wait for container exit ===== */

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("[mdock] waitpid");
        return 1;
    }

    /* Extract exit code and detect resource limit violations */
    int exit_code = 0;
    const char *exit_reason = NULL;
    
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        exit_code = 128 + sig;
        
        /* Detect resource limit signals */
        if (sig == SIGXCPU) {
            exit_reason = "CPU time limit exceeded";
        } else if (sig == SIGKILL) {
            exit_reason = "killed (possibly memory limit exceeded)";
        } else {
            exit_reason = "terminated by signal";
        }
    }

    /* Update container record with exit info */
    if (update_container_exit(base_dir, container_id, exit_code) != 0) {
        fprintf(stderr, "[mdock] failed to update container exit status\n");
    }

    /* Log EXIT event */
    if (exit_reason) {
        mdock_logf("EXIT container_id=%s pid=%d exit_code=%d reason=%s", 
                   container_id, pid, exit_code, exit_reason);
    } else {
        mdock_logf("EXIT container_id=%s pid=%d exit_code=%d", container_id, pid, exit_code);
    }

    /* Print exit message */
    if (exit_reason) {
        printf("[mdock] Container %s exited with code %d (%s)\n", container_id, exit_code, exit_reason);
    } else {
        printf("[mdock] Container %s exited with code %d\n", container_id, exit_code);
    }

    return 0;
}

/* ----- Issue #10: mdock ps command ----- */

int cmd_ps(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* Initialize ~/.mdock */
    char base_dir[PATH_MAX];
    if (mdock_init_home(base_dir, sizeof(base_dir)) != 0) {
        fprintf(stderr, "[mdock] failed to initialize home directory\n");
        return 1;
    }

    char db_path[PATH_MAX];
    if (snprintf(db_path, sizeof(db_path), "%s/containers.db", base_dir) >= (int)sizeof(db_path)) {
        fprintf(stderr, "[mdock] containers.db path too long\n");
        return 1;
    }

    FILE *f = fopen(db_path, "r");
    if (!f) {
        /* If file doesn't exist, just show header with no containers */
        printf("%-8s %-8s %-12s %-10s %s\n", "ID", "PID", "IMAGE", "STATUS", "UPTIME");
        return 0;
    }

    /* Print header */
    printf("%-8s %-8s %-12s %-10s %s\n", "ID", "PID", "IMAGE", "STATUS", "UPTIME");

    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        /* Parse: id|pid|image|status|start_time|end_time|exit_code */
        char *p1 = strchr(line, '|');
        if (!p1) continue;
        *p1 = '\0';
        char *id = line;

        char *p2 = strchr(p1 + 1, '|');
        if (!p2) continue;
        *p2 = '\0';
        char *pid_str = p1 + 1;
        int pid = atoi(pid_str);

        char *p3 = strchr(p2 + 1, '|');
        if (!p3) continue;
        *p3 = '\0';
        char *image = p2 + 1;

        char *p4 = strchr(p3 + 1, '|');
        if (!p4) continue;
        *p4 = '\0';
        char *status = p3 + 1;

        char *p5 = strchr(p4 + 1, '|');
        if (!p5) continue;
        *p5 = '\0';
        char *start_time = p4 + 1;

        char *p6 = strchr(p5 + 1, '|');
        if (!p6) continue;
        *p6 = '\0';
        char *end_time = p5 + 1;

        /* Check if PID is still alive */
        char actual_status[32];
        if (strcmp(status, "running") == 0) {
            if (is_pid_alive(pid)) {
                strcpy(actual_status, "running");
            } else {
                strcpy(actual_status, "exited");
            }
        } else {
            strcpy(actual_status, status);
        }

        /* Calculate uptime */
        char uptime[32];
        calculate_uptime(start_time, end_time, uptime, sizeof(uptime));

        /* Print formatted output */
        printf("%-8s %-8d %-12s %-10s %s\n", id, pid, image, actual_status, uptime);
    }

    fclose(f);
    return 0;
}

/* ----- Issue #11: mdock stop command ----- */

int cmd_stop(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: mdock stop <container_id>\n");
        return 1;
    }

    const char *container_id = argv[1];

    /* Initialize ~/.mdock */
    char base_dir[PATH_MAX];
    if (mdock_init_home(base_dir, sizeof(base_dir)) != 0) {
        fprintf(stderr, "[mdock] failed to initialize home directory\n");
        return 1;
    }

    /* Find container */
    int pid;
    char status[64];
    if (find_container_by_id(base_dir, container_id, &pid, status, sizeof(status)) != 0) {
        fprintf(stderr, "[mdock] container '%s' not found\n", container_id);
        return 1;
    }

    /* Check if already stopped */
    if (strcmp(status, "exited") == 0 || strcmp(status, "stopped") == 0 || strcmp(status, "killed") == 0) {
        printf("[mdock] Container %s is already stopped (status: %s)\n", container_id, status);
        return 0;
    }

    /* Check if PID is alive */
    if (!is_pid_alive(pid)) {
        printf("[mdock] Container %s process (PID %d) is not running\n", container_id, pid);
        if (strcmp(status, "running") == 0) {
            /* Update status to exited if db says running but process is dead */
            update_container_status(base_dir, container_id, "exited");
        }
        return 0;
    }

    /* Send SIGTERM */
    printf("[mdock] Sending SIGTERM to container %s (PID %d)...\n", container_id, pid);
    if (kill(pid, SIGTERM) != 0) {
        perror("[mdock] kill SIGTERM");
        return 1;
    }

    /* Wait 5 seconds for graceful shutdown */
    printf("[mdock] Waiting for graceful shutdown (5 seconds)...\n");
    sleep(5);

    /* Check if still alive */
    if (is_pid_alive(pid)) {
        /* Still alive, send SIGKILL */
        printf("[mdock] Process still running, sending SIGKILL...\n");
        if (kill(pid, SIGKILL) != 0) {
            perror("[mdock] kill SIGKILL");
            return 1;
        }

        /* Wait a bit more */
        sleep(2);

        /* Update status to killed */
        if (update_container_status(base_dir, container_id, "killed") != 0) {
            fprintf(stderr, "[mdock] failed to update container status\n");
        }

        /* Log STOP event with SIGKILL */
        mdock_logf("STOP container_id=%s pid=%d signal=SIGKILL", container_id, pid);

        printf("[mdock] Container %s killed (PID %d)\n", container_id, pid);
    } else {
        /* Stopped gracefully */
        if (update_container_status(base_dir, container_id, "stopped") != 0) {
            fprintf(stderr, "[mdock] failed to update container status\n");
        }

        /* Log STOP event with SIGTERM */
        mdock_logf("STOP container_id=%s pid=%d signal=SIGTERM", container_id, pid);

        printf("[mdock] Container %s stopped gracefully (PID %d)\n", container_id, pid);
    }

    return 0;
}
