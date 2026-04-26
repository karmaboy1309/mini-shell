<div align="center">

# 🐚 mysh — Mini Unix Shell

A production-quality, POSIX-compliant Unix shell written in C.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C Standard](https://img.shields.io/badge/C-C99%2FPOSIX-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS-lightgrey.svg)]()

</div>

---

## 📋 Overview

**mysh** is a feature-rich, modular Unix shell built from scratch in C. It demonstrates core operating system concepts including process creation (`fork`/`exec`), inter-process communication (`pipe`), file descriptor manipulation (`dup2`), and signal handling — all wrapped in a clean, well-documented codebase.

Designed to be **GitHub-ready** and showcase strong systems programming skills.

---

## ✨ Features

| Category | Feature | Status |
|----------|---------|--------|
| **Core** | Custom colorized prompt (`mysh@user:~/path$`) | ✅ |
| **Core** | Command execution via `fork()` + `execvp()` + `wait()` | ✅ |
| **Core** | Multi-argument parsing (`ls -l -a /tmp`) | ✅ |
| **Built-ins** | `cd` (with `~`, `-`, `$HOME` support) | ✅ |
| **Built-ins** | `pwd` — print working directory | ✅ |
| **Built-ins** | `exit [code]` — terminate shell | ✅ |
| **Built-ins** | `history [N]` — show last N commands | ✅ |
| **Piping** | Multi-stage pipelines (`cmd1 \| cmd2 \| cmd3`) | ✅ |
| **Redirection** | Output `>`, Append `>>`, Input `<` | ✅ |
| **Background** | Background execution with `&` | ✅ |
| **Signals** | `Ctrl+C` (SIGINT) — interrupts child, not shell | ✅ |
| **Signals** | `Ctrl+D` (EOF) — graceful exit | ✅ |
| **History** | Circular buffer storing last 100 commands | ✅ |
| **Parsing** | Single & double quote support | ✅ |
| **Cleanup** | Zombie process reaping for background jobs | ✅ |

---

## 📁 Project Structure

```
mini-shell/
├── include/              # Header files
│   ├── parser.h          # Parser data structures & API
│   ├── executor.h        # Execution engine API
│   ├── builtins.h        # Built-in command API
│   └── utils.h           # Utility function API
├── src/                  # Source files
│   ├── main.c            # Shell loop, signal setup, REPL
│   ├── parser.c          # Tokenizer, pipe/redirection parsing
│   ├── executor.c        # fork/exec, pipes, dup2 redirection
│   ├── builtins.c        # cd, pwd, exit, history
│   └── utils.c           # String helpers, prompt rendering
├── Makefile              # Build system
├── .gitignore
├── LICENSE
└── README.md
```

### Module Responsibilities

| Module | Purpose |
|--------|---------|
| `main.c` | REPL loop, signal handling, prompt display |
| `parser.c` | Tokenizes input; splits pipes; extracts `<`, `>`, `>>`, `&` |
| `executor.c` | Creates pipes, forks children, sets up redirections, waits |
| `builtins.c` | In-process commands: `cd`, `pwd`, `exit`, `history` |
| `utils.c` | String trimming, prompt rendering, `safe_strdup()` |

---

## 🛠️ Build & Run

### Prerequisites

- **OS:** Linux or macOS
- **Compiler:** GCC (or any C99/POSIX-compliant compiler)
- **Tools:** `make` (optional — a quick-build command is also provided)

### Option 1: Using Make

```bash
# Standard build
make

# Run the shell
./bin/mysh

# Debug build (AddressSanitizer + UBSan)
make debug

# Clean build artifacts
make clean
```

### Option 2: Quick single-command build

```bash
gcc -Wall -Wextra -std=c99 -Iinclude src/*.c -o mysh
./mysh
```

---

## 🚀 Usage Examples

### Basic Commands

```bash
mysh@user:~$ ls -la
mysh@user:~$ echo "Hello, World!"
mysh@user:~$ whoami
mysh@user:~$ date
```

### Built-in Commands

```bash
mysh@user:~$ pwd
/home/user

mysh@user:~$ cd /tmp
mysh@user:/tmp$ cd -
/home/user

mysh@user:~$ cd ~
mysh@user:~$ history 5
     1  ls -la
     2  echo "Hello, World!"
     3  pwd
     4  cd /tmp
     5  history 5

mysh@user:~$ exit
```

### Piping

```bash
mysh@user:~$ ls -l | grep ".c"
mysh@user:~$ cat /etc/passwd | grep root | wc -l
mysh@user:~$ ps aux | sort -k3 -rn | head -5
```

### I/O Redirection

```bash
# Output redirection (overwrite)
mysh@user:~$ echo "hello" > output.txt

# Output redirection (append)
mysh@user:~$ echo "world" >> output.txt

# Input redirection
mysh@user:~$ wc -l < output.txt

# Combined
mysh@user:~$ sort < unsorted.txt > sorted.txt
```

### Background Execution

```bash
mysh@user:~$ sleep 10 &
[bg] 12345

mysh@user:~$ ls
[done] 12345 (exit 0)
file1.txt  file2.txt
```

### Pipes + Redirection

```bash
mysh@user:~$ cat /var/log/syslog | grep error | sort > errors.txt
```

### Signal Handling

```
mysh@user:~$ sleep 100
^C                        # ← kills sleep, shell survives
mysh@user:~$              # ← ready for next command
```

---

## 🏗️ Architecture

```
                    ┌─────────────┐
                    │   main.c    │
                    │  REPL Loop  │
                    │  Signals    │
                    └──────┬──────┘
                           │ raw input
                    ┌──────▼──────┐
                    │  parser.c   │
                    │  Tokenize   │
                    │  Pipe split │
                    │  Redir scan │
                    └──────┬──────┘
                           │ Pipeline struct
                    ┌──────▼──────┐
               ┌────┤ executor.c  ├────┐
               │    │ fork/exec   │    │
               │    │ pipe/dup2   │    │
               │    └─────────────┘    │
        ┌──────▼──────┐         ┌──────▼──────┐
        │ builtins.c  │         │  External   │
        │ cd,pwd,exit │         │  Commands   │
        │ history     │         │  (execvp)   │
        └─────────────┘         └─────────────┘
```

---

## 🔑 Key System Calls Used

| System Call | Purpose |
|-------------|---------|
| `fork()` | Create child process |
| `execvp()` | Execute external command (PATH lookup) |
| `waitpid()` | Wait for child process completion |
| `pipe()` | Create inter-process pipe |
| `dup2()` | Redirect file descriptors |
| `open()` / `close()` | File I/O for redirection |
| `sigaction()` | Install signal handlers |
| `chdir()` | Change working directory |
| `getcwd()` | Get current working directory |

---

## 📝 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

---

<div align="center">

**Built with ❤️ and systems programming expertise.**

</div>
