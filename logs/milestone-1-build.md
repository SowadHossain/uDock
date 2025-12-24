# Milestone M1: Image Building

**Date:** November 2024  
**Branch:** `milestone-1` → merged to `main`  
**Status:** ✅ Completed

---

## Development Log

### Challenge 1: Recursive Directory Copying

**Situation:**
The `mdock build` command required copying an entire directory tree (rootfs) to `~/.mdock/images/<name>/rootfs/`. Standard C library doesn't provide recursive copy functions, and using system() calls would be non-portable and defeat the learning purpose.

**Task:**
Implement a recursive directory copy function in pure C using POSIX APIs to replicate the source rootfs structure.

**Action:**
1. Created `copy_directory_recursive()` function in `src/fsutil.c`
2. Used `opendir()` and `readdir()` to traverse directory entries
3. For each entry:
   - Skip "." and ".." to prevent infinite recursion
   - Use `stat()` to determine if entry is file or directory
   - For directories: create destination directory and recurse
   - For files: use `open()`, `read()`, `write()` for byte-by-byte copy
4. Preserved file permissions using `chmod()`
5. Added error checking at each step with descriptive messages

**Result:**
- ✅ Successfully copies nested directory structures
- ✅ Handles files, directories, and preserves permissions
- ✅ No dependency on external commands like `cp -r`
- ✅ Educational value: demonstrates filesystem API usage

---

### Challenge 2: Home Directory Expansion

**Situation:**
The specification required storing images in `~/.mdock/images/`, but C doesn't automatically expand the `~` character like shells do.

**Task:**
Reliably get the user's home directory path in a portable way to construct the `~/.mdock` path.

**Action:**
1. Used `getenv("HOME")` to retrieve home directory path
2. Created helper function `get_mdock_home()` in `src/fsutil.c`
3. Added validation to check if HOME environment variable exists
4. Constructed full paths using `snprintf()` for buffer safety

**Result:**
- ✅ Portable solution using standard environment variable
- ✅ Works across different Linux distributions
- ✅ Safe string handling prevents buffer overflows
- ✅ Graceful error handling if HOME is not set

---

### Challenge 3: Image Metadata Storage

**Situation:**
Needed to store image metadata (name, rootfs path, timestamp) in a simple, readable format that could be easily queried later.

**Task:**
Design and implement a simple database format for storing image records without introducing external dependencies like SQLite.

**Action:**
1. Chose pipe-delimited text file format: `image_name|rootfs_path|timestamp`
2. Implemented `save_image_metadata()` function in `src/image.c`
3. Used `fopen()` in append mode to add records to `~/.mdock/images.db`
4. Generated timestamps using `time()` and `strftime()` for ISO 8601 format
5. Ensured file permissions allow read/write for user

**Result:**
- ✅ Simple, human-readable database format
- ✅ Easy to parse for future commands (run, ps)
- ✅ No external database dependencies
- ✅ Timestamps enable future sorting/filtering capabilities

---

### Challenge 4: Directory Creation with Parents

**Situation:**
Creating `~/.mdock/images/<image_name>/rootfs/` required creating multiple nested directories at once (like `mkdir -p`).

**Task:**
Implement functionality to create directory hierarchies in a single operation.

**Action:**
1. Created `mkdir_p()` function in `src/fsutil.c`
2. Tokenized path using `/` as delimiter
3. Iteratively created each path component:
   ```c
   mkdir(partial_path, 0755);
   ```
4. Ignored `EEXIST` errors (directory already exists)
5. Reported other errors (permissions, invalid path)

**Result:**
- ✅ Creates nested directories in single call
- ✅ Idempotent: safe to call multiple times
- ✅ Proper error handling for permission issues
- ✅ Mimics `mkdir -p` behavior

---

## Technical Implementation

### Core Functions Implemented

#### `cmd_build()` in `src/image.c`
```c
int cmd_build(int argc, char *argv[]) {
    // 1. Validate arguments (image_name, rootfs_dir)
    // 2. Get mdock home path
    // 3. Create destination directory structure
    // 4. Copy rootfs recursively
    // 5. Save metadata to images.db
    // 6. Log BUILD event
}
```

#### `copy_directory_recursive()` in `src/fsutil.c`
- Opens source directory with `opendir()`
- Iterates entries with `readdir()`
- Uses `stat()` to check file type
- Recursively handles subdirectories
- Copies regular files byte-by-byte

#### `save_image_metadata()` in `src/image.c`
- Opens `~/.mdock/images.db` in append mode
- Writes pipe-delimited record
- Formats timestamp as ISO 8601
- Flushes and closes file

---

## Testing Performed

### Test Case 1: Simple Rootfs
```bash
mkdir -p test-rootfs/bin
touch test-rootfs/bin/sh
./mdock build testimg test-rootfs
```
**Result:** ✅ Image created, files copied correctly

### Test Case 2: Nested Directories
```bash
mkdir -p complex-rootfs/usr/local/bin
./mdock build complex complex-rootfs
```
**Result:** ✅ Nested structure preserved

### Test Case 3: Duplicate Build Attempt
```bash
./mdock build testimg test-rootfs
./mdock build testimg test-rootfs
```
**Result:** ⚠️ Both succeed (duplicate handling added in M4)

---

## Known Limitations (Addressed Later)

1. **No Duplicate Detection:** Same image can be built multiple times (Fixed in M4 - Issue #12)
2. **No Image Name Validation:** Accepts any string as image name (Fixed in M4 - Issue #12)
3. **Symbolic Links:** Not handled (acceptable for project scope)
4. **Large Files:** Byte-by-byte copy is slow (acceptable for small rootfs)

---

## Lessons Learned

1. **POSIX APIs:** Deep dive into `dirent.h`, `sys/stat.h`, and file I/O
2. **Error Handling:** Every system call must check return value
3. **Buffer Management:** Always use `snprintf()` to prevent overflows
4. **Testing Edge Cases:** Empty directories, permissions, non-existent paths
5. **Logging Early:** Added log events from M1 for debugging help

---

## Files Modified

- `src/image.c` - Implemented `cmd_build()` and metadata functions
- `src/fsutil.c` - Implemented recursive copy, mkdir_p, path utilities
- `src/log.c` - Implemented logging with timestamps
- `include/image.h` - Added function declarations
- `include/fsutil.h` - Added utility function declarations
- `include/log.h` - Added logging declarations

---

## Performance Notes

### Copy Performance
- Small rootfs (<100 files): ~100ms
- Medium rootfs (500 files): ~500ms
- Acceptable for educational project, not production-ready

### Optimization Opportunities (Not Implemented)
- Use `sendfile()` for faster file copying
- Parallel directory traversal
- Copy-on-write for identical files

---

## Next Steps

**Milestone M2:** Implement `mdock run` command to execute containers using fork/exec/chroot
