# Makefile for uDock Real Programs
# Compiles C programs to run inside containers

CC = gcc
CFLAGS = -Wall -O2
STATIC_FLAGS = -static

# Programs to build
PROGRAMS = hello webserver stress counter filetest

# Default target
all: $(PROGRAMS)

# Static builds (recommended for minimal containers)
static: CFLAGS += $(STATIC_FLAGS)
static: $(PROGRAMS)

# Individual program builds
hello: hello.c
	$(CC) $(CFLAGS) -o hello hello.c

webserver: webserver.c
	$(CC) $(CFLAGS) -o webserver webserver.c

stress: stress.c
	$(CC) $(CFLAGS) -o stress stress.c

counter: counter.c
	$(CC) $(CFLAGS) -o counter counter.c

filetest: filetest.c
	$(CC) $(CFLAGS) -o filetest filetest.c

# Clean
clean:
	rm -f $(PROGRAMS) *.o

# Install to a rootfs
install: $(PROGRAMS)
	@echo "Installing to $(ROOTFS)/bin/"
	@mkdir -p $(ROOTFS)/bin
	@cp $(PROGRAMS) $(ROOTFS)/bin/
	@echo "Programs installed successfully"

# Test compilation (compile but don't produce executable)
test:
	$(CC) $(CFLAGS) -fsyntax-only hello.c
	$(CC) $(CFLAGS) -fsyntax-only webserver.c
	$(CC) $(CFLAGS) -fsyntax-only stress.c
	$(CC) $(CFLAGS) -fsyntax-only counter.c
	$(CC) $(CFLAGS) -fsyntax-only filetest.c
	@echo "All programs passed syntax check"

.PHONY: all static clean install test
