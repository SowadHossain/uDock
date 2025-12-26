<p align="center"># uDock – Minimal Container Runtime (CSE 323 OS Project)

  <img src="https://img.shields.io/badge/Language-C-blue?style=for-the-badge&logo=c" alt="C Language">

  <img src="https://img.shields.io/badge/Platform-Linux-orange?style=for-the-badge&logo=linux" alt="Linux">**Student Name:** Sowad Hossain

  <img src="https://img.shields.io/badge/Course-CSE%20323-green?style=for-the-badge" alt="CSE 323">

  <img src="https://img.shields.io/badge/Status-Complete-success?style=for-the-badge" alt="Complete">**ID:** 2323614042 

</p>

**Course:** CSE 323 – Operating Systems Design 

<h1 align="center">🐳 uDock</h1>

<h3 align="center">A Minimal Docker-like Container Runtime in User Space</h3>**Project Title:** uDock – Minimal Docker-like Container Runtime in User Space 



<p align="center">**Language / Platform:** C on Linux

  <strong>CSE 323 – Operating Systems Design Project</strong><br>

  Building containers from scratch to understand OS fundamentals---

</p>

## 1. Project Overview

---

uDock is a **minimal Docker-like container runtime** implemented as a user-space system tool in C on Linux.

## 📋 Project Information

It allows the user to:

| Field | Details |

|-------|---------|* **Build images** from directory trees (a simple root filesystem).

| **Student Name** | Sowad Hossain |* **Run containers** as child processes in an isolated root filesystem.

| **Student ID** | 2323614042 |* **List containers** and their status.

| **Course** | CSE 323 – Operating Systems Design |* **Stop containers** using signals.

| **Project Title** | uDock – Minimal Docker-like Container Runtime |* **Log** all major actions (build, run, exit, stop) for later inspection.

| **Language** | C (POSIX APIs) |

| **Platform** | Linux |The goal is **not** to re-implement Docker, but to design a small system that uses real operating system mechanisms to demonstrate core OS concepts from CSE 323: process management, storage management, asynchronism and signals, basic resource handling, and contemporary OS ideas such as containers.



------



## 🎬 Demo Video## 2. Features



<!-- uDock currently supports the following commands:

╔═══════════════════════════════════════════════════════════════════════════════╗

║                         HOW TO ADD YOUR VIDEO                                  ║### 2.1 `mdock build <image_name> <rootfs_dir>`

╠═══════════════════════════════════════════════════════════════════════════════╣

║                                                                               ║* Creates a new image with the name `<image_name>`.

║  1. Upload your video to YouTube                                              ║

║                                                                               ║* Recursively copies `<rootfs_dir>` into:

║  2. Get your Video ID from the URL:                                           ║

║     Example URL: https://www.youtube.com/watch?v=dQw4w9WgXcQ                  ║  ```text

║     Video ID: dQw4w9WgXcQ (the part after v=)                                 ║  ~/.mdock/images/<image_name>/rootfs/

║                                                                               ║  ```

║  3. Replace YOUR_VIDEO_ID below with your actual Video ID                     ║

║                                                                               ║* Appends a record to `~/.mdock/images.db` with:

║  Note: GitHub README doesn't support iframes, but the thumbnail               ║

║  image with a link works perfectly and looks professional!                    ║  * image name

║                                                                               ║  * rootfs path

╚═══════════════════════════════════════════════════════════════════════════════╝  * creation timestamp

-->

### 2.2 `mdock run [OPTIONS] <image_name>`

<p align="center">

  <a href="https://www.youtube.com/watch?v=YOUR_VIDEO_ID">* Looks up `<image_name>` in `images.db` to find the rootfs path.

    <img src="https://img.youtube.com/vi/YOUR_VIDEO_ID/maxresdefault.jpg" alt="uDock Demo Video" width="600">* Generates a unique container ID (e.g. `c1`, `c2`, …).

  </a>* `fork()`s a child process:

</p>

  * In the child:

<p align="center">

  <strong>▶️ Click the image above to watch the full demonstration</strong>    * `chdir()`s into the rootfs directory.

</p>    * Optionally `chroot()`s to that directory (if permitted).

    * `execve()`s a shell (e.g. `/bin/sh`) inside the container.

---  * In the parent:



## 📖 Table of Contents    * Records the container in `~/.mdock/containers.db` with:



- [Overview](#-overview)      * container ID

- [Features](#-features)      * PID

- [Architecture](#-architecture)      * image name

- [Prerequisites](#-prerequisites)      * start time

- [Installation](#-installation)      * status (`running` / `exited`)

- [Usage](#-usage)    * Waits for the child and updates its status and exit code.

- [Command Reference](#-command-reference)

- [OS Concepts Demonstrated](#-os-concepts-demonstrated)### 2.3 `mdock ps`

- [Project Structure](#-project-structure)

- [Testing](#-testing)* Reads `containers.db` and prints a list of containers.

- [Development Logs](#-development-logs)* For each container:

- [References](#-references)

  * Checks if the PID is still alive (`kill(pid, 0)` or `/proc/<pid>`).

---  * Computes uptime from start and end times.

* Output format (example):

## 🎯 Overview

  ```text

