/*
 * Simple Counter Program for uDock
 * Good for testing log following
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int count = 10;
    int delay = 0;  /* No delay by default for fast demo */
    
    // Get from environment or arguments
    char *count_env = getenv("COUNT");
    char *delay_env = getenv("DELAY");
    
    if (count_env) count = atoi(count_env);
    if (delay_env) delay = atoi(delay_env);
    
    if (argc > 1) count = atoi(argv[1]);
    if (argc > 2) delay = atoi(argv[2]);
    
    printf("========================================\n");
    printf("  Counter Program\n");
    printf("========================================\n");
    if (delay > 0) {
        printf("Counting from 1 to %d with %d second delay\n", count, delay);
    } else {
        printf("Counting from 1 to %d (fast mode)\n", count);
    }
    printf("PID: %d\n", getpid());
    printf("========================================\n\n");
    
    for (int i = 1; i <= count; i++) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
        
        printf("[%s] Count: %d/%d\n", timestamp, i, count);
        fflush(stdout);
        
        if (i < count && delay > 0) {
            sleep(delay);
        } else if (i % 10 == 0) {
            usleep(50000);  /* Small delay every 10 items to see output */
        }
    }
    
    printf("\n========================================\n");
    printf("✓ Counter Complete! Final count: %d\n", count);
    printf("========================================\n");
    
    return 0;
}
