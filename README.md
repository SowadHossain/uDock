# uDock – Minimal Container Runtime (CSE 323 OS Project)

**Student Name:** Sowad Hossain

**ID:** 2323614042 

**Course:** CSE 323 – Operating Systems Design 

**Project Title:** uDock – Minimal Docker-like Container Runtime in User Space 

**Language / Platform:** C on Linux

---

## 1. Project Overview

uDock is a **minimal Docker-like container runtime** implemented as a user-space system tool in C on Linux.

It allows the user to:

* **Build images** from directory trees (a simple root filesystem).
* **Run containers** as child processes in an isolated root filesystem.
* **List containers** and their status.
* **Stop containers** using signals.
* **Log** all major actions (build, run, exit, stop) for later inspection.

The goal is **not** to re-implement Docker, but to design a small system that uses real operating system mechanisms to demonstrate core OS concepts from CSE 323: process management, storage management, asynchronism and signals, basic resource handling, and contemporary OS ideas such as containers.

---

## 2. Features

uDock currently supports the following commands:

### 2.1 `mdock build <image_name> <rootfs_dir>`

* Creates a new image with the name `<image_name>`.

* Recursively copies `<rootfs_dir>` into:

  ```text
  ~/.mdock/images/<image_name>/rootfs/
  ```

* Appends a record to `~/.mdock/images.db` with:

  * image name
  * rootfs path
  * creation timestamp

### 2.2 `mdock run <image_name>`

* Looks up `<image_name>` in `images.db` to find the rootfs path.
* Generates a unique container ID (e.g. `c1`, `c2`, …).
* `fork()`s a child process:

  * In the child:

    * `chdir()`s into the rootfs directory.
    * Optionally `chroot()`s to that directory (if permitted).
    * `execve()`s a shell (e.g. `/bin/sh`) inside the container.
  * In the parent:

    * Records the container in `~/.mdock/containers.db` with:

      * container ID
      * PID
      * image name
      * start time
      * status (`running` / `exited`)
    * Waits for the child and updates its status and exit code.

### 2.3 `mdock ps`

* Reads `containers.db` and prints a list of containers.
* For each container:

  * Checks if the PID is still alive (`kill(pid, 0)` or `/proc/<pid>`).
  * Computes uptime from start and end times.
* Output format (example):

  ```text
  ID    PID    IMAGE     STATUS     UPTIME
  c1    3214   myimg     running    00:01:23
  c2    3250   base      exited     00:00:05
  ```

### 2.4 `mdock stop <container_id>`

* Looks up the container ID in `containers.db`.
* If the container is running:

  * Sends `SIGTERM` to the PID.
  * Optionally falls back to `SIGKILL` if the process does not exit in time.
* Updates the container status to `stopped` / `killed`.

### 2.5 Logging

* All major events are logged to `~/.mdock/log.txt`, for example:

  ```text
  [2025-11-22T12:01:09] BUILD image=myimg src=./rootfs
  [2025-11-22T12:02:15] RUN   id=c1 image=myimg pid=3214
  [2025-11-22T12:03:10] EXIT  id=c1 status=0
  [2025-11-22T12:04:00] STOP  id=c1 signal=SIGTERM
  ```

---

## 3. How to Build and Run

### 3.1 Requirements

* Linux (with `/proc` filesystem)
* GCC or any C compiler supporting POSIX APIs
* Standard C library and POSIX headers

### 3.2 Build

From the project root directory:

```bash
make
```

This should compile the sources into a single executable:

```text
./mdock
```

### 3.3 Usage Example

1. **Prepare a root filesystem directory**, e.g.:

   ```bash
   mkdir -p ./rootfs/bin
   # Copy a shell and basic tools into ./rootfs/bin as needed
   ```

2. **Build an image:**

   ```bash
   ./mdock build myimg ./rootfs
   ```

3. **Run a container:**

   ```bash
   ./mdock run myimg
   ```

   This should drop you into a shell running with the image’s rootfs.

4. **List containers:**

   ```bash
   ./mdock ps
   ```

5. **Stop a container:**

   ```bash
   ./mdock stop c1
   ```

   (Assuming `c1` is a valid container ID.)

