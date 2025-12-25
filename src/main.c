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
            "  build  <image_name> <rootfs_dir>     Build a new image\n"
            "  images                                List all images\n"
            "  rmi    <image_name>                   Remove an image\n"
            "  run    [OPTIONS] <image_name>         Run a container\n"
            "  ps                                    List containers\n"
            "  stop   <container_id>                 Stop a container\n"
            "  rm     <container_id>                 Remove a stopped container\n"
            "  logs   [-f] <container_id>            View container logs\n"
            "\n"
            "Run Options:\n"
            "  --mem <size>       Memory limit (e.g., 128M, 1G)\n"
            "  --cpu <seconds>    CPU time limit in seconds\n"
            "  -e KEY=VALUE       Set environment variable\n"
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
    } else if (strcmp(cmd, "images") == 0) {
        return cmd_images(argc - 1, &argv[1]);
    } else if (strcmp(cmd, "rmi") == 0) {
        return cmd_rmi(argc - 1, &argv[1]);
    } else if (strcmp(cmd, "run") == 0) {
        return cmd_run(argc - 1, &argv[1]);
    } else if (strcmp(cmd, "ps") == 0) {
        return cmd_ps(argc - 1, &argv[1]);
    } else if (strcmp(cmd, "stop") == 0) {
        return cmd_stop(argc - 1, &argv[1]);
    } else if (strcmp(cmd, "rm") == 0) {
        return cmd_rm(argc - 1, &argv[1]);
    } else if (strcmp(cmd, "logs") == 0) {
        return cmd_logs(argc - 1, &argv[1]);
    } else {
        fprintf(stderr, "Unknown command: %s\n\n", cmd);
        print_usage(argv[0]);
        return 1;
    }
}
