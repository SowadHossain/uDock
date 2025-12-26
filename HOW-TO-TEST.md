# 🧪 uDock Testing Guide

## Complete Testing Documentation for uDock Container Runtime

This document provides comprehensive testing procedures for all uDock features. It includes step-by-step instructions, expected outputs, and test programs for thorough validation.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Quick Start Testing](#2-quick-start-testing)
3. [Test Environment Setup](#3-test-environment-setup)
4. [Feature Tests](#4-feature-tests)
   - [Test 1: Build Images](#test-1-build-images)
   - [Test 2: List Images](#test-2-list-images)
   - [Test 3: Run Containers](#test-3-run-containers)
   - [Test 4: Environment Variables](#test-4-environment-variables)
   - [Test 5: Container Logs](#test-5-container-logs)
   - [Test 6: List Containers (ps)](#test-6-list-containers-ps)
   - [Test 7: Stop Containers](#test-7-stop-containers)
   - [Test 8: Resource Limits](#test-8-resource-limits)
   - [Test 9: Remove Containers](#test-9-remove-containers)
   - [Test 10: Remove Images](#test-10-remove-images)
5. [Real Program Tests](#5-real-program-tests)
6. [Automated Testing](#6-automated-testing)
7. [Troubleshooting](#7-troubleshooting)
8. [Test Verification Checklist](#8-test-verification-checklist)

---

## 1. Prerequisites

### System Requirements

```bash
# Check Linux version
uname -a

# Ensure you have GCC
gcc --version

# Ensure you have Make
make --version

# You'll need root access for chroot
sudo whoami
```

### Clone and Build uDock

```bash
# Clone repository
git clone https://github.com/SowadHossain/uDock.git
cd uDock

# Build the project
make clean
make

# Verify build successful
ls -lh mdock
# Expected: -rwxr-xr-x ... mdock

# Test help command
./mdock help
```

**Expected Help Output:**
```
Usage: mdock <command> [options]

Commands:
  build <name> <rootfs>     Build an image from a rootfs directory
  images                    List all images
  rmi <image>               Remove an image
  run [options] <image>     Run a container from an image
  ps                        List all containers
  stop <id>                 Stop a running container
  rm <id>                   Remove a stopped container
  logs [-f] <id>            View container logs (-f to follow)

Run Options:
  -e KEY=VALUE              Set environment variable
  --mem SIZE                Memory limit (e.g., 128M, 1G)
  --cpu SECONDS             CPU time limit in seconds
```

---

## 2. Quick Start Testing

For quick validation, use the automated setup script:

```bash
# Make scripts executable
chmod +x setup_demo.sh test_udock.sh

# Run full setup (compiles everything, creates demo image)
sudo ./setup_demo.sh

# Or run automated tests
sudo ./test_udock.sh
```

The automated test will run all tests and show pass/fail results.

---

## 3. Test Environment Setup

### 3.1 Create Base Root Filesystem

```bash
# Create directory structure
ROOTFS="/tmp/test-rootfs"
rm -rf $ROOTFS
mkdir -p $ROOTFS/{bin,lib,lib64,tmp,home,root,etc}

# Copy essential binaries
cp /bin/sh $ROOTFS/bin/
cp /bin/ls $ROOTFS/bin/
cp /bin/echo $ROOTFS/bin/
cp /bin/cat $ROOTFS/bin/
cp /bin/sleep $ROOTFS/bin/
cp /bin/ps $ROOTFS/bin/ 2>/dev/null || true

# Copy required libraries (for /bin/sh)
ldd /bin/sh | grep "=>" | awk '{print $3}' | while read lib; do
    [ -f "$lib" ] && cp "$lib" $ROOTFS/lib/
done

# Copy libraries for /bin/ls
ldd /bin/ls | grep "=>" | awk '{print $3}' | while read lib; do
    [ -f "$lib" ] && cp "$lib" $ROOTFS/lib/
done

# Copy dynamic linker
cp /lib64/ld-linux-x86-64.so.2 $ROOTFS/lib64/ 2>/dev/null || \
cp /lib/ld-linux.so.2 $ROOTFS/lib/ 2>/dev/null

# Create basic /etc files
echo "root:x:0:0:root:/root:/bin/sh" > $ROOTFS/etc/passwd
echo "root:x:0:" > $ROOTFS/etc/group

# Verify structure
echo "=== Root Filesystem Structure ==="
ls -la $ROOTFS/
echo ""
echo "=== /bin contents ==="
ls -la $ROOTFS/bin/
```

### 3.2 Compile Real Test Programs (Optional)

```bash
# Compile the real C programs
cd real_programs
make clean
make

# Verify compilation
ls -lh hello webserver stress counter filetest

# Copy to rootfs
cp hello webserver stress counter filetest /tmp/test-rootfs/bin/
chmod +x /tmp/test-rootfs/bin/*

cd ..
```

---

## 4. Feature Tests

### Test 1: Build Images

**Purpose:** Verify image creation from root filesystem.

```bash
# Build first test image
./mdock build testimg /tmp/test-rootfs

# Expected output:
# [mdock] Copying rootfs from /tmp/test-rootfs...
# [mdock] Image 'testimg' created successfully.

# Build second image for later tests
./mdock build testimg2 /tmp/test-rootfs

# Verify image was created
ls -la ~/.mdock/images/

# Check images.db
echo "=== Images Database ==="
cat ~/.mdock/images.db
```

**Expected Result:**
- Image directories created in `~/.mdock/images/`
- Entry added to `~/.mdock/images.db`
- No errors during copy

**Verification:**
```bash
# Verify rootfs was copied
ls ~/.mdock/images/testimg/rootfs/bin/
# Should show: sh, ls, echo, cat, sleep, etc.
```

---

### Test 2: List Images

**Purpose:** Verify image listing with metadata.

```bash
# List all images
./mdock images
```

**Expected Output:**
```
IMAGE        SIZE       CREATED
testimg      XX.XM      2025-12-26 HH:MM:SS
testimg2     XX.XM      2025-12-26 HH:MM:SS
```

**Verification:**
- All built images appear in the list
- Size is displayed (calculated via `du`)
- Timestamp shows creation time

---

### Test 3: Run Containers

**Purpose:** Verify container execution with process isolation.

```bash
# Run a container (requires sudo for chroot)
sudo ./mdock run testimg
```

**Inside the container, run these commands:**
```bash
# Check process ID
echo "PID: $$"

# List root directory
ls /

# Show we're isolated
pwd

# Try echo
echo "Hello from container!"

# Exit container
exit
```

**Expected Behavior:**
- Drops into a shell prompt (`/ #` or `sh-x.x#`)
- `ls /` shows only the copied rootfs contents (bin, lib, lib64, tmp, etc.)
- Process appears to be PID 1 or low number inside container
- `exit` returns to host with exit message

**Verification After Exit:**
```bash
# Check container was recorded
cat ~/.mdock/containers.db

# Should show entry like:
# c1|12345|testimg|2025-12-26T10:30:00|exited
```

---

### Test 4: Environment Variables

**Purpose:** Verify custom environment variables are passed to containers.

```bash
# Run with environment variables
sudo ./mdock run -e MY_VAR=hello -e DEBUG=true -e APP_ENV=production testimg
```

**Inside the container:**
```bash
# Check environment variables
echo "MY_VAR=$MY_VAR"
echo "DEBUG=$DEBUG"
echo "APP_ENV=$APP_ENV"

# Check default variables
echo "PATH=$PATH"
echo "HOME=$HOME"
echo "TERM=$TERM"

exit
```

**Expected Output Inside Container:**
```
MY_VAR=hello
DEBUG=true
APP_ENV=production
PATH=/usr/local/bin:/usr/bin:/bin
HOME=/root
TERM=linux
```

**Using Real Test Program (if compiled):**
```bash
sudo ./mdock run -e CUSTOM_MESSAGE="Testing env vars!" testimg

# Inside container:
/bin/hello
```

---

### Test 5: Container Logs

**Purpose:** Verify stdout/stderr capture and log viewing.

#### Test 5.1: Basic Log Capture

```bash
# Run container with output
sudo ./mdock run testimg

# Inside container, generate output:
echo "Test log line 1"
echo "Test log line 2"
ls /bin
exit
```

```bash
# View the logs (find container ID from ps first)
./mdock ps
# Note the container ID (e.g., c1)

./mdock logs c1
```

**Expected Output:**
```
Test log line 1
Test log line 2
cat   echo   ls   sh   sleep
```

#### Test 5.2: Log Following (Real-time)

**Terminal 1:**
```bash
# Start a container with continuous output
sudo ./mdock run -e COUNT=20 -e DELAY=2 testimg

# Inside container (if real programs compiled):
/bin/counter

# Or manually:
for i in 1 2 3 4 5 6 7 8 9 10; do echo "Line $i"; sleep 2; done
```

**Terminal 2:**
```bash
# Follow logs in real-time
./mdock logs -f c1

# You should see lines appear every 2 seconds
# Ctrl+C to stop following, or it auto-exits when container stops
```

---

### Test 6: List Containers (ps)

**Purpose:** Verify container listing with status and uptime.

```bash
# First, run some containers
sudo ./mdock run testimg
# Inside: exit

sudo ./mdock run testimg
# Inside: sleep 60 &
# Then: exit

# List all containers
./mdock ps
```

**Expected Output:**
```
ID       PID      IMAGE        STATUS     UPTIME
c1       12345    testimg      exited     00:01:23
c2       12400    testimg      exited     00:00:45
```

**Status Values:**
- `running` - Container process is still active
- `exited` - Container completed normally
- `stopped` - Container was stopped via `mdock stop`

---

### Test 7: Stop Containers

**Purpose:** Verify graceful container termination.

#### Test 7.1: Stop Running Container

**Terminal 1:**
```bash
# Start a long-running container
sudo ./mdock run testimg

# Inside container:
sleep 300
```

**Terminal 2:**
```bash
# List to confirm running
./mdock ps
# Should show status: running

# Stop the container
./mdock stop c1

# Expected output:
# [mdock] Sending SIGTERM to container c1...
# [mdock] Container c1 stopped.

# Verify stopped
./mdock ps
# Should show status: stopped
```

#### Test 7.2: Stop with Webserver (Real Program)

**Terminal 1:**
```bash
sudo ./mdock run -e SERVER_ENV=test testimg
# Inside: /bin/webserver
```

**Terminal 2:**
```bash
./mdock logs -f c1
# Watch the server logs
```

**Terminal 3:**
```bash
./mdock ps       # Verify running
./mdock stop c1  # Stop gracefully
./mdock ps       # Verify stopped
```

---

### Test 8: Resource Limits

**Purpose:** Verify CPU and memory constraints.

#### Test 8.1: CPU Time Limit

```bash
# Run with 5-second CPU limit
sudo ./mdock run --cpu 5 testimg

# Inside container, run CPU-intensive task:
i=0; while true; do i=$((i+1)); done

# Or with real program:
/bin/stress cpu 60

# Container should be killed after ~5 seconds of CPU time
# Expected: "Killed" or signal message
```

**Expected Behavior:**
- Container runs for approximately 5 CPU seconds
- Kernel sends SIGXCPU then SIGKILL
- Container terminates automatically

#### Test 8.2: Memory Limit

```bash
# Run with 64MB memory limit
sudo ./mdock run --mem 64M testimg

# Inside container:
/bin/stress memory

# Or try to allocate large memory manually
# The container should be constrained
```

#### Test 8.3: Combined Limits

```bash
sudo ./mdock run --mem 128M --cpu 10 -e TEST=limits testimg

# Inside:
echo "Testing with limits"
echo "Memory: 128M, CPU: 10s"
/bin/stress combined
```

---

### Test 9: Remove Containers

**Purpose:** Verify container cleanup.

#### Test 9.1: Remove Stopped Container

```bash
# List containers
./mdock ps

# Remove a stopped container
./mdock rm c1

# Expected output:
# [mdock] Container 'c1' removed successfully.

# Verify removal
./mdock ps
# c1 should no longer appear
```

#### Test 9.2: Try to Remove Running Container (Should Fail)

**Terminal 1:**
```bash
sudo ./mdock run testimg
# Inside: sleep 120
```

**Terminal 2:**
```bash
./mdock ps    # Note container ID

./mdock rm c2
# Expected error:
# Error: Cannot remove running container 'c2'
# Stop the container first using 'mdock stop c2'

# Correct procedure:
./mdock stop c2
./mdock rm c2
```

---

### Test 10: Remove Images

**Purpose:** Verify safe image removal.

#### Test 10.1: Remove Unused Image

```bash
# Remove all containers using testimg2 first
./mdock ps
./mdock rm <container_ids_using_testimg2>

# Remove the image
./mdock rmi testimg2

# Expected output:
# [mdock] Image 'testimg2' removed successfully.

# Verify removal
./mdock images
# testimg2 should not appear

# Verify filesystem deleted
ls ~/.mdock/images/
# testimg2 directory should not exist
```

#### Test 10.2: Try to Remove In-Use Image (Should Fail)

```bash
# Run a container from testimg
sudo ./mdock run testimg
# Inside: exit

# Try to remove image while containers exist
./mdock rmi testimg

# Expected error:
# Error: Cannot remove image 'testimg' - in use by container(s)

# Remove containers first
./mdock rm c1 c2 c3  # all containers

# Now it should work
./mdock rmi testimg
```

---

## 5. Real Program Tests

If you've compiled the real C programs, use these comprehensive tests:

### 5.1 Hello Program Test

```bash
# Build image with real programs
./mdock build demo /tmp/test-rootfs

# Run hello program
sudo ./mdock run -e CUSTOM_MESSAGE="Hello Faculty!" -e APP_ENV=demo demo

# Inside container:
/bin/hello
```

**Expected Output:**
```
========================================
  Hello from uDock Container!
========================================

Container Information:
  - Process ID (PID): 1
  - Parent PID (PPID): 0
  - User ID (UID): 0
  - Group ID (GID): 0

Environment Variables:
  - PATH: /usr/local/bin:/usr/bin:/bin
  - HOME: /root
  - TERM: linux
  - CUSTOM_MESSAGE: Hello Faculty!
  - APP_ENV: demo

✓ Container program executed successfully!
```

### 5.2 Webserver Test (Long-Running)

**Terminal 1:**
```bash
sudo ./mdock run -e SERVER_ENV=production -e DEBUG=false demo

# Inside container:
/bin/webserver 8080
```

**Terminal 2:**
```bash
# Follow logs - see requests being logged
./mdock logs -f c1
```

**Terminal 3:**
```bash
# Monitor, then stop
./mdock ps
# Let it run for 30 seconds to see statistics
./mdock stop c1
```

### 5.3 Counter Test (Log Following)

**Terminal 1:**
```bash
sudo ./mdock run -e COUNT=15 -e DELAY=1 demo

# Inside container:
/bin/counter
```

**Terminal 2:**
```bash
./mdock logs -f c1
# Should see:
# [HH:MM:SS] Count: 1/15
# [HH:MM:SS] Count: 2/15
# ... appearing every second
```

### 5.4 Stress Test (Resource Limits)

```bash
# CPU limit test - should be killed after 10 CPU seconds
sudo ./mdock run --cpu 10 demo

# Inside container:
/bin/stress cpu 60
# Expected: Killed after ~10 seconds
```

```bash
# Memory limit test
sudo ./mdock run --mem 128M demo

# Inside container:
/bin/stress memory
# Will show allocation up to limit
```

### 5.5 Filesystem Test

```bash
sudo ./mdock run demo

# Inside container:
/bin/filetest
```

**Expected Output:**
```
========================================
  Filesystem Test Program
========================================
PID: 1
CWD: /

[FS] Listing directory: /
----------------------------------------
d         0  .
d         0  ..
d      4096  bin
d      4096  lib
...

[FS] Testing file operations...
[FS] Writing to /tmp/udock_test.txt
[FS] ✓ Write successful
[FS] Reading from /tmp/udock_test.txt
[FS] File contents:
--- BEGIN ---
Hello from uDock container!
This is a test file.
--- END ---
[FS] ✓ Delete successful

✓ Filesystem test completed!
```

---

## 6. Automated Testing

### Run Full Test Suite

```bash
chmod +x test_udock.sh
sudo ./test_udock.sh
```

**Expected Output:**
```
========================================
uDock Comprehensive Test Suite
========================================
[INFO] Starting tests at Thu Dec 26 10:00:00 2025

========================================
Cleaning Up Test Environment
========================================
[✓] Cleanup complete

========================================
Checking Prerequisites
========================================
[✓] mdock binary found
[✓] All required commands available
[✓] Running as root

========================================
Test 1: Build Images
========================================
[✓] Image testimg1 built successfully
[✓] Image testimg2 built successfully

... (more tests) ...

========================================
Test Summary
========================================
Tests Passed: 15
Tests Failed: 0

✓ All tests passed!
```

---

## 7. Troubleshooting

### Issue: "Operation not permitted" during chroot

**Solution:** Run with sudo
```bash
sudo ./mdock run testimg
```

### Issue: "No such file or directory" when running container

**Cause:** Missing libraries in rootfs

**Solution:** Copy all required libraries
```bash
# Find dependencies
ldd /bin/sh

# Copy each one to rootfs/lib/
cp /lib/x86_64-linux-gnu/libc.so.6 /tmp/test-rootfs/lib/
```

### Issue: Container immediately exits

**Cause:** Shell can't start properly

**Solution:** Ensure dynamic linker is copied
```bash
cp /lib64/ld-linux-x86-64.so.2 /tmp/test-rootfs/lib64/
```

### Issue: Logs are empty

**Cause:** Log redirection might have failed

**Check:**
```bash
ls -la ~/.mdock/logs/
cat ~/.mdock/logs/c1.log
```

### Issue: `mdock ps` shows wrong status

**Cause:** PID checking may fail

**Verify:**
```bash
# Check if PID is actually alive
ps aux | grep <pid>
cat /proc/<pid>/status
```

---

## 8. Test Verification Checklist

Use this checklist to verify all features work correctly:

### Core Commands

- [ ] `mdock build` - Creates image and copies rootfs
- [ ] `mdock images` - Lists images with size and date
- [ ] `mdock run` - Starts container with shell
- [ ] `mdock ps` - Shows all containers with status
- [ ] `mdock stop` - Stops running container gracefully
- [ ] `mdock logs` - Shows container output
- [ ] `mdock rm` - Removes stopped container
- [ ] `mdock rmi` - Removes unused image

### Run Options

- [ ] `-e KEY=VALUE` - Environment variables are passed
- [ ] `--mem SIZE` - Memory limit is enforced
- [ ] `--cpu SECONDS` - CPU time limit is enforced
- [ ] Multiple `-e` flags work together
- [ ] Combined `--mem` and `--cpu` work together

### Log Features

- [ ] `mdock logs <id>` - Shows captured output
- [ ] `mdock logs -f <id>` - Follows logs in real-time
- [ ] Log following exits when container stops
- [ ] Both stdout and stderr are captured

### Safety Features

- [ ] Cannot remove running container
- [ ] Cannot remove in-use image
- [ ] Helpful error messages for invalid operations

### Data Persistence

- [ ] `~/.mdock/images.db` updated on build
- [ ] `~/.mdock/containers.db` updated on run
- [ ] `~/.mdock/log.txt` contains event log
- [ ] `~/.mdock/logs/<id>.log` contains container output

### Process Management

- [ ] Container gets isolated PID
- [ ] Container runs in chroot jail
- [ ] Container inherits environment variables
- [ ] SIGTERM stops container gracefully
- [ ] SIGKILL used as fallback

---

## Summary

This testing guide covers all uDock features:

1. **Image Management**: build, list, remove
2. **Container Lifecycle**: run, list, stop, remove
3. **Configuration**: environment variables, resource limits
4. **Logging**: output capture and real-time following
5. **Safety**: validation and error handling

Run through all tests to ensure your uDock installation is working correctly.

---

**Document Version:** 1.0  
**Last Updated:** December 2025  
**Author:** Sowad Hossain