---

## 4. Design & Architecture

### 4.1 Directory Layout

* uDock home:

  ```text
  ~/.mdock/
  ```

* Images:

  ```text
  ~/.mdock/images/<image_name>/rootfs/
  ```

* Containers metadata (single file):

  ```text
  ~/.mdock/containers.db
  ```

* Images metadata:

  ```text
  ~/.mdock/images.db
  ```

* Logs:

  ```text
  ~/.mdock/log.txt
  ```

### 4.2 Main Components (Code-Level)

* `main.c`
  Parses command-line arguments and dispatches to subcommands (`build`, `run`, `ps`, `stop`).

* `image.c`

  * Implements `build` functionality.
  * Handles recursive directory copy.
  * Manages `images.db`.

* `container.c`

  * Implements `run`, `ps`, and `stop`.
  * Calls `fork`, `execve`, `chdir`, `chroot`.
  * Manages `containers.db` and container status.

* `fsutil.c`

  * Utility for directory copying (`copy_dir`), path handling.

* `log.c`

  * Appends formatted events to `log.txt`.

* `timeutil.c`

  * Handles timestamps and uptime calculation.

This modular structure mirrors a simple OS-style decomposition: image manager, container/process manager, filesystem utilities, and logging subsystem.

---

## 5. OS Concepts Covered (Mapping to CSE 323)

This project demonstrates several key topics from the CSE 323 course outline:

### 5.1 Operating Systems Structure

* Clear separation of concerns:

  * Image management (`build`, `images.db`)
  * Container lifecycle (`run`, `ps`, `stop`, `containers.db`)
  * Logging (`log.txt`)
* Simple but explicit architecture similar to OS subsystems.

### 5.2 Process State Transition & Process Management

* Uses `fork` to create container processes and `execve` to run a shell/program.
* Uses `waitpid` to observe process termination and exit status.
* Tracks container PIDs and statuses (`running`, `exited`, `stopped`) in `containers.db`.
* `mdock ps` shows which containers are still running by checking the underlying process state.

### 5.3 Asynchronism, Signals, Interrupts (Conceptually)

* `mdock stop` uses `kill` to send signals (`SIGTERM`, `SIGKILL`) to container processes.
* This models **asynchronous** control: the parent process (uDock) can asynchronously interrupt or terminate a child process.
* Conceptually related to software interrupts and signal handling.

### 5.4 Storage Management (Real and Auxiliary)

* Real storage:

  * Images: on-disk directory trees under `~/.mdock/images/`.
  * Containers: associated metadata and working directories.
* Auxiliary storage:

  * Metadata files: `images.db` and `containers.db`.
  * Logs: `log.txt` for build/run/stop/exit events.
* Demonstrates how an OS-style tool maintains persistent state and logs.

### 5.5 Memory / Resource Management (Optional Extension)

If resource limits are implemented (via `setrlimit(RLIMIT_AS, RLIMIT_CPU)`):

* Each container can have a cap on:

  * Maximum address space (virtual memory).
  * Maximum CPU time.
* Demonstrates basic memory and CPU resource management at the process level.

### 5.6 Processor Scheduling & Multi-Processing (Observed)

* Multiple containers are simply multiple processes managed by the OS scheduler.
* Running several containers concurrently demonstrates:

  * how the scheduler interleaves process execution,
  * how process creation and termination look from a user-space tool’s perspective.

### 5.7 Network & Security (Conceptual)

* Using `chdir` and optionally `chroot` simulates basic filesystem isolation.
* Conceptually related to how real containers and modern OSes (e.g. Android, Docker) isolate applications with separate filesystem views and permissions.
* In the report/presentation, this is connected to application sandboxing and process isolation in contemporary systems.

---

## 6. STAR – Challenges and How They Were Addressed

The following subsections describe some key challenges in **STAR format**, as requested.

---

### 6.1 Challenge 1 – Implementing `run` with Correct Process Isolation

**Situation:**
Initially, `mdock run` needed to launch a new program "inside" an image’s filesystem, but the process was still running in the host’s normal directory tree, and it wasn’t clear how to isolate it properly.

