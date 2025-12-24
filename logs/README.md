# Development Logs

This directory contains detailed development logs for each milestone of the uDock project. Each log documents the challenges faced, solutions implemented, and lessons learned using the STAR method.

## STAR Method

Each challenge is documented using:
- **Situation:** What were the challenges/problems
- **Task:** What needed to be accomplished
- **Action:** What was done to address the issue
- **Result:** What was the outcome

## Log Files

### [Milestone 0: Project Setup](milestone-0-setup.md)
- Project structure design
- Build system setup
- Command routing architecture
- Initial Git repository

### [Milestone 1: Image Building](milestone-1-build.md)
- Recursive directory copying
- Home directory expansion
- Image metadata storage
- Directory creation with parents

### [Milestone 2: Container Execution](milestone-2-run.md)
- Process creation with fork()
- chroot() permission handling
- Container ID generation
- Container metadata tracking
- Image lookup by name
- execve() error handling

### [Milestone 3: Container Management](milestone-3-management.md)
- Process liveness detection
- Uptime calculation from timestamps
- Formatted table output
- Signal-based container stopping
- Container ID lookup and validation
- Database updates after stop

### [Milestone 4: Polish & Extras](milestone-4-polish.md)
- Input validation for image names
- Duplicate image detection
- Memory limit parsing and implementation
- CPU limit with setrlimit
- Resource limit application in child process
- Argument parsing for optional flags
- Comprehensive documentation updates

## Purpose

These logs serve multiple purposes:

1. **Educational Reflection:** Document learning process and OS concepts applied
2. **Technical Reference:** Detailed implementation notes for future maintenance
3. **Problem-Solving Documentation:** Show approach to overcoming challenges
4. **Course Evaluation:** Demonstrate understanding of operating systems concepts

## Key Themes Across Milestones

### System Programming
- POSIX APIs (fork, exec, wait, kill, setrlimit)
- File I/O and directory operations
- Signal handling and process control
- Error handling and validation

### Operating Systems Concepts
- Process management and lifecycle
- Resource limits and scheduling
- Filesystem operations and isolation
- Contemporary container technologies

### Software Engineering
- Modular design and code organization
- Testing and validation strategies
- User experience and error messages
- Documentation and communication

---

**Project:** uDock - Minimal Container Runtime  
**Course:** CSE 323 - Operating Systems Design  
**Student:** Sowad Hossain (2323614042)
