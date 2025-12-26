/*
 * Simple Hello World program for uDock containers
 * Demonstrates basic container execution
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("  Hello from uDock Container!\n");
    printf("========================================\n\n");
    
    printf("Container Information:\n");
    printf("  - Process ID (PID): %d\n", getpid());
    printf("  - Parent PID (PPID): %d\n", getppid());
    printf("  - User ID (UID): %d\n", getuid());
    printf("  - Group ID (GID): %d\n", getgid());
    
    printf("\nEnvironment Variables:\n");
    printf("  - PATH: %s\n", getenv("PATH") ? getenv("PATH") : "(not set)");
    printf("  - HOME: %s\n", getenv("HOME") ? getenv("HOME") : "(not set)");
    printf("  - TERM: %s\n", getenv("TERM") ? getenv("TERM") : "(not set)");
    
    // Custom environment variables
    char *custom = getenv("CUSTOM_MESSAGE");
    if (custom) {
        printf("  - CUSTOM_MESSAGE: %s\n", custom);
    }
    
    char *app_env = getenv("APP_ENV");
    if (app_env) {
        printf("  - APP_ENV: %s\n", app_env);
    }
    
    printf("\nArguments passed:\n");
    for (int i = 0; i < argc; i++) {
        printf("  argv[%d] = %s\n", i, argv[i]);
    }
    
    printf("\n✓ Container program executed successfully!\n");
    
    return 0;
}
