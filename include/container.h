#ifndef MDOCK_CONTAINER_H
#define MDOCK_CONTAINER_H

#include <stddef.h>

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

/* Commands */
int cmd_run(int argc, char **argv);
int cmd_ps(int argc, char **argv);
int cmd_stop(int argc, char **argv);

#endif /* MDOCK_CONTAINER_H */
