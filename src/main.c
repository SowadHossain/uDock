#include <stdio.h>
#include <string.h>

#include "image.h"
#include "container.h"

static void print_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s <command> [args]\n"
            "\n"
            "Commands:\n"
            "  build <image_name> <rootfs_dir>\n"
            "  run   <image_name>\n"
            "  ps\n"
            "  stop  <container_id>\n"
            "\n",
            prog);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "build") == 0) {
        return cmd_build(argc - 1, &argv[1]);
    } else if (strcmp(cmd, "run") == 0) {
        return cmd_run(argc - 1, &argv[1]);
    } else if (strcmp(cmd, "ps") == 0) {
        return cmd_ps(argc - 1, &argv[1]);
    } else if (strcmp(cmd, "stop") == 0) {
        return cmd_stop(argc - 1, &argv[1]);
    } else {
        fprintf(stderr, "Unknown command: %s\n\n", cmd);
        print_usage(argv[0]);
        return 1;
    }
}
