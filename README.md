# 🐳 uDock — Minimal Container Runtime (from scratch)

<p align="center">
  <img src="https://img.shields.io/badge/Language-C-blue?style=for-the-badge&logo=c" alt="C Language">
  <img src="https://img.shields.io/badge/Platform-Linux-orange?style=for-the-badge&logo=linux" alt="Linux">
  <img src="https://img.shields.io/badge/Course-CSE%20323-green?style=for-the-badge" alt="CSE 323">
  <img src="https://img.shields.io/badge/Status-Complete-success?style=for-the-badge" alt="Complete">
</p>

**Student:** Sowad Hossain
**ID:** 2323614042
**Course:** CSE 323 — Operating Systems Design
**Project:** uDock — Minimal Docker-like Container Runtime in User Space (C on Linux)

---

## 🎯 Overview

**uDock** is a lightweight container runtime written in **C** using Linux **system calls**.
It demonstrates how container-like isolation can be built from first principles (educational focus — not production-grade like Docker).

### What “container” means here

uDock uses core OS mechanisms to approximate container behavior:

- **Process control:** `fork()`, `execve()`, `waitpid()`
- **Filesystem isolation:** `chdir()`, optional `chroot()`
- **Signals / lifecycle:** `kill()`, signal handling
- **Resource limits:** `setrlimit()` for memory / CPU limits
- **Logging:** stdout/stderr capture + event logs

---

## ✨ Features

### Core commands

- Build images from directory trees
- Run programs inside an isolated root filesystem
- List containers and status
- Stop containers using signals
- View logs
- Remove containers / images

### Resource controls

- Memory limit (e.g., `128M`, `1G`)
- CPU time limit (seconds)

---

## 📋 Project Information

| Field        | Details                             |
| ------------ | ----------------------------------- |
| Student Name | Sowad Hossain                       |
| Student ID   | 2323614042                          |
| Course       | CSE 323 – Operating Systems Design |
| Language     | C (POSIX APIs)                      |
| Platform     | Linux                               |

---

## 🚀 Quick Start

### 1) Compile

```bash
make clean
make
```

### 2) Setup demo environment

```bash
chmod +x setup_demo.sh
sudo ./setup_demo.sh
```

### 3) Run tests

```bash
chmod +x test_udock.sh
sudo ./test_udock.sh
```

### 4) Try demo programs

```bash
sudo ./mdock run demo hello
sudo ./mdock run demo webserver
sudo ./mdock run demo counter
```

---

## 📚 Command Reference

| Command                   | Description                 | Example                             |
| ------------------------- | --------------------------- | ----------------------------------- |
| `build <name> <dir>`    | Create image from directory | `./mdock build myimg /tmp/rootfs` |
| `run <image> [program]` | Start container             | `sudo ./mdock run demo hello`     |
| `ps`                    | List containers             | `./mdock ps`                      |
| `stop <id>`             | Stop running container      | `./mdock stop c1`                 |
| `logs <id>`             | View container logs         | `./mdock logs c1`                 |
| `rm <id>`               | Remove stopped container    | `./mdock rm c1`                   |
| `images`                | List images                 | `./mdock images`                  |
| `rmi <image>`           | Remove image                | `./mdock rmi demo`                |

### `run` options

| Option            | Example           | Meaning        |
| ----------------- | ----------------- | -------------- |
| `-e KEY=VALUE`  | `-e DEBUG=true` | Set env var    |
| `--mem SIZE`    | `--mem 256M`    | Memory limit   |
| `--cpu SECONDS` | `--cpu 10`      | CPU time limit |

---

## 🧪 Custom Programs for Testing

uDock includes example C programs that help demonstrate isolation and lifecycle control:

1. **hello.c** — prints greeting + environment info
2. **webserver.c** — basic HTTP server (port 8080)
3. **stress.c** — CPU intensive test (use `--cpu`)
4. **counter.c** — long-running (use `stop`)
5. **filetest.c** — filesystem operations to verify isolation

Example:

```bash
sudo ./mdock run demo stress --cpu 10
```

---

## 🧱 Project Structure

```text
uDock/
├── src/                 # main.c, container.c, image.c, utilities
├── include/             # header files
├── real_programs/       # C demo programs
├── logs/                # development logs (STAR method)
├── test_udock.sh        # automated tests
├── setup_demo.sh        # demo setup
└── HOW-TO-TEST.md       # testing guide
```

---

## 🧠 OS Concepts Demonstrated

| Concept                   | System Calls / Mechanism                 |
| ------------------------- | ---------------------------------------- |
| Process creation          | `fork()`                               |
| Program execution         | `execve()`                             |
| Process synchronization   | `waitpid()`                            |
| Filesystem isolation      | `chdir()`, `chroot()`                |
| Signals and control       | `kill()`, signal handling              |
| Resource limits           | `setrlimit()`                          |
| Logging / I/O redirection | `dup2()`, file redirection             |
| File operations           | `opendir()`, `readdir()`, `stat()` |

---

## 🧾 Development Journey (STAR)

### Situation

Docker is powerful but complex. To understand containers at the OS level, I needed a minimal runtime that exposes the underlying mechanisms (process creation, filesystem isolation, signals, and resource control).

### Task

Build a functional container runtime from scratch using C and Linux system calls, implementing core lifecycle commands similar to Docker.

### Action

Implemented uDock through milestones:

1. CLI + build system
2. Image building (`build`, `images`)
3. Container execution (`run` using `fork/exec/chroot`)
4. Lifecycle management (`ps`, `stop`, `rm`)
5. Resource controls (`setrlimit`)
6. Logging and cleanup features

### Result

A working container runtime with key OS concepts demonstrated in practice. It runs real programs inside isolated environments with controllable lifecycle and resource limits.

---

## 🧪 Testing Instructions

See **[HOW-TO-TEST.md](HOW-TO-TEST.md)** for detailed step-by-step testing and expected outputs.

---

## 🎬 Demo Video

Watch the full demonstration of uDock features:

[![uDock Demo Video](https://img.youtube.com/vi/_-RtKr80h7g/maxresdefault.jpg)](https://youtu.be/_-RtKr80h7g)

---

## 📚 References

- Silberschatz, Galvin, Gagne — *Operating System Concepts*
- Kerrisk — *The Linux Programming Interface*
- `man 2 fork`, `man 2 execve`, `man 2 chroot`, `man 2 setrlimit`
- Docker Documentation
- OCI Runtime Specification

---

<p align="center">
  <strong>Developed by Sowad Hossain</strong><br>
  CSE 323 – Operating Systems Design<br>
  December 2025<br>
  <em>"Understanding containers from the ground up"</em>
</p>
