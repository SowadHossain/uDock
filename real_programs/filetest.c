/*
 * File I/O Test Program for uDock
 * Tests filesystem isolation and I/O operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

void list_directory(const char *path) {
    printf("\n[FS] Listing directory: %s\n", path);
    printf("----------------------------------------\n");
    
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Get file stats
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            char type = '-';
            if (S_ISDIR(st.st_mode)) type = 'd';
            else if (S_ISLNK(st.st_mode)) type = 'l';
            
            printf("%c  %8ld  %s\n", type, st.st_size, entry->d_name);
        } else {
            printf("?  %8s  %s\n", "???", entry->d_name);
        }
    }
    
    closedir(dir);
    printf("----------------------------------------\n");
}

void test_file_operations() {
    printf("\n[FS] Testing file operations...\n");
    
    const char *test_file = "/tmp/udock_test.txt";
    const char *test_data = "Hello from uDock container!\nThis is a test file.\n";
    
    // Write test
    printf("[FS] Writing to %s\n", test_file);
    FILE *f = fopen(test_file, "w");
    if (!f) {
        perror("fopen (write)");
        return;
    }
    
    fprintf(f, "%s", test_data);
    fclose(f);
    printf("[FS] ✓ Write successful\n");
    
    // Read test
    printf("[FS] Reading from %s\n", test_file);
    f = fopen(test_file, "r");
    if (!f) {
        perror("fopen (read)");
        return;
    }
    
    char buffer[256];
    printf("[FS] File contents:\n");
    printf("--- BEGIN ---\n");
    while (fgets(buffer, sizeof(buffer), f)) {
        printf("%s", buffer);
    }
    printf("--- END ---\n");
    fclose(f);
    
    // Stat test
    struct stat st;
    if (stat(test_file, &st) == 0) {
        printf("[FS] File size: %ld bytes\n", st.st_size);
    }
    
    // Delete test
    printf("[FS] Deleting %s\n", test_file);
    if (unlink(test_file) == 0) {
        printf("[FS] ✓ Delete successful\n");
    } else {
        perror("unlink");
    }
}

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("  Filesystem Test Program\n");
    printf("========================================\n");
    printf("PID: %d\n", getpid());
    printf("CWD: %s\n", getcwd(NULL, 0));
    printf("========================================\n");
    
    // List root directory
    list_directory("/");
    
    // List /bin if it exists
    list_directory("/bin");
    
    // List /tmp if it exists
    list_directory("/tmp");
    
    // Test file operations
    test_file_operations();
    
    printf("\n✓ Filesystem test completed!\n");
    
    return 0;
}
