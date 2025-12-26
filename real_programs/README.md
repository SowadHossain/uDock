# Real C Programs for uDock Containers

This directory contains **real compiled C programs** that can be run inside uDock containers for testing and demonstration.

---

## Programs

### 1. `hello.c` - Basic Information Display
Displays container information, environment variables, and process details.

**Features:**
- Shows PID, PPID, UID, GID
- Displays environment variables (PATH, HOME, custom vars)
- Shows command-line arguments

### 2. `webserver.c` - Simulated Web Server
A long-running program that simulates a web server logging requests.

**Features:**
- Runs indefinitely until stopped
- Logs simulated HTTP requests every 3 seconds
- Handles SIGTERM gracefully
- Displays statistics every 10 requests
- Perfect for testing `mdock logs -f` and `mdock stop`

### 3. `stress.c` - Resource Stress Testing
Tests CPU and memory limits with intensive operations.

**Features:**
- CPU stress: Calculate prime numbers
- Memory stress: Allocate large chunks
- Combined stress test
- Perfect for testing `--cpu` and `--mem` limits

### 4. `counter.c` - Simple Counter
Counts from 1 to N with timestamps.

**Features:**
- Configurable count and delay via env vars or args
- Timestamps each count
- Perfect for testing log following

### 5. `filetest.c` - Filesystem Testing
Tests filesystem isolation and I/O operations.

**Features:**
- Lists directories (/, /bin, /tmp)
- Creates, reads, and deletes test files
- Shows file metadata
- Demonstrates filesystem isolation

---

## Compilation Script

A Makefile is provided to compile all programs:

```bash
cd real_programs
make all          # Compile all programs
make clean        # Remove binaries
```

Or compile individually:
```bash
gcc -o hello hello.c
gcc -o webserver webserver.c
gcc -o stress stress.c
gcc -o counter counter.c
gcc -o filetest filetest.c
```

For static binaries (easier to run in minimal containers):
```bash
gcc -static -o hello hello.c
gcc -static -o webserver webserver.c
gcc -static -o stress stress.c
gcc -static -o counter counter.c
gcc -static -o filetest filetest.c
```

---

## Usage in Containers

### Step 1: Compile the Programs

```bash
cd real_programs
make all
```

### Step 2: Create Rootfs with Programs

```bash
# Create rootfs structure
mkdir -p /tmp/real-rootfs/{bin,lib,lib64,tmp}

# Copy compiled programs
cp real_programs/hello /tmp/real-rootfs/bin/
cp real_programs/webserver /tmp/real-rootfs/bin/
cp real_programs/stress /tmp/real-rootfs/bin/
cp real_programs/counter /tmp/real-rootfs/bin/
cp real_programs/filetest /tmp/real-rootfs/bin/

# If not using static compilation, copy libraries
cp /bin/sh /tmp/real-rootfs/bin/
ldd real_programs/hello | grep "=>" | awk '{print $3}' | xargs -I {} cp {} /tmp/real-rootfs/lib/
# Repeat for other programs or just copy all system libs

# Copy dynamic linker
cp /lib64/ld-linux-x86-64.so.2 /tmp/real-rootfs/lib64/
```

### Step 3: Build Image and Run

```bash
# Build image
./mdock build realtest /tmp/real-rootfs

# Test 1: Hello program
./mdock run -e CUSTOM_MESSAGE="OS Project Demo" realtest
# Inside container: /bin/hello

# Test 2: Counter with environment variables
./mdock run -e COUNT=20 -e DELAY=1 realtest
# Inside: /bin/counter

# Test 3: Web server (long-running)
./mdock run -e SERVER_ENV=production realtest
# Inside: /bin/webserver
# In another terminal: mdock logs -f c1
# Stop: mdock stop c1

# Test 4: CPU stress with limit
./mdock run --cpu 10 realtest
# Inside: /bin/stress cpu 30
# Should be killed after 10 seconds of CPU time

# Test 5: Memory stress with limit
./mdock run --mem 128M realtest
# Inside: /bin/stress memory
# Should be limited to 128MB

# Test 6: Filesystem test
./mdock run realtest
# Inside: /bin/filetest
```

---

## Automated Testing with Real Programs

Modify `test_udock.sh` to use these programs:

```bash
# In the test script, after building rootfs:
cp real_programs/{hello,webserver,stress,counter,filetest} "$ROOTFS/bin/"

# Then run tests:
./test_udock.sh
```

---

## Example Test Session

```bash
# Session 1: Build and prepare
cd real_programs
make all
cd ..

# Create rootfs with real programs
mkdir -p /tmp/demo-rootfs/bin
cp real_programs/* /tmp/demo-rootfs/bin/
cp /bin/sh /tmp/demo-rootfs/bin/

# Build image
./mdock build demo /tmp/demo-rootfs

# Session 2: Test hello
./mdock run -e APP_ENV=demo demo
/bin/hello
exit

# Session 3: Test counter with logs
./mdock run -e COUNT=10 demo
/bin/counter &
exit
./mdock logs c1

# Session 4: Test web server
# Terminal 1:
./mdock run demo
/bin/webserver 3000

# Terminal 2:
./mdock logs -f c1
# Watch live logs

# Terminal 3:
./mdock ps
./mdock stop c1

# Session 5: Test resource limits
./mdock run --cpu 5 --mem 64M demo
/bin/stress cpu 20
# Should be killed after 5 CPU seconds

# Cleanup
./mdock ps
./mdock rm c1 c2 c3
./mdock rmi demo
```

---

## Educational Value

These real programs demonstrate:

1. **Process Management:**
   - PIDs, parent-child relationships
   - Signal handling (SIGTERM, SIGINT, SIGXCPU)
   - Process lifecycle

2. **Memory Management:**
   - Dynamic allocation (malloc/free)
   - Memory limits via setrlimit
   - Memory touching to force allocation

3. **CPU Scheduling:**
   - CPU-intensive operations
   - CPU time limits
   - Time slicing demonstration

4. **File I/O:**
   - File operations (open, read, write, close)
   - Directory listing
   - File metadata (stat)
   - Filesystem isolation via chroot

5. **Environment & Context:**
   - Environment variable inheritance
   - Working directory context
   - User/group IDs

Perfect for demonstrating OS concepts in your CSE 323 project! 🎓
