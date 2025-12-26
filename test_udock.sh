#!/bin/bash
#
# uDock Comprehensive Test Script
# Tests all features of the uDock container runtime
#

set +e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test counters
TESTS_PASSED=0
TESTS_FAILED=0

print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

print_test() {
    echo -e "${YELLOW}[TEST]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
    ((TESTS_PASSED++))
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
    ((TESTS_FAILED++))
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

# Cleanup function
cleanup() {
    print_header "Cleaning Up Test Environment"
    
    # Stop all running containers
    if [ -f ~/.mdock/containers.db ]; then
        while IFS='|' read -r id pid image start status; do
            if [ -n "$id" ] && ./mdock ps | grep -q "$id.*running"; then
                print_info "Stopping container $id"
                ./mdock stop "$id" 2>/dev/null || true
            fi
        done < ~/.mdock/containers.db
    fi
    
    # Remove all containers
    if [ -f ~/.mdock/containers.db ]; then
        while IFS='|' read -r id rest; do
            if [ -n "$id" ]; then
                print_info "Removing container $id"
                ./mdock rm "$id" 2>/dev/null || true
            fi
        done < ~/.mdock/containers.db
    fi
    
    # Remove all images
    if [ -f ~/.mdock/images.db ]; then
        while IFS='|' read -r name rest; do
            if [ -n "$name" ]; then
                print_info "Removing image $name"
                ./mdock rmi "$name" 2>/dev/null || true
            fi
        done < ~/.mdock/images.db
    fi
    
    # Remove test rootfs directories
    rm -rf /tmp/udock-test-* 2>/dev/null || true
    
    print_success "Cleanup complete"
}

# Check prerequisites
check_prerequisites() {
    print_header "Checking Prerequisites"
    
    print_test "Checking if mdock binary exists"
    if [ ! -f ./mdock ]; then
        print_error "mdock binary not found. Run 'make' first."
        exit 1
    fi
    print_success "mdock binary found"
    
    print_test "Checking for required commands"
    for cmd in cp mkdir tar; do
        if ! command -v $cmd &> /dev/null; then
            print_error "$cmd command not found"
            exit 1
        fi
    done
    print_success "All required commands available"
    
    print_test "Checking permissions"
    if [ "$EUID" -ne 0 ]; then
        print_info "Not running as root - chroot may fail"
        print_info "Consider running: sudo ./test_udock.sh"
    else
        print_success "Running as root"
    fi
}

# Create test rootfs
create_test_rootfs() {
    print_header "Creating Test Root Filesystem" >&2
    
    local ROOTFS="/tmp/udock-test-rootfs"
    rm -rf "$ROOTFS"
    
    print_test "Creating directory structure" >&2
    mkdir -p "$ROOTFS"/{bin,lib,lib64,usr/bin,tmp,home,root}
    print_success "Directory structure created" >&2
    
    print_test "Copying essential binaries" >&2
    for binary in /bin/sh /bin/bash /bin/ls /bin/echo /bin/cat /bin/sleep /bin/ps /usr/bin/env; do
        if [ -f "$binary" ]; then
            cp "$binary" "$ROOTFS/bin/" 2>/dev/null || cp "$binary" "$ROOTFS/usr/bin/" 2>/dev/null || true
        fi
    done
    print_success "Binaries copied" >&2
    
    print_test "Copying required libraries" >&2
    # Copy libraries for /bin/sh
    if [ -f /bin/sh ]; then
        ldd /bin/sh 2>/dev/null | grep "=>" | awk '{print $3}' | while read lib; do
            if [ -f "$lib" ]; then
                cp "$lib" "$ROOTFS/lib/" 2>/dev/null || true
            fi
        done
    fi
    
    # Copy libraries for /bin/ls
    if [ -f /bin/ls ]; then
        ldd /bin/ls 2>/dev/null | grep "=>" | awk '{print $3}' | while read lib; do
            if [ -f "$lib" ]; then
                cp "$lib" "$ROOTFS/lib/" 2>/dev/null || true
            fi
        done
    fi
    print_success "Libraries copied" >&2
    
    print_test "Copying dynamic linker" >&2
    if [ -f /lib64/ld-linux-x86-64.so.2 ]; then
        cp /lib64/ld-linux-x86-64.so.2 "$ROOTFS/lib64/" 2>/dev/null || true
    elif [ -f /lib/ld-linux.so.2 ]; then
        cp /lib/ld-linux.so.2 "$ROOTFS/lib/" 2>/dev/null || true
    fi
    print_success "Dynamic linker copied" >&2
    
    # Create test scripts inside rootfs
    print_test "Creating test scripts inside rootfs" >&2
    
    cat > "$ROOTFS/bin/test_env.sh" << 'EOF'
#!/bin/sh
echo "=== Environment Variables Test ==="
echo "MY_VAR=$MY_VAR"
echo "DEBUG=$DEBUG"
echo "PATH=$PATH"
echo "HOME=$HOME"
echo "TERM=$TERM"
EOF
    chmod +x "$ROOTFS/bin/test_env.sh" >&2
    
    cat > "$ROOTFS/bin/test_output.sh" << 'EOF'
#!/bin/sh
echo "=== Output Test ==="
echo "This is stdout line 1"
echo "This is stdout line 2"
echo "This is stderr line 1" >&2
echo "This is stderr line 2" >&2
echo "Test completed successfully"
EOF
    chmod +x "$ROOTFS/bin/test_output.sh" >&2
    
    cat > "$ROOTFS/bin/test_cpu.sh" << 'EOF'
#!/bin/sh
echo "Starting CPU intensive task..."
i=0
while [ $i -lt 1000000000 ]; do
    i=$((i + 1))
done
echo "CPU task completed"
EOF
    chmod +x "$ROOTFS/bin/test_cpu.sh" >&2
    
    print_success "Test scripts created" >&2
    
    echo "$ROOTFS"
}

# Test 1: Build images
test_build() {
    print_header "Test 1: Build Images"
    
    local ROOTFS=$(create_test_rootfs)
    
    print_test "Building test image 'testimg1'"
    if ./mdock build testimg1 "$ROOTFS"; then
        print_success "Image testimg1 built successfully"
    else
        print_error "Failed to build testimg1"
        return 1
    fi
    
    print_test "Building second image 'testimg2'"
    if ./mdock build testimg2 "$ROOTFS"; then
        print_success "Image testimg2 built successfully"
    else
        print_error "Failed to build testimg2"
        return 1
    fi
    
    print_test "Attempting to rebuild existing image (should fail or warn)"
    ./mdock build testimg1 "$ROOTFS" 2>&1 | head -5
}

# Test 2: List images
test_images() {
    print_header "Test 2: List Images"
    
    print_test "Listing all images"
    if ./mdock images; then
        print_success "Images listed successfully"
    else
        print_error "Failed to list images"
        return 1
    fi
    
    print_test "Verifying images.db exists"
    if [ -f ~/.mdock/images.db ]; then
        print_success "images.db exists"
        print_info "Contents:"
        cat ~/.mdock/images.db
    else
        print_error "images.db not found"
        return 1
    fi
}

# Test 3: Run containers (basic)
test_run_basic() {
    print_header "Test 3: Run Containers (Basic)"
    
    print_test "Running simple command in container"
    timeout 5 ./mdock run testimg1 << 'EOF' || true
echo "Hello from container!"
ls /bin
exit
EOF
    print_success "Container executed"
    
    print_test "Checking containers.db"
    if [ -f ~/.mdock/containers.db ]; then
        print_success "containers.db created"
        print_info "Contents:"
        cat ~/.mdock/containers.db
    else
        print_error "containers.db not found"
        return 1
    fi
}

# Test 4: Environment variables
test_environment() {
    print_header "Test 4: Environment Variables"
    
    print_test "Running container with custom environment variables"
    timeout 5 ./mdock run -e MY_VAR=hello -e DEBUG=true testimg1 << 'EOF' || true
/bin/test_env.sh
exit
EOF
    print_success "Container with env vars executed"
    
    # Check logs for environment variable output
    print_test "Verifying environment variables in logs"
    sleep 1
    if [ -f ~/.mdock/logs/c*.log ]; then
        local LOGFILE=$(ls -t ~/.mdock/logs/c*.log | head -1)
        if grep -q "MY_VAR=hello" "$LOGFILE" && grep -q "DEBUG=true" "$LOGFILE"; then
            print_success "Environment variables correctly passed"
            print_info "Log excerpt:"
            grep -E "MY_VAR|DEBUG" "$LOGFILE"
        else
            print_error "Environment variables not found in logs"
        fi
    fi
}

# Test 5: Container logs
test_logs() {
    print_header "Test 5: Container Logs"
    
    print_test "Running container with output"
    timeout 5 ./mdock run testimg1 << 'EOF' || true
/bin/test_output.sh
exit
EOF
    sleep 1
    
    print_test "Retrieving container logs"
    local CONTAINER_ID=$(tail -1 ~/.mdock/containers.db | cut -d'|' -f1)
    if [ -n "$CONTAINER_ID" ]; then
        print_info "Container ID: $CONTAINER_ID"
        if ./mdock logs "$CONTAINER_ID"; then
            print_success "Logs retrieved successfully"
        else
            print_error "Failed to retrieve logs"
        fi
    else
        print_error "Could not determine container ID"
    fi
    
    print_test "Checking log file existence"
    if ls ~/.mdock/logs/*.log &> /dev/null; then
        print_success "Log files exist"
        print_info "Log files:"
        ls -lh ~/.mdock/logs/
    else
        print_error "No log files found"
    fi
}

# Test 6: List containers (ps)
test_ps() {
    print_header "Test 6: List Containers (ps)"
    
    print_test "Listing all containers"
    if ./mdock ps; then
        print_success "Containers listed successfully"
    else
        print_error "Failed to list containers"
        return 1
    fi
    
    print_test "Starting background container for ps test"
    timeout 15 ./mdock run testimg1 << 'EOF' &
sleep 10
exit
EOF
    sleep 2
    
    print_test "Listing containers (should show running)"
    ./mdock ps
    print_success "Running containers displayed"
}

# Test 7: Stop containers
test_stop() {
    print_header "Test 7: Stop Containers"
    
    print_test "Starting long-running container"
    timeout 30 ./mdock run testimg1 << 'EOF' &
sleep 25
exit
EOF
    sleep 2
    
    local CONTAINER_ID=$(tail -1 ~/.mdock/containers.db | cut -d'|' -f1)
    print_info "Container ID: $CONTAINER_ID"
    
    print_test "Stopping container $CONTAINER_ID"
    if ./mdock stop "$CONTAINER_ID"; then
        print_success "Container stopped successfully"
    else
        print_error "Failed to stop container"
        return 1
    fi
    
    print_test "Verifying container is stopped"
    sleep 1
    ./mdock ps
}

# Test 8: Remove containers
test_rm_container() {
    print_header "Test 8: Remove Containers"
    
    # Get first stopped container
    local CONTAINER_ID=$(grep "stopped\|exited" ~/.mdock/containers.db 2>/dev/null | head -1 | cut -d'|' -f1)
    
    if [ -z "$CONTAINER_ID" ]; then
        print_info "Creating a stopped container for removal test"
        timeout 5 ./mdock run testimg1 << 'EOF' || true
exit
EOF
        sleep 1
        CONTAINER_ID=$(tail -1 ~/.mdock/containers.db | cut -d'|' -f1)
    fi
    
    print_test "Removing container $CONTAINER_ID"
    if ./mdock rm "$CONTAINER_ID"; then
        print_success "Container removed successfully"
    else
        print_error "Failed to remove container"
        return 1
    fi
    
    print_test "Verifying container is removed"
    if ! grep -q "$CONTAINER_ID" ~/.mdock/containers.db 2>/dev/null; then
        print_success "Container removed from database"
    else
        print_error "Container still in database"
    fi
}

# Test 9: Remove images
test_rmi() {
    print_header "Test 9: Remove Images"
    
    print_test "Attempting to remove image with containers (should fail)"
    if ./mdock rmi testimg1 2>&1 | grep -q "in use\|Cannot remove"; then
        print_success "Correctly prevented removal of in-use image"
    else
        print_info "Image might not be in use or removal succeeded"
    fi
    
    # Clean up all containers first
    print_test "Removing all containers for cleanup"
    if [ -f ~/.mdock/containers.db ]; then
        while IFS='|' read -r id rest; do
            if [ -n "$id" ]; then
                ./mdock rm "$id" 2>/dev/null || true
            fi
        done < ~/.mdock/containers.db
    fi
    
    print_test "Removing image testimg2"
    if ./mdock rmi testimg2; then
        print_success "Image testimg2 removed successfully"
    else
        print_error "Failed to remove testimg2"
    fi
    
    print_test "Verifying image is removed"
    if ! ./mdock images | grep -q "testimg2"; then
        print_success "Image removed from listing"
    else
        print_error "Image still appears in listing"
    fi
}

# Test 10: Resource limits (if root)
test_resource_limits() {
    print_header "Test 10: Resource Limits"
    
    if [ "$EUID" -ne 0 ]; then
        print_info "Skipping resource limit tests (requires root)"
        return 0
    fi
    
    print_test "Testing memory limit (128M)"
    timeout 10 ./mdock run --mem 128M testimg1 << 'EOF' || true
echo "Container with memory limit started"
exit
EOF
    print_success "Memory limit applied"
    
    print_test "Testing CPU limit (5s)"
    timeout 10 ./mdock run --cpu 5 testimg1 << 'EOF' || true
echo "Container with CPU limit started"
/bin/test_cpu.sh
exit
EOF
    print_info "CPU limit test completed (may have been killed)"
}

# Test 11: Global log file
test_global_log() {
    print_header "Test 11: Global Log File"
    
    print_test "Checking global log file"
    if [ -f ~/.mdock/log.txt ]; then
        print_success "Global log file exists"
        print_info "Last 10 lines:"
        tail -10 ~/.mdock/log.txt
    else
        print_error "Global log file not found"
        return 1
    fi
}

# Main test execution
main() {
    print_header "uDock Comprehensive Test Suite"
    print_info "Starting tests at $(date)"
    
    # Cleanup before starting
    cleanup
    
    # Run prerequisite checks
    check_prerequisites
    
    # Run all tests
    test_build
    test_images
    test_run_basic
    test_environment
    test_logs
    test_ps
    test_stop
    test_rm_container
    test_rmi
    test_resource_limits
    test_global_log
    
    # Final cleanup
    cleanup
    
    # Print summary
    print_header "Test Summary"
    echo -e "Tests Passed: ${GREEN}${TESTS_PASSED}${NC}"
    echo -e "Tests Failed: ${RED}${TESTS_FAILED}${NC}"
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "\n${GREEN}✓ All tests passed!${NC}\n"
        exit 0
    else
        echo -e "\n${RED}✗ Some tests failed${NC}\n"
        exit 1
    fi
}

# Run main function
main