**uDock** is a minimal container runtime implemented entirely in C, designed to demonstrate core operating system concepts through practical implementation. Unlike production container runtimes, uDock focuses on educational clarity while maintaining functional correctness.  ID    PID    IMAGE     STATUS     UPTIME

  c1    3214   myimg     running    00:01:23

### What is a Container?  c2    3250   base      exited     00:00:05

  ```

A container is an isolated execution environment that provides:

- **Filesystem Isolation**: Separate root filesystem view via `chroot()`### 2.4 `mdock stop <container_id>`

- **Process Isolation**: Independent process with controlled environment

- **Resource Control**: CPU and memory limits via `setrlimit()`* Looks up the container ID in `containers.db`.

* If the container is running:

uDock implements these concepts using fundamental Linux system calls, providing hands-on understanding of how containers work at the OS level.

  * Sends `SIGTERM` to the PID.

### Project Goals  * Optionally falls back to `SIGKILL` if the process does not exit in time.

* Updates the container status to `stopped` / `killed`.

| Goal | Description |

|------|-------------|### 2.5 `mdock images`

| **Educational** | Understand OS concepts through implementation |

| **Practical** | Build a working container runtime from scratch |* Lists all built images with detailed metadata.

| **Complete** | Cover process management, filesystem, signals, and resource control |* Displays a formatted table showing:

| **Documented** | Maintain comprehensive development logs using STAR method |  * Image name

  * Size (calculated using `du`)

---  * Creation timestamp

* Example output:

## ✨ Features  ```text

  IMAGE      SIZE       CREATED

### Core Container Operations  myimg      12.5M      2025-12-24 10:30:15

  base       8.3M       2025-12-24 09:15:42

| Feature | Command | Description |  ```

|---------|---------|-------------|

| **Build Images** | `mdock build` | Create container images from root filesystems |### 2.6 `mdock rmi <image_name>`

| **Run Containers** | `mdock run` | Execute isolated processes in containers |

| **List Containers** | `mdock ps` | Display all containers with status and uptime |* Removes an image and its associated rootfs directory.

| **Stop Containers** | `mdock stop` | Gracefully terminate running containers |* Safety checks:

| **View Logs** | `mdock logs` | Access container stdout/stderr output |  * Verifies the image exists in `images.db`.

| **List Images** | `mdock images` | Display all available images with metadata |  * Checks if any containers (running or stopped) are using this image.

| **Remove Containers** | `mdock rm` | Delete stopped containers from system |  * Prevents deletion if the image is in use.

| **Remove Images** | `mdock rmi` | Delete unused images safely |* Removes the image record from `images.db` and deletes the rootfs directory.



### Advanced Capabilities### 2.7 `mdock rm <container_id>`



| Feature | Option | Description |* Removes a stopped container from the system.

|---------|--------|-------------|* Validation:

| **Environment Variables** | `-e KEY=VALUE` | Pass custom environment to containers |  * Ensures the container exists in `containers.db`.

| **Memory Limits** | `--mem SIZE` | Restrict container memory (e.g., `128M`, `1G`) |  * Verifies the container is not currently running.

| **CPU Time Limits** | `--cpu SECONDS` | Limit total CPU time for containers |* Removes the container record from `containers.db`.

| **Log Following** | `logs -f` | Real-time log streaming (like `tail -f`) |

| **Graceful Shutdown** | `stop` | SIGTERM with 5-second timeout, fallback to SIGKILL |### 2.8 Environment Variables in `mdock run`



### System Features* Supports passing custom environment variables to containers using `-e` flags.

* Syntax:

- ✅ **Persistent Logging**: All operations logged to `~/.mdock/log.txt`  ```bash

