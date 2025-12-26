# Sample Programs for uDock Containers

This directory contains sample programs to run inside uDock containers for testing and demonstration purposes.

---

## Available Programs

### 1. `hello.sh` - Simple Hello World

Basic program that prints container information and environment variables.

**Usage:**
```bash
# Copy to rootfs
cp sample_programs/hello.sh /tmp/test-rootfs/bin/

# Run in container
./mdock run myimg
# Inside container:
/bin/hello.sh
```

**With custom environment:**
```bash
./mdock run -e CUSTOM_MESSAGE="Welcome to OS class!" myimg
# Inside: /bin/hello.sh
```

---

### 2. `webserver.sh` - Simulated Web Server

Simulates a long-running web server that logs "requests" every 5 seconds.

**Usage:**
```bash
# Copy to rootfs
cp sample_programs/webserver.sh /tmp/test-rootfs/bin/

# Run as background service
./mdock run -e SERVER_ENV=production -e DEBUG=false myimg
# Inside: /bin/webserver.sh &

# In another terminal, follow logs:
./mdock logs -f c1

# Stop when done:
./mdock stop c1
```

**Perfect for testing:**
- Container logs (`mdock logs`)
- Log following (`mdock logs -f`)
- Background containers
- Container stop functionality

---

### 3. `counter.sh` - Counting Program

Counts from 1 to N with configurable delay. Great for testing log following.

**Usage:**
```bash
# Copy to rootfs
cp sample_programs/counter.sh /tmp/test-rootfs/bin/

# Count to 20 with 1 second delay
./mdock run -e COUNT=20 -e DELAY=1 myimg
# Inside: /bin/counter.sh

# Follow logs in real-time (from another terminal)
./mdock logs -f c1
```

**Perfect for testing:**
- Real-time log following
- Environment variable passing
- Time-based container operations

---

### 4. `stress_test.sh` - Resource Stress Test

Tests CPU and memory limits by running intensive operations.

**Usage:**
```bash
# Copy to rootfs
cp sample_programs/stress_test.sh /tmp/test-rootfs/bin/

# Test CPU limit (10 seconds max)
./mdock run --cpu 10 myimg
# Inside: /bin/stress_test.sh cpu

# Test memory limit
./mdock run --mem 128M myimg
# Inside: /bin/stress_test.sh memory
```

**Perfect for testing:**
- CPU time limits (`--cpu`)
- Memory limits (`--mem`)
- Resource enforcement via `setrlimit()`

---

## Quick Start: Prepare Rootfs with All Programs

```bash
# Create rootfs
mkdir -p /tmp/test-rootfs/bin

# Copy system binaries
cp /bin/sh /tmp/test-rootfs/bin/
cp /bin/ls /tmp/test-rootfs/bin/
cp /bin/echo /tmp/test-rootfs/bin/
cp /bin/sleep /tmp/test-rootfs/bin/

# Copy sample programs
cp sample_programs/*.sh /tmp/test-rootfs/bin/
chmod +x /tmp/test-rootfs/bin/*.sh

# Copy required libraries
mkdir -p /tmp/test-rootfs/{lib,lib64}
ldd /bin/sh | grep "=>" | awk '{print $3}' | xargs -I {} cp {} /tmp/test-rootfs/lib/
cp /lib64/ld-linux-x86-64.so.2 /tmp/test-rootfs/lib64/

# Build image
./mdock build demo /tmp/test-rootfs

# Test it!
./mdock run demo
# Inside: /bin/hello.sh
```

---

## Example Testing Workflow

### Test 1: Basic Container Functionality
```bash
./mdock run demo
# Inside: /bin/hello.sh
# Inside: exit
./mdock logs c1
```

### Test 2: Environment Variables
```bash
./mdock run -e COUNT=15 -e DELAY=2 demo
# Inside: /bin/counter.sh
# Inside: exit
```

### Test 3: Long-Running Container
```bash
# Terminal 1
./mdock run -e SERVER_ENV=production demo
# Inside: /bin/webserver.sh

# Terminal 2 (while server is running)
./mdock ps
./mdock logs -f c1  # Watch logs in real-time
# Press Ctrl+C to stop following

# Terminal 2
./mdock stop c1
./mdock ps
```

### Test 4: Resource Limits
```bash
# CPU limit (should be killed after 10 CPU seconds)
./mdock run --cpu 10 demo
# Inside: /bin/stress_test.sh cpu

# Memory limit
./mdock run --mem 128M demo
# Inside: /bin/stress_test.sh memory
```

### Test 5: Cleanup
```bash
./mdock ps              # List all containers
./mdock rm c1           # Remove stopped containers
./mdock images          # List images
./mdock rmi demo        # Remove image (after removing containers)
```

---

## Notes

- All programs are written in **POSIX shell** (`/bin/sh`) for maximum compatibility
- Programs are designed to be **simple and educational** - showing OS concepts clearly
- Each program demonstrates different aspects of container runtime:
  - Process isolation
  - Environment variable inheritance
  - I/O redirection and logging
  - Resource limits
  - Process lifecycle management

---

## Educational Value

These programs help demonstrate:

1. **Process Management:** How containers are just processes with isolation
2. **Environment Inheritance:** How child processes inherit environment
3. **I/O Streams:** How stdout/stderr are redirected to log files
4. **Resource Limits:** How OS enforces CPU and memory constraints
5. **Signal Handling:** How containers respond to SIGTERM/SIGKILL
6. **Filesystem Isolation:** How chroot provides separate filesystem view

Perfect for your CSE 323 Operating Systems project presentation! 🎓
