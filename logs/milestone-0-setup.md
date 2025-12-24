# Milestone M0: Project Setup & Skeleton

**Date:** November 2024  
**Branch:** `main`  
**Status:** ✅ Completed

---

## Development Log

### Challenge 1: Project Structure Design

**Situation:**
Starting a new C project from scratch required establishing a proper directory structure, build system, and modular architecture for a container runtime tool.

**Task:**
Set up the foundational project structure with Makefile, source organization, and basic CLI argument parsing to support future milestone implementations.

**Action:**
1. Created directory structure:
   - `src/` for source files
   - `include/` for header files
   - `Makefile` for build automation
2. Implemented `main.c` with basic command routing
3. Set up Git repository and initial commit
4. Created stub functions for future commands (build, run, ps, stop)
5. Configured compiler flags: `-Wall -Wextra -std=c11 -g`

**Result:**
- ✅ Clean modular structure established
- ✅ Makefile compiles successfully with no warnings
- ✅ CLI dispatcher routes commands correctly
- ✅ Foundation ready for M1 implementation
- ✅ Git repository initialized with proper .gitignore

---

## Key Decisions

### Architecture Choice
- **Decision:** Modular design with separate files for image, container, logging, and filesystem utilities
- **Rationale:** Maintainability and clear separation of concerns for educational project

### Build System
- **Decision:** Simple Makefile instead of CMake/autotools
- **Rationale:** Project size doesn't warrant complex build system; simplicity aids understanding

### Error Handling Strategy
- **Decision:** Return error codes from functions, print to stderr
- **Rationale:** Standard C practice, allows caller to decide how to handle errors

---

## Technical Notes

### Makefile Structure
```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -I./include
SOURCES = src/main.c src/image.c src/container.c src/fsutil.c src/log.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = mdock
```

### Command Routing Pattern
```c
if (strcmp(argv[1], "build") == 0) {
    return cmd_build(argc - 1, argv + 1);
} else if (strcmp(argv[1], "run") == 0) {
    return cmd_run(argc - 1, argv + 1);
}
// ...
```

---

## Lessons Learned

1. **Start Simple:** Initial skeleton with stub functions allowed iterative development
2. **Compiler Warnings:** Using `-Wall -Wextra` caught potential issues early
3. **Modular Design:** Separate files for distinct functionality made later work easier
4. **Git Early:** Starting with Git from M0 enabled proper version control throughout

---

## Files Created

- `src/main.c` - Entry point and command dispatcher
- `src/image.c` - Image management stubs
- `src/container.c` - Container operations stubs
- `src/fsutil.c` - Filesystem utilities stubs
- `src/log.c` - Logging utilities stubs
- `include/image.h` - Image function declarations
- `include/container.h` - Container function declarations
- `include/fsutil.h` - Filesystem utilities declarations
- `include/log.h` - Logging declarations
- `Makefile` - Build system
- `.gitignore` - Ignore build artifacts

---

## Next Steps

**Milestone M1:** Implement `mdock build` command to create images from rootfs directories
