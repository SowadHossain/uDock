#ifndef MDOCK_IMAGE_H
#define MDOCK_IMAGE_H

#include <stddef.h>

int cmd_build(int argc, char **argv);
int cmd_images(int argc, char **argv);
int cmd_rmi(int argc, char **argv);

/* Initialize ~/.mdock and return its path in out_base_dir */
int mdock_init_home(char *out_base_dir, size_t size);

/* Lookup rootfs path for an image (used later in run) */
int find_image_rootfs(const char *base_dir,
                      const char *image_name,
                      char *out_path,
                      size_t out_size);

/* Check if image is in use by any container */
int image_in_use(const char *base_dir, const char *image_name);

#endif /* MDOCK_IMAGE_H */
