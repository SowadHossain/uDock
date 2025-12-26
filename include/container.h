#ifndef MDOCK_CONTAINER_H
#define MDOCK_CONTAINER_H

#include <stddef.h>
#include <sys/types.h>

/* Container database helpers */
int add_container_record(const char *base_dir,
                         const char *container_id,
                         int pid,
                         const char *image_name);

int update_container_exit(const char *base_dir,
                          const char *container_id,
                          int exit_code);

/* Generate next container ID (c1, c2, ...) */
int generate_container_id(const char *base_dir,
                          char *out_id,
                          size_t out_size);

/* Check if a PID is still alive */
int is_pid_alive(pid_t pid);

/* Find container by ID and get its PID and status */
int find_container_by_id(const char *base_dir,
                         const char *container_id,
                         int *out_pid,
                         char *out_status,
                         size_t status_size);

/* Update container status field */
int update_container_status(const char *base_dir,
                            const char *container_id,
                            const char *new_status);

/* Commands */
int cmd_run(int argc, char **argv);
int cmd_ps(int argc, char **argv);
int cmd_stop(int argc, char **argv);
int cmd_rm(int argc, char **argv);
int cmd_logs(int argc, char **argv);

#endif /* MDOCK_CONTAINER_H */
