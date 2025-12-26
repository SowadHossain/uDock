/*
 * CPU Stress Test for uDock
 * Tests CPU limits and resource constraints
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

volatile sig_atomic_t keep_running = 1;

void handle_signal(int sig) {
    printf("\n[STRESS] Received signal %d (killed by resource limit)\n", sig);
    keep_running = 0;
}

// Calculate prime numbers (CPU intensive)
int is_prime(unsigned long n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    
    for (unsigned long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;
    }
    return 1;
}

void cpu_stress_test(int duration_seconds) {
    printf("\n[CPU STRESS] Starting CPU intensive test...\n");
    printf("[CPU STRESS] Will run until CPU limit (approx %d seconds)\n", duration_seconds);
    printf("[CPU STRESS] Finding prime numbers...\n\n");
    
    time_t start_time = time(NULL);
    unsigned long num = 2;
    int prime_count = 0;
    
    /* Run forever until killed by CPU limit */
    while (keep_running) {
        if (is_prime(num)) {
            prime_count++;
            
            if (prime_count % 500 == 0) {  /* Report more frequently */
                int elapsed = time(NULL) - start_time;
                printf("[CPU STRESS] Found %d primes in %d seconds (current: %lu)\n", 
                       prime_count, elapsed, num);
                fflush(stdout);
            }
        }
        num++;
    }
    
    int total_time = time(NULL) - start_time;
    printf("\n[CPU STRESS] Test completed!\n");
    printf("[CPU STRESS] Total primes found: %d\n", prime_count);
    printf("[CPU STRESS] Total time: %d seconds\n", total_time);
    printf("[CPU STRESS] Rate: %.2f primes/second\n", (float)prime_count / total_time);
}

void memory_stress_test() {
    printf("\n[MEMORY STRESS] Starting memory allocation test...\n");
    
    size_t chunk_size = 5 * 1024 * 1024;  // 5 MB chunks (faster to hit limits)
    int chunks = 0;
    void *allocations[200];
    
    for (int i = 0; i < 200 && keep_running; i++) {
        void *ptr = malloc(chunk_size);
        if (ptr == NULL) {
            printf("[MEMORY STRESS] Allocation failed at %d MB\n", chunks * 5);
            break;
        }
        
        // Touch the memory to actually allocate it
        char *cptr = (char *)ptr;
        for (size_t j = 0; j < chunk_size; j += 4096) {
            cptr[j] = 1;
        }
        
        allocations[i] = ptr;
        chunks++;
        
        if (chunks % 5 == 0) {  /* Report every 25 MB */
            printf("[MEMORY STRESS] Allocated %d MB\n", chunks * 5);
            fflush(stdout);
        }
    }
    
    printf("\n[MEMORY STRESS] Total allocated: %d MB\n", chunks * 10);
    printf("[MEMORY STRESS] Cleaning up...\n");
    
    // Free allocations
    for (int i = 0; i < chunks; i++) {
        free(allocations[i]);
    }
    
    printf("[MEMORY STRESS] Memory stress test completed\n");
}

void combined_stress_test() {
    printf("\n[COMBINED STRESS] Running CPU and memory stress together...\n");
    
    // Allocate some memory
    size_t mem_size = 50 * 1024 * 1024;  // 50 MB
    void *mem = malloc(mem_size);
    if (mem) {
        printf("[COMBINED STRESS] Allocated 50 MB\n");
        // Touch it
        char *cptr = (char *)mem;
        for (size_t i = 0; i < mem_size; i += 4096) {
            cptr[i] = 1;
        }
    }
    
    // Run CPU stress for 30 seconds
    cpu_stress_test(30);
    
    if (mem) {
        free(mem);
        printf("[COMBINED STRESS] Memory freed\n");
    }
}

int main(int argc, char *argv[]) {
    // Setup signal handlers
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    signal(SIGXCPU, handle_signal);  // CPU limit exceeded
    
    printf("========================================\n");
    printf("  uDock Resource Stress Test\n");
    printf("========================================\n");
    printf("PID: %d\n", getpid());
    printf("Use with: --cpu <seconds> or --mem <MB>\n");
    printf("========================================\n");
    
    // Determine test type
    if (argc > 1) {
        if (strcmp(argv[1], "cpu") == 0) {
            int duration = (argc > 2) ? atoi(argv[2]) : 60;
            cpu_stress_test(duration);
        } else if (strcmp(argv[1], "memory") == 0) {
            memory_stress_test();
        } else if (strcmp(argv[1], "combined") == 0) {
            combined_stress_test();
        } else {
            printf("\nUsage:\n");
            printf("  %s cpu [seconds]    - CPU stress test\n", argv[0]);
            printf("  %s memory           - Memory stress test\n", argv[0]);
            printf("  %s combined         - Combined stress test\n", argv[0]);
            return 1;
        }
    } else {
        // Default: run combined test
        combined_stress_test();
    }
    
    printf("\n✓ Stress test completed successfully!\n");
    return 0;
}
