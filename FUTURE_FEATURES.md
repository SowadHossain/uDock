# Future Feature Ideas for uDock

This document outlines potential features to add to uDock, organized by difficulty and usefulness.

## Quick Wins (1-2 hours each)

### 1. `mdock images` - List All Built Images
**Command:** `./mdock images`

**Use Case:** See all available images with metadata

**Implementation:**
- Read `~/.mdock/images.db`
- Parse and display in table format
- Calculate image size (du -sh on rootfs)
- Show creation timestamps

**Output Example:**
```
IMAGE          SIZE      CREATED           
ubuntu         245MB     2024-12-20 10:30
alpine         50MB      2024-12-19 15:22
myapp          120MB     2024-12-24 09:15
```

**Files to modify:**
- `src/image.c` - Add `cmd_images()`
- `src/main.c` - Add routing for "images" command

---

### 2. `mdock rm <container_id>` - Remove Container Record
**Command:** `./mdock rm c1`

**Use Case:** Clean up old/stopped containers from database

**Implementation:**
- Verify container exists and is stopped
- Remove line from `containers.db`
- Use atomic rename pattern for safety

**Files to modify:**
- `src/container.c` - Add `cmd_rm()`
- `src/main.c` - Add routing for "rm" command

---

### 3. `mdock rmi <image_name>` - Remove Image
**Command:** `./mdock rmi oldimage`

**Use Case:** Free disk space from unused images

**Implementation:**
- Check if any containers use this image (parse containers.db)
- Remove image rootfs directory: `~/.mdock/images/<name>/`
- Remove line from `images.db`
- Confirm before deletion

**Safety:** Refuse if containers exist using this image

**Files to modify:**
- `src/image.c` - Add `cmd_rmi()`, `image_in_use()`
- `src/main.c` - Add routing for "rmi" command

---

## Medium Features (4-8 hours each)

### 4. Environment Variables
**Command:** `./mdock run -e VAR=value -e DEBUG=1 myapp`

**Use Case:** Configure applications without rebuilding images

**Implementation:**
```c
// In cmd_run(), build environment array
char *envp[MAX_ENV];
int env_idx = 0;

// Add user-specified variables
envp[env_idx++] = "PATH=/bin:/usr/bin";
envp[env_idx++] = "HOME=/root";
envp[env_idx++] = "DATABASE_URL=...";  // From -e flag
envp[env_idx] = NULL;

// Pass to execve
execve("/bin/sh", argv, envp);
```

**Argument Parsing:**
- Parse `-e KEY=VALUE` flags
- Store in array
- Pass to child process

**Files to modify:**
- `src/container.c` - Modify `cmd_run()` for env parsing and execve call

---

### 5. Custom Commands (Not Just Shell)
**Command:** `./mdock run myapp -- python app.py --port 8000`

**Use Case:** Run specific programs instead of interactive shell

**Implementation:**
```c
// Parse: mdock run [OPTIONS] <image> -- <command> [args...]
// Find "--" separator
// Everything after "--" becomes exec arguments

char *exec_argv[MAX_ARGS];
exec_argv[0] = "/usr/bin/python";
exec_argv[1] = "app.py";
exec_argv[2] = "--port";
exec_argv[3] = "8000";
exec_argv[4] = NULL;

execve(exec_argv[0], exec_argv, envp);
```

**Files to modify:**
- `src/container.c` - Enhance `cmd_run()` argument parsing

---

### 6. Container Logs (Capture stdout/stderr)
**Command:** `./mdock logs c1` or `./mdock logs -f c1` (follow)

**Use Case:** Debug containers, see output after they exit

**Implementation:**
```c
// In child process before exec:
char log_path[256];
snprintf(log_path, sizeof(log_path), "%s/logs/%s.log", 
         mdock_home, container_id);

int log_fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);

// Redirect stdout and stderr
dup2(log_fd, STDOUT_FILENO);
dup2(log_fd, STDERR_FILENO);
close(log_fd);

execve(...);  // Now all output goes to log file
```

**cmd_logs Implementation:**
- Open `~/.mdock/logs/<container_id>.log`
- Print contents
- For `-f`: use `tail -f` behavior (loop + sleep)

**Files to modify:**
- `src/container.c` - Modify child process setup, add `cmd_logs()`
- Create `~/.mdock/logs/` directory

---

### 7. Working Directory Option
**Command:** `./mdock run -w /app myapp`

**Use Case:** Start container in specific directory

**Implementation:**
```c
// After chroot, before exec:
if (working_dir) {
    if (chdir(working_dir) != 0) {
        perror("chdir to working directory");
        exit(1);
    }
}
```

