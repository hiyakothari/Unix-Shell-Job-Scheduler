
# Unix Shell Job Scheduler


## 🌟 Features

- **Background Job Execution** - Run processes in the background using `&`
- **Foreground Job Control** - Manage jobs with `fg` and `bg` commands
- **Signal Handling** - Proper handling of `SIGINT` (Ctrl+C), `SIGTSTP` (Ctrl+Z), and `SIGCHLD`
- **Job Queue Management** - Track up to 100 concurrent jobs
- **Process State Tracking** - Monitor RUNNING, STOPPED, and DONE states
- **Built-in Commands** - `jobs`, `fg`, `bg`, `kill`, `help`, `exit`


## 🎥 Demo

```bash
shell> ./test_program &
[1] 12345 ./test_program
Count: 1
Count: 2

shell> jobs
Job ID  PID     State     Command
------  ------  --------  -------
[1]     12345   Running   ./test_program

shell> fg 1
Bringing job [1] to foreground: ./test_program
Count: 3
^Z
[1] Stopped (use 'fg 1' to resume)

shell> bg 1
Job [1] continued in background: ./test_program

shell> kill 1
Job [1] terminated
```


## 🚀 Usage

### Starting the Shell

```bash
./shell
```

### Basic Commands

| Command | Description | Example |
|---------|-------------|---------|
| `<command> &` | Run command in background | `./test_program &` |
| `jobs` | List all jobs | `jobs` |
| `fg <job_id>` | Bring job to foreground | `fg 1` |
| `bg <job_id>` | Resume stopped job in background | `bg 1` |
| `kill <job_id>` | Terminate a job | `kill 1` |
| `help` | Show help message | `help` |
| `exit` / `quit` | Exit the shell | `exit` |

### Keyboard Shortcuts

- **Ctrl+C** - Send SIGINT to foreground process (interrupt)
- **Ctrl+Z** - Send SIGTSTP to foreground process (suspend)

## 🏗️ Architecture

### Component Overview

```
┌─────────────────────────────────────────┐
│          Shell Process                  │
├─────────────────────────────────────────┤
│  Signal Handlers                        │
│  ├─ SIGCHLD → Reap child processes     │
│  ├─ SIGINT  → Forward to foreground    │
│  └─ SIGTSTP → Forward to foreground    │
├─────────────────────────────────────────┤
│  Job Queue (Array-based)                │
│  ├─ Job ID                              │
│  ├─ Process ID (PID)                    │
│  ├─ State (RUNNING/STOPPED/DONE)       │
│  └─ Command String                      │
├─────────────────────────────────────────┤
│  Command Processor                      │
│  ├─ Parser                              │
│  ├─ Built-in Commands                   │
│  └─ External Command Executor           │
└─────────────────────────────────────────┘
```

### Key System Calls

- **`fork()`** - Create child processes
- **`execvp()`** - Execute commands
- **`waitpid()`** - Wait for process state changes
- **`kill()`** - Send signals to processes
- **`signal()`** - Register signal handlers

### Job States

```
     ┌─────────┐
     │ RUNNING │
     └────┬────┘
          │
    Ctrl+Z│         SIGCONT
          ▼           │
     ┌─────────┐      │
     │ STOPPED │◄─────┘
     └────┬────┘
          │
   SIGTERM│SIGKILL
   exit() │
          ▼
     ┌─────────┐
     │  DONE   │
     └─────────┘
```

## 💡 Examples

### Example 1: Background Job Management

```bash
shell> ./test_program &
[1] 12345 ./test_program

shell> ./test_program &
[2] 12346 ./test_program

shell> jobs
Job ID  PID     State     Command
------  ------  --------  -------
[1]     12345   Running   ./test_program
[2]     12346   Running   ./test_program

shell> kill 1
Job [1] terminated

shell> jobs
Job ID  PID     State     Command
------  ------  --------  -------
[2]     12346   Running   ./test_program
```

### Example 2: Suspending and Resuming Jobs

```bash
shell> ./test_program
Count: 1
Count: 2
^Z
[1] Stopped (use 'fg 1' to resume)

shell> jobs
Job ID  PID     State     Command
------  ------  --------  -------
[1]     12345   Stopped   (foreground job)

shell> bg 1
Job [1] continued in background: (foreground job)

shell> fg 1
Bringing job [1] to foreground: (foreground job)
Count: 3
Count: 4
^C
Received SIGINT, exiting...
```

### Example 3: Multiple Job Control

```bash
shell> sleep 100 &
[1] 12345 sleep 100

shell> sleep 200 &
[2] 12346 sleep 200

shell> jobs
Job ID  PID     State     Command
------  ------  --------  -------
[1]     12345   Running   sleep 100
[2]     12346   Running   sleep 200

shell> kill 1
Job [1] terminated

shell> fg 2
Bringing job [2] to foreground: sleep 200
^C
```

## 📁 Project Structure

```
unix-shell-scheduler/
├── shell.c              # Main shell implementation
├── test_program.c       # Test program for job control
├── Makefile             # Build configuration
├── README.md            # This file
├── LICENSE              # MIT License
├── .gitignore           # Git ignore file
└── docs/
    └── CONTRIBUTING.md  # Contribution guidelines
```

## 📚 Learning Outcomes

This project demonstrates understanding of:

### Operating Systems Concepts
- Process creation and management (`fork`, `exec`)
- Inter-process communication
- Signal handling and propagation
- Process groups and sessions
- Zombie process prevention

### Systems Programming
- Low-level C programming
- POSIX system calls
- Memory management
- Error handling in system calls
- Signal-safe programming

### Data Structures
- Queue implementation for job management
- Efficient search algorithms
- State management

## 🛠️ Technical Details

### Supported Platforms
- Linux (Ubuntu, Debian, Fedora, Arch, etc.)
- macOS
- BSD variants
- Any POSIX-compliant Unix system
