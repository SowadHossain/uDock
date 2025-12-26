#!/bin/bash
#
# Quick Setup Script for uDock with Real Programs
# Compiles programs and creates a ready-to-use container image
#

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

# Check if running on Linux
if [[ ! "$OSTYPE" == "linux-gnu"* ]]; then
    print_error "This script must be run on Linux"
    exit 1
fi

print_header "uDock Quick Setup with Real Programs"

# Step 1: Build uDock
print_info "Step 1: Building uDock..."
if make clean && make; then
    print_success "uDock compiled successfully"
else
    print_error "Failed to compile uDock"
    exit 1
fi

# Step 2: Compile real programs
print_info "Step 2: Compiling real C programs..."
cd real_programs

if make clean && make static; then
    print_success "All programs compiled (static binaries)"
else
    print_error "Failed to compile programs"
    exit 1
fi

cd ..

# Step 3: Create rootfs
print_info "Step 3: Creating root filesystem..."

ROOTFS="/tmp/udock-demo-rootfs"
rm -rf "$ROOTFS"

mkdir -p "$ROOTFS"/{bin,lib,lib64,usr/bin,tmp,home,root,dev,proc,sys,etc}

print_success "Directory structure created"

# Step 4: Copy system binaries
print_info "Step 4: Copying system binaries..."

for binary in /bin/sh /bin/bash /bin/ls /bin/cat /bin/echo /bin/sleep /bin/ps; do
    if [ -f "$binary" ]; then
        cp "$binary" "$ROOTFS/bin/" 2>/dev/null || true
    fi
done

print_success "System binaries copied"

# Step 5: Copy our compiled programs
print_info "Step 5: Installing real programs..."

cp real_programs/{hello,webserver,stress,counter,filetest} "$ROOTFS/bin/"
chmod +x "$ROOTFS/bin"/*

print_success "Real programs installed to rootfs"

# Step 6: Copy libraries (only if not static)
print_info "Step 6: Copying libraries..."

# Check if binaries are static
if ldd real_programs/hello 2>&1 | grep -q "not a dynamic"; then
    print_info "Programs are statically linked - no libraries needed"
else
    # Copy libraries
    for prog in /bin/sh real_programs/hello; do
        if [ -f "$prog" ]; then
            ldd "$prog" 2>/dev/null | grep "=>" | awk '{print $3}' | while read lib; do
                if [ -f "$lib" ]; then
                    cp "$lib" "$ROOTFS/lib/" 2>/dev/null || true
                fi
            done
        fi
    done
    
    # Copy dynamic linker
    if [ -f /lib64/ld-linux-x86-64.so.2 ]; then
        cp /lib64/ld-linux-x86-64.so.2 "$ROOTFS/lib64/"
    elif [ -f /lib/ld-linux.so.2 ]; then
        cp /lib/ld-linux.so.2 "$ROOTFS/lib/"
    fi
    
    print_success "Libraries copied"
fi

# Step 7: Create /etc/passwd and /etc/group (for some programs)
print_info "Step 7: Creating basic system files..."

cat > "$ROOTFS/etc/passwd" << 'EOF'
root:x:0:0:root:/root:/bin/sh
nobody:x:65534:65534:nobody:/:/bin/false
EOF

cat > "$ROOTFS/etc/group" << 'EOF'
root:x:0:
nobody:x:65534:
EOF

print_success "System files created"

# Step 8: Build container image
print_info "Step 8: Building container image 'demo'..."

if ./mdock build demo "$ROOTFS"; then
    print_success "Container image 'demo' built successfully"
else
    print_error "Failed to build container image"
    exit 1
fi

# Display summary
print_header "Setup Complete!"

echo "Container image 'demo' is ready with the following programs:"
echo ""
echo "  📦 System Binaries:"
ls -lh "$ROOTFS/bin/" | grep -E "sh|ls|cat|echo|sleep" | awk '{print "     - " $9}'
echo ""
echo "  🚀 Real C Programs:"
ls -lh "$ROOTFS/bin/" | grep -E "hello|webserver|stress|counter|filetest" | awk '{print "     - " $9 " (" $5 ")"}'
echo ""

print_header "Quick Start Commands"

cat << 'EOF'
# Test 1: Hello World
./mdock run -e CUSTOM_MESSAGE="Hello OS!" demo
# Inside: /bin/hello

# Test 2: Counter (with log following)
# Terminal 1:
./mdock run -e COUNT=20 -e DELAY=1 demo
# Inside: /bin/counter

# Terminal 2:
./mdock logs -f c1

# Test 3: Web Server
# Terminal 1:
./mdock run -e SERVER_ENV=production demo
# Inside: /bin/webserver

# Terminal 2:
./mdock logs -f c1

# Terminal 3:
./mdock ps
./mdock stop c1

# Test 4: CPU Stress (with limits)
./mdock run --cpu 10 demo
# Inside: /bin/stress cpu 30
# Will be killed after 10 CPU seconds

# Test 5: Memory Stress (with limits)
./mdock run --mem 128M demo
# Inside: /bin/stress memory

# Test 6: Filesystem Test
./mdock run demo
# Inside: /bin/filetest

# View all images
./mdock images

# View all containers
./mdock ps

# Cleanup
./mdock rm c1 c2 c3
./mdock rmi demo
EOF

print_header "Ready to Test!"
print_info "Run: ./mdock run demo"
print_info "Then try: /bin/hello"
