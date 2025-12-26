/*
 * Simple Web Server Simulator for uDock
 * Demonstrates long-running container processes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>

volatile sig_atomic_t keep_running = 1;

void handle_signal(int sig) {
    printf("\n[SERVER] Received signal %d, shutting down gracefully...\n", sig);
    keep_running = 0;
}

void log_request(int request_num, const char *method, const char *path, int status) {
    time_t now;
    struct tm *tm_info;
    char timestamp[64];
    
    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    printf("[%s] %s %s - %d (Request #%d)\n", 
           timestamp, method, path, status, request_num);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    int port = 8080;
    int request_count = 0;
    
    // Check for custom port in arguments
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    
    // Check for environment variables
    char *server_env = getenv("SERVER_ENV");
    char *debug = getenv("DEBUG");
    
    // Setup signal handlers
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    
    printf("========================================\n");
    printf("  uDock Web Server (Simulator)\n");
    printf("========================================\n\n");
    
    printf("[SERVER] Starting server...\n");
    printf("[SERVER] PID: %d\n", getpid());
    printf("[SERVER] Port: %d\n", port);
    printf("[SERVER] Environment: %s\n", server_env ? server_env : "development");
    printf("[SERVER] Debug mode: %s\n", debug ? debug : "false");
    printf("\n");
    
    sleep(1);
    printf("[SERVER] Loading configuration...\n");
    sleep(1);
    printf("[SERVER] Binding to port %d...\n", port);
    sleep(1);
    printf("[SERVER] Server ready to accept connections!\n");
    printf("[SERVER] Press Ctrl+C or send SIGTERM to stop\n\n");
    
    // Simulate incoming requests
    const char *methods[] = {"GET", "POST", "PUT", "DELETE"};
    const char *paths[] = {
        "/index.html",
        "/api/users",
        "/api/products",
        "/static/style.css",
        "/static/app.js",
        "/api/login",
        "/api/logout",
        "/health"
    };
    
    int num_methods = sizeof(methods) / sizeof(methods[0]);
    int num_paths = sizeof(paths) / sizeof(paths[0]);
    
    while (keep_running) {
        sleep(3);  // Wait 3 seconds between "requests"
        
        if (!keep_running) break;
        
        request_count++;
        
        // Simulate random request
        int method_idx = request_count % num_methods;
        int path_idx = request_count % num_paths;
        int status = (request_count % 10 == 0) ? 404 : 200;
        
        log_request(request_count, methods[method_idx], paths[path_idx], status);
        
        // Print statistics every 10 requests
        if (request_count % 10 == 0) {
            printf("\n[SERVER] === Statistics ===\n");
            printf("[SERVER] Total requests handled: %d\n", request_count);
            printf("[SERVER] Server uptime: %d seconds\n", request_count * 3);
            printf("[SERVER] Memory: OK, CPU: Normal\n\n");
        }
    }
    
    printf("\n[SERVER] Server stopped after handling %d requests\n", request_count);
    printf("[SERVER] Cleanup complete. Goodbye!\n");
    
    return 0;
}