**Task:**
Design and implement a way to start a new process that uses the image’s root filesystem as its working environment, without breaking the host system.

**Action:**

* Used `fork()` to create a child process from uDock.
* In the child:

  * Called `chdir(rootfs_path)` to switch into the image’s rootfs directory.
  * Optionally called `chroot(rootfs_path)` (when permissions allowed) so that the process would see this directory as `/`.
  * Executed a shell inside that environment using `execve("/bin/sh", ...)`.
* In the parent:

  * Used `waitpid` to wait for the child’s termination and record its exit status.

**Result:**

* `mdock run <image_name>` successfully starts a shell that operates relative to the image’s rootfs, imitating a container.
* The host system remains unaffected, and the design cleanly separates parent/child responsibilities.
* This demonstrates practical use of process creation, `chdir`, `chroot`, and `execve`.

---

### 6.2 Challenge 2 – Keeping Consistent Container Metadata

**Situation:**
The `containers.db` file needed to track IDs, PIDs, statuses, and timestamps for all containers. Early versions often produced inconsistent data (e.g. status not updated, stale PIDs) if a run crashed or the program exited unexpectedly.

**Task:**
Ensure that container metadata is written and updated consistently, so that `mdock ps` shows correct information even after multiple runs and failures.

**Action:**

* Defined a simple, line-based textual format for `containers.db`, with key–value pairs per container.
* Centralized metadata updates in helper functions:

  * `add_container_record(...)` for new containers.
  * `update_container_status(...)` for status changes.
* Ensured that metadata is written after key events:

  * Immediately after `run` starts a container (status=`running`).
  * After the child process exits (status=`exited`, with exit code).
  * After `stop` sends signals and confirms the container is no longer running.
* Added checks in `mdock ps` to verify whether a PID is still alive and to correct the status if necessary.

**Result:**

* `containers.db` remains simple and robust enough for the project.
* `mdock ps` reliably displays current statuses and uptimes.
* This improved the system’s reliability and illustrated the importance of careful auxiliary storage management in OS-related tools.

---

### 6.3 Challenge 3 – Recursive Directory Copy for `build`

**Situation:**
`mdock build` needed to copy an entire directory tree (`rootfs_dir`) into the internal images folder. A naive copy risked missing subdirectories or mishandling file permissions.

**Task:**
Implement a reliable, recursive directory copy that can reproduce the `rootfs_dir` structure under `~/.mdock/images/<image_name>/rootfs/`.

**Action:**

* Wrote a `copy_dir(src, dst)` function that:

  * Uses `opendir` / `readdir` to iterate over directory entries.
  * Recursively calls itself when encountering subdirectories.
  * Uses `open`, `read`, `write`, and `close` to copy regular files.
  * Uses `mkdir` to create directories under the destination.
* Added checks to skip `.` and `..` entries and handle basic error conditions.
* Tested the function on small directory trees before integrating it with `mdock build`.

**Result:**

* `mdock build` correctly replicates rootfs directory trees into the internal images directory.
* The copying logic is robust enough for project use and demonstrates practical filesystem and storage management using OS-level APIs.

---

### 6.4 (Optional) Challenge 4 – Resource Limits with `setrlimit` (If Implemented)

**Situation:**
Containers could potentially consume unbounded memory or CPU, which does not illustrate resource control and can be problematic during testing.

**Task:**
Add optional resource limits (`--mem`, `--cpu`) to enforce per-container caps on memory and CPU time.

**Action:**

* Extended `mdock run` to parse options like `--mem=128M` and `--cpu=5s`.
* In the child process, before calling `execve`, used `setrlimit` with:

  * `RLIMIT_AS` for maximum address space.
  * `RLIMIT_CPU` for maximum CPU time.
* Interpreted the `waitpid` status to determine whether the process exited normally or was killed due to a resource limit signal.

**Result:**

* Containers can be run with explicit resource bounds, showcasing how an OS controls memory and CPU per process.
* This feature strengthened the connection between the project and the course’s focus on memory management and processor scheduling.

---

## 7. Relation to Contemporary Operating Systems

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