**Files to modify:**
- `src/container.c` - Add `-w` flag parsing, chdir call

---

## Advanced Features (16+ hours each)

### 8. Volume Mounts (Bind Mounts)
**Command:** `./mdock run -v /host/data:/container/data myapp`

**Use Case:** Share files between host and container, persist data

**Implementation:**
```c
// Before chroot:
// Parse mount specifications
// For each -v flag:
struct Mount {
    char host_path[PATH_MAX];
    char container_path[PATH_MAX];
    bool readonly;
};

// After chdir, before chroot:
for each mount {
    // Create mount point inside rootfs
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s%s", 
             rootfs_path, mount.container_path);
    mkdir_p(full_path);
    
    // Bind mount
    if (mount(mount.host_path, full_path, NULL, MS_BIND, NULL) != 0) {
        perror("mount");
        exit(1);
    }
    
    if (mount.readonly) {
        mount(NULL, full_path, NULL, MS_BIND | MS_REMOUNT | MS_RDONLY, NULL);
    }
}
```

**Challenges:**
- Requires root for mount()
- Cleanup: unmount when container stops
- Security: validate paths

**Files to modify:**
- `src/container.c` - Add mount parsing and system calls
- May need cleanup process for unmounting

---

### 9. Network Port Mapping
**Command:** `./mdock run -p 8080:80 webserver`

**Use Case:** Access container services from host

**Implementation:**
- Requires network namespaces (very advanced)
- Alternative: Use iptables/nftables for port forwarding
- Create virtual ethernet pair (veth)
- Set up NAT rules

**This is Docker-level complexity** - beyond scope of educational project

---

### 10. Container Stats (Real-time Resource Usage)
**Command:** `./mdock stats`

**Use Case:** Monitor CPU and memory usage

**Implementation:**
```c
// Read /proc/<pid>/stat for each running container
// Parse fields:
// - utime, stime (CPU usage)
// - vsize, rss (memory usage)

// Calculate percentages
// Display in updating table (like top)
```

**Output:**
```
ID    CPU%    MEM%     MEM USAGE      LIMIT
c1    5.2%    12.3%    245MB / 2GB    2GB
c2    0.1%    3.4%     67MB / 512MB   512MB
```

**Files to modify:**
- `src/container.c` - Add `cmd_stats()`, proc parsing

---

## Recommended Implementation Order

### Phase 1: Polish Current Features (M4 Complete ✓)
- Input validation ✓
- Resource limits ✓
- Documentation ✓

### Phase 2: Basic Management (Easy Wins)
1. `mdock images` - See what you have
2. `mdock rm` - Clean up containers  
3. `mdock rmi` - Clean up images

### Phase 3: Better UX (Medium)
4. Environment variables - Configure containers
5. Custom commands - Flexibility
6. Container logs - Debugging

### Phase 4: Advanced (If Time Permits)
7. Volume mounts - Data persistence
8. Stats monitoring - Resource tracking

---

## Educational Value per Feature

| Feature | OS Concepts Taught | Difficulty | Impact |
|---------|-------------------|------------|--------|
| `images` | File I/O, formatting | Easy | Medium |
| `rm` | Database operations | Easy | Medium |
| `rmi` | Filesystem operations | Easy | Medium |
| Env vars | Process environment | Medium | High |
| Custom commands | execve arguments | Medium | High |
| Logs | File descriptors, redirection | Medium | High |
| Working dir | chdir, process context | Easy | Low |
| Volumes | Mount syscalls, filesystem | Hard | High |
| Port mapping | Networking, namespaces | Very Hard | Medium |
| Stats | /proc filesystem, monitoring | Medium | Medium |

---

## Feature Complexity Matrix

```
Easy (1-2 hours):          Medium (4-8 hours):        Hard (16+ hours):
├─ mdock images            ├─ Environment vars        ├─ Volume mounts
├─ mdock rm                ├─ Custom commands         ├─ Network isolation
├─ mdock rmi               ├─ Container logs          ├─ User namespaces
└─ Working directory       ├─ Stats monitoring        └─ Multi-container orchestration
                           └─ Inspect command
```

---

## For Your Next Session

**Suggested First 3 Features to Add:**

1. **`mdock images`** - Takes 1 hour, immediately useful
2. **Environment variables** - Shows process environment mastery  
3. **Container logs** - Professional feature, great for demos

**Implementation Plan:**
```bash
# 1. Create feature branch
git checkout -b feature/images-env-logs

# 2. Implement mdock images (src/image.c)
# 3. Implement -e env vars (src/container.c)
# 4. Implement logs capture and mdock logs command
# 5. Test thoroughly
# 6. Update README and logs/
# 7. Commit and merge
```

Good luck! 🚀