- ✅ **Structured Databases**: Container and image metadata in pipe-delimited files  ./mdock run -e KEY1=VALUE1 -e KEY2=VALUE2 myimg

- ✅ **Comprehensive Error Handling**: Validation and informative error messages  ```

- ✅ **Signal Management**: Proper handling of container lifecycle signals* Default environment variables provided:

- ✅ **Safety Checks**: Prevents deletion of in-use images and running containers  * `PATH=/usr/local/bin:/usr/bin:/bin`

  * `HOME=/root`

---  * `TERM=linux`

* Custom variables override or extend the defaults.

## 🏗️ Architecture

### 2.9 Container Logs with `mdock logs`

### System Components

* Captures container stdout and stderr to persistent log files.

```* Log files stored at: `~/.mdock/logs/<container_id>.log`

┌─────────────────────────────────────────────────────────────────┐* View logs:

│                        uDock CLI (mdock)                        │  ```bash

├─────────────────────────────────────────────────────────────────┤  ./mdock logs <container_id>

│                                                                 │  ```

│  ┌──────────────┐  ┌───────────────┐  ┌───────────────────────┐│* Follow logs in real-time (like `tail -f`):

│  │    Image     │  │   Container   │  │      Utilities        ││  ```bash

│  │   Manager    │  │    Manager    │  │                       ││  ./mdock logs -f <container_id>

│  │              │  │               │  │                       ││  ```

│  │ • build      │  │ • run         │  │ • fsutil (copy_dir)   ││* In follow mode, automatically exits when the container stops.

│  │ • images     │  │ • ps          │  │ • log (event logging) ││

│  │ • rmi        │  │ • stop        │  │ • timeutil (uptime)   ││### 2.10 Logging and Error Handling

│  │              │  │ • rm          │  │                       ││

│  │              │  │ • logs        │  │                       ││* All major events are logged to `~/.mdock/log.txt`, for example:

│  └──────────────┘  └───────────────┘  └───────────────────────┘│

│                                                                 │  ```text

├─────────────────────────────────────────────────────────────────┤  [2025-11-22T12:01:09] BUILD image=myimg src=./rootfs

│                     Linux System Calls                          │  [2025-11-22T12:02:15] RUN   id=c1 image=myimg pid=3214 mem=128M cpu=10s

│  fork() │ execve() │ chroot() │ setrlimit() │ kill() │ dup2()  │  [2025-11-22T12:03:10] EXIT  id=c1 status=0

│  waitpid() │ open() │ chdir() │ signal() │ opendir() │ stat()  │  [2025-11-22T12:04:00] STOP  id=c1 signal=SIGTERM

└─────────────────────────────────────────────────────────────────┘  ```

```

---

### Data Storage Layout

## 3. How to Build and Run

```

~/.mdock/### 3.1 Requirements

├── images.db              # Image registry (name|path|timestamp)

├── containers.db          # Container records (id|pid|image|start|status)* Linux (with `/proc` filesystem)

├── log.txt                # Global event log* GCC or any C compiler supporting POSIX APIs

├── images/* Standard C library and POSIX headers

│   └── <image_name>/

│       └── rootfs/        # Container root filesystem### 3.2 Build

└── logs/

    └── <container_id>.log # Per-container stdout/stderrFrom the project root directory:

```

```bash

---make

```

## 📦 Prerequisites

This should compile the sources into a single executable:

### System Requirements

```text

| Requirement | Details |./mdock

|-------------|---------|```

| **Operating System** | Linux (tested on Ubuntu 20.04+, Debian 11+) |

| **Compiler** | GCC with C99 support |### 3.3 Usage Examples

| **Build Tools** | GNU Make |

| **Privileges** | Root access required for `chroot()` operations |#### Basic Workflow



### Installing Dependencies1. **Prepare a root filesystem directory**, e.g.:



```bash   ```bash

# Ubuntu/Debian   mkdir -p ./rootfs/bin

sudo apt update   # Copy a shell and basic tools into ./rootfs/bin as needed

sudo apt install build-essential gcc make   ```



# Fedora/RHEL2. **Build an image:**

sudo dnf install gcc make

   ```bash

# Arch Linux   ./mdock build myimg ./rootfs

sudo pacman -S base-devel   ```

```

3. **Run a container:**

---

   ```bash

## 🚀 Installation   ./mdock run myimg

   ```

### Step 1: Clone the Repository

   This should drop you into a shell running with the image’s rootfs.

```bash

git clone https://github.com/SowadHossain/uDock.git4. **List containers:**

cd uDock

```   ```bash

   ./mdock ps

### Step 2: Compile the Project   ```



```bash5. **Stop a container:**

make clean

make   ```bash

```   ./mdock stop c1

   ```

**Expected Output:**

```   (Assuming `c1` is a valid container ID.)

gcc -Wall -Wextra -Iinclude -c src/main.c -o src/main.o

gcc -Wall -Wextra -Iinclude -c src/image.c -o src/image.o6. **List all images:**

gcc -Wall -Wextra -Iinclude -c src/container.c -o src/container.o

gcc -Wall -Wextra -Iinclude -c src/fsutil.c -o src/fsutil.o   ```bash

gcc -Wall -Wextra -Iinclude -c src/log.c -o src/log.o   ./mdock images

gcc -Wall -Wextra -Iinclude -c src/timeutil.c -o src/timeutil.o   ```

gcc -o mdock src/main.o src/image.o src/container.o src/fsutil.o src/log.o src/timeutil.o

```7. **Run a container with environment variables:**



### Step 3: Verify Installation   ```bash

   ./mdock run -e DATABASE_URL=postgres://localhost -e DEBUG=true myimg

```bash   ```

./mdock help

```8. **View container logs:**



### Quick Start (Optional)   ```bash

   ./mdock logs c1

For immediate testing with pre-compiled programs:   ```



```bash   Or follow logs in real-time:

chmod +x setup_demo.sh

sudo ./setup_demo.sh   ```bash

```   ./mdock logs -f c1

   ```

This creates a ready-to-use `demo` image with test programs.

9. **Remove a stopped container:**

---

   ```bash

## 📘 Usage   ./mdock rm c1

   ```

### Basic Workflow

10. **Remove an unused image:**

#### 1. Prepare a Root Filesystem

    ```bash

```bash    ./mdock rmi myimg

# Create minimal rootfs    ```

mkdir -p /tmp/myroot/{bin,lib,lib64,tmp}

---

# Copy essential binaries

cp /bin/sh /bin/ls /bin/echo /bin/cat /tmp/myroot/bin/## 4. Design & Architecture



# Copy required libraries### 4.1 Directory Layout

ldd /bin/sh | grep "=>" | awk '{print $3}' | xargs -I {} cp {} /tmp/myroot/lib/

cp /lib64/ld-linux-x86-64.so.2 /tmp/myroot/lib64/* uDock home:

```

  ```text

#### 2. Build an Image  ~/.mdock/

  ```

```bash

./mdock build myimage /tmp/myroot* Images:

```

  ```text

#### 3. Run a Container  ~/.mdock/images/<image_name>/rootfs/

  ```

```bash

sudo ./mdock run myimage* Containers metadata (single file):

```

  ```text

**Inside the container:**  ~/.mdock/containers.db

```bash  ```

/ # ls /

bin  lib  lib64  tmp* Images metadata:



/ # echo "Hello from container!"  ```text

Hello from container!  ~/.mdock/images.db

  ```

/ # exit

```* Logs:



#### 4. Monitor and Manage  ```text

  ~/.mdock/log.txt

```bash  ```

# List containers

./mdock ps### 4.2 Main Components (Code-Level)



# View logs* `main.c`

./mdock logs c1  Parses command-line arguments and dispatches to subcommands (`build`, `run`, `ps`, `stop`).



# Stop if running* `image.c`

./mdock stop c1

  * Implements `build` functionality.

# Cleanup  * Handles recursive directory copy.

./mdock rm c1  * Manages `images.db`.

./mdock rmi myimage

```* `container.c`



---  * Implements `run`, `ps`, and `stop`.

  * Calls `fork`, `execve`, `chdir`, `chroot`.

## 📚 Command Reference  * Manages `containers.db` and container status.



### `mdock build <name> <rootfs_dir>`* `fsutil.c`



Creates a container image from a directory.  * Utility for directory copying (`copy_dir`), path handling.



```bash* `log.c`

./mdock build alpine /path/to/alpine-rootfs

```  * Appends formatted events to `log.txt`.



---* `timeutil.c`



### `mdock run [OPTIONS] <image>`  * Handles timestamps and uptime calculation.



Starts a new container from an image.This modular structure mirrors a simple OS-style decomposition: image manager, container/process manager, filesystem utilities, and logging subsystem.



| Option | Example | Description |---

|--------|---------|-------------|

| `-e` | `-e DEBUG=true` | Set environment variable |## 5. OS Concepts Covered (Mapping to CSE 323)

| `--mem` | `--mem 256M` | Memory limit (K, M, G) |

| `--cpu` | `--cpu 30` | CPU time limit (seconds) |This project demonstrates several key topics from the CSE 323 course outline:



**Examples:**### 5.1 Operating Systems Structure

```bash

sudo ./mdock run myimage* Clear separation of concerns:

sudo ./mdock run -e APP_ENV=prod -e DEBUG=false myimage

sudo ./mdock run --mem 128M --cpu 60 myimage  * Image management (`build`, `images.db`)

```  * Container lifecycle (`run`, `ps`, `stop`, `containers.db`)

  * Logging (`log.txt`)

---* Simple but explicit architecture similar to OS subsystems.



### `mdock ps`### 5.2 Process State Transition & Process Management



Lists all containers with status.* Uses `fork` to create container processes and `execve` to run a shell/program.

* Uses `waitpid` to observe process termination and exit status.

```* Tracks container PIDs and statuses (`running`, `exited`, `stopped`) in `containers.db`.

ID       PID      IMAGE        STATUS     UPTIME* `mdock ps` shows which containers are still running by checking the underlying process state.

c1       12345    myimage      running    00:05:23

c2       12400    alpine       exited     00:02:15### 5.3 Asynchronism, Signals, Interrupts (Conceptually)

```

* `mdock stop` uses `kill` to send signals (`SIGTERM`, `SIGKILL`) to container processes.

---* This models **asynchronous** control: the parent process (uDock) can asynchronously interrupt or terminate a child process.

* Conceptually related to software interrupts and signal handling.

### `mdock stop <id>`

### 5.4 Storage Management (Real and Auxiliary)

Gracefully stops a container (SIGTERM → SIGKILL fallback).

* Real storage:

```bash

./mdock stop c1  * Images: on-disk directory trees under `~/.mdock/images/`.

```  * Containers: associated metadata and working directories.

* Auxiliary storage:

---

  * Metadata files: `images.db` and `containers.db`.

### `mdock logs [-f] <id>`  * Logs: `log.txt` for build/run/stop/exit events.

* Demonstrates how an OS-style tool maintains persistent state and logs.

Views container output. Use `-f` to follow in real-time.

### 5.5 Memory / Resource Management (Optional Extension)

```bash

./mdock logs c1      # View all logsIf resource limits are implemented (via `setrlimit(RLIMIT_AS, RLIMIT_CPU)`):

./mdock logs -f c1   # Follow logs (like tail -f)

```* Each container can have a cap on:



---  * Maximum address space (virtual memory).

  * Maximum CPU time.

### `mdock images`* Demonstrates basic memory and CPU resource management at the process level.



Lists all available images.### 5.6 Processor Scheduling & Multi-Processing (Observed)



```* Multiple containers are simply multiple processes managed by the OS scheduler.

IMAGE        SIZE       CREATED* Running several containers concurrently demonstrates:

myimage      15.2M      2025-12-26 10:30:45

alpine       5.6M       2025-12-26 09:15:20  * how the scheduler interleaves process execution,

```  * how process creation and termination look from a user-space tool’s perspective.



---### 5.7 Network & Security (Conceptual)



### `mdock rm <id>`* Using `chdir` and optionally `chroot` simulates basic filesystem isolation.

* Conceptually related to how real containers and modern OSes (e.g. Android, Docker) isolate applications with separate filesystem views and permissions.

Removes a stopped container.* In the report/presentation, this is connected to application sandboxing and process isolation in contemporary systems.



```bash---

./mdock rm c1

```## 6. STAR – Challenges and How They Were Addressed



---The following subsections describe some key challenges in **STAR format**, as requested.



### `mdock rmi <image>`---



Removes an unused image.### 6.1 Challenge 1 – Implementing `run` with Correct Process Isolation



```bash**Situation:**

./mdock rmi myimageInitially, `mdock run` needed to launch a new program "inside" an image’s filesystem, but the process was still running in the host’s normal directory tree, and it wasn’t clear how to isolate it properly.

```

**Task:**

---Design and implement a way to start a new process that uses the image’s root filesystem as its working environment, without breaking the host system.



## 🎓 OS Concepts Demonstrated**Action:**



| Concept | System Calls | Application |* Used `fork()` to create a child process from uDock.

|---------|--------------|-------------|* In the child:

| **Process Creation** | `fork()` | Creating container processes |

| **Program Execution** | `execve()` | Running shell in container |  * Called `chdir(rootfs_path)` to switch into the image’s rootfs directory.

| **Process Synchronization** | `waitpid()` | Parent waits for container exit |  * Optionally called `chroot(rootfs_path)` (when permissions allowed) so that the process would see this directory as `/`.

| **Filesystem Isolation** | `chroot()`, `chdir()` | Container root filesystem |  * Executed a shell inside that environment using `execve("/bin/sh", ...)`.

| **Signal Handling** | `kill()`, `signal()` | Container stop (SIGTERM/SIGKILL) |* In the parent:

| **Resource Limits** | `setrlimit()` | Memory and CPU constraints |

| **I/O Redirection** | `dup2()`, `open()` | Container log capture |  * Used `waitpid` to wait for the child’s termination and record its exit status.

| **File Operations** | `opendir()`, `stat()` | Recursive directory copy |

**Result:**

---

* `mdock run <image_name>` successfully starts a shell that operates relative to the image’s rootfs, imitating a container.

## 📂 Project Structure* The host system remains unaffected, and the design cleanly separates parent/child responsibilities.

* This demonstrates practical use of process creation, `chdir`, `chroot`, and `execve`.

```

uDock/---

├── Makefile                    # Build configuration

├── README.md                   # Project documentation### 6.2 Challenge 2 – Keeping Consistent Container Metadata

├── HOW-TO-TEST.md              # Comprehensive testing guide

│**Situation:**

├── include/                    # Header filesThe `containers.db` file needed to track IDs, PIDs, statuses, and timestamps for all containers. Early versions often produced inconsistent data (e.g. status not updated, stale PIDs) if a run crashed or the program exited unexpectedly.

│   ├── container.h

│   ├── fsutil.h**Task:**

│   ├── image.hEnsure that container metadata is written and updated consistently, so that `mdock ps` shows correct information even after multiple runs and failures.

│   ├── log.h

│   └── timeutil.h**Action:**

│

├── src/                        # Source files* Defined a simple, line-based textual format for `containers.db`, with key–value pairs per container.

│   ├── main.c                  # CLI entry point* Centralized metadata updates in helper functions:

│   ├── container.c             # Container operations

│   ├── image.c                 # Image management  * `add_container_record(...)` for new containers.

│   ├── fsutil.c                # Filesystem utilities  * `update_container_status(...)` for status changes.

│   ├── log.c                   # Event logging* Ensured that metadata is written after key events:

│   └── timeutil.c              # Time utilities

│  * Immediately after `run` starts a container (status=`running`).

├── real_programs/              # Compiled C test programs  * After the child process exits (status=`exited`, with exit code).

│   ├── hello.c, webserver.c, stress.c, counter.c, filetest.c  * After `stop` sends signals and confirms the container is no longer running.

│   └── Makefile* Added checks in `mdock ps` to verify whether a PID is still alive and to correct the status if necessary.

│

├── sample_programs/            # Shell script alternatives**Result:**

│

├── logs/                       # Development documentation (STAR method)* `containers.db` remains simple and robust enough for the project.

│   ├── milestone-0-setup.md* `mdock ps` reliably displays current statuses and uptimes.

│   ├── milestone-1-build.md* This improved the system’s reliability and illustrated the importance of careful auxiliary storage management in OS-related tools.

│   ├── milestone-2-run.md

│   ├── milestone-3-management.md---

│   ├── milestone-4-polish.md

│   └── milestone-5-features.md### 6.3 Challenge 3 – Recursive Directory Copy for `build`

│

├── test_udock.sh               # Automated test suite**Situation:**

└── setup_demo.sh               # Quick demo setup`mdock build` needed to copy an entire directory tree (`rootfs_dir`) into the internal images folder. A naive copy risked missing subdirectories or mishandling file permissions.

```

**Task:**

---Implement a reliable, recursive directory copy that can reproduce the `rootfs_dir` structure under `~/.mdock/images/<image_name>/rootfs/`.



## 🧪 Testing**Action:**



See **[HOW-TO-TEST.md](HOW-TO-TEST.md)** for comprehensive testing instructions.* Wrote a `copy_dir(src, dst)` function that:



### Quick Test  * Uses `opendir` / `readdir` to iterate over directory entries.

  * Recursively calls itself when encountering subdirectories.

```bash  * Uses `open`, `read`, `write`, and `close` to copy regular files.

chmod +x test_udock.sh  * Uses `mkdir` to create directories under the destination.

sudo ./test_udock.sh* Added checks to skip `.` and `..` entries and handle basic error conditions.

```* Tested the function on small directory trees before integrating it with `mdock build`.



---**Result:**



## 📝 Development Logs* `mdock build` correctly replicates rootfs directory trees into the internal images directory.

* The copying logic is robust enough for project use and demonstrates practical filesystem and storage management using OS-level APIs.

Development documented using STAR method (Situation, Task, Action, Result):

---

| Milestone | Document | Focus |

|-----------|----------|-------|### 6.4 (Optional) Challenge 4 – Resource Limits with `setrlimit` (If Implemented)

| M0 | [milestone-0-setup.md](logs/milestone-0-setup.md) | Project skeleton |

| M1 | [milestone-1-build.md](logs/milestone-1-build.md) | Image building |**Situation:**

| M2 | [milestone-2-run.md](logs/milestone-2-run.md) | Container execution |Containers could potentially consume unbounded memory or CPU, which does not illustrate resource control and can be problematic during testing.

| M3 | [milestone-3-management.md](logs/milestone-3-management.md) | ps and stop |

| M4 | [milestone-4-polish.md](logs/milestone-4-polish.md) | Resource limits |**Task:**

| M5 | [milestone-5-features.md](logs/milestone-5-features.md) | Management commands |Add optional resource limits (`--mem`, `--cpu`) to enforce per-container caps on memory and CPU time.



---**Action:**



## 📚 References* Extended `mdock run` to parse options like ``--mem 128M`` and ``--cpu 10``.

* Added input validation and helpful error messages for limit values.

- Silberschatz, Galvin, Gagne - *Operating System Concepts** In the child process, before calling `execve`, used `setrlimit` with:

- Kerrisk, Michael - *The Linux Programming Interface*

- `man 2 fork`, `man 2 execve`, `man 2 chroot`, `man 2 setrlimit`  * ``RLIMIT_AS`` for maximum address space (memory limit).

- [Docker Documentation](https://docs.docker.com/)  * ``RLIMIT_CPU`` for maximum CPU time in seconds.

- [OCI Runtime Specification](https://github.com/opencontainers/runtime-spec)* Enhanced exit code handling to detect resource limit violations:

  * ``SIGXCPU`` signal indicates CPU time limit exceeded.

---  * ``SIGKILL`` may indicate memory limit exceeded.

* Logged resource limits in RUN events for tracking.

<p align="center">

  <strong>Developed by Sowad Hossain</strong><br>**Result:**

  CSE 323 – Operating Systems Design<br>

  December 2025* Containers can be run with explicit resource bounds, showcasing how an OS controls memory and CPU per process.

</p>* This feature strengthened the connection between the project and the course’s focus on memory management and processor scheduling.



<p align="center">---

  <em>"Understanding containers from the ground up"</em>

</p>## 7. Relation to Contemporary Operating Systems


uDock is intentionally inspired by **container technologies** used in modern systems, such as Docker and the way **Android** and other mobile OSes isolate applications:

* Each container corresponds to a separate process with its own filesystem view, similar to how containers and Android apps run in isolated sandboxes.
* Commands like `mdock run`, `ps`, and `stop` resemble basic container lifecycle management in real container runtimes.
* Optional use of `setrlimit` for memory and CPU limits reflects how modern OSes and container platforms enforce resource quotas.

This ties the project directly to the course objective of demonstrating knowledge of **contemporary operating systems** and relating classic OS concepts (processes, memory, storage, scheduling) to modern abstractions (containers, app sandboxes).

---

## 8. Limitations and Future Work

Current limitations:

* Not a secure container runtime: no full namespace or cgroup isolation.
* No networking isolation or per-container network configuration.
* Uses simple text-based metadata; no transactional or crash-safe database.
* Assumes a Linux environment with a reasonably standard filesystem layout.

Possible future extensions:

* Use Linux namespaces and cgroups for stronger isolation.
* Add resource usage statistics (CPU, memory) in `mdock ps` using `/proc`.
* Support running arbitrary commands inside the container (`mdock run <image> -- /bin/ls /`).
* Improve metadata robustness using atomic file updates or a small embedded database.

---

*End of README.*
