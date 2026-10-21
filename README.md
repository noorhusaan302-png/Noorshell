# Noorshell 🚀

A professional-grade, custom Unix shell written from scratch in C for macOS and Linux systems. Built to implement core operating system abstractions and low-level POSIX system calls.

---

## 🛠️ Features & Architecture

* **Persistent REPL Loop:** Continuously reads user input, handles parsing, and manages memory cleanly.
* **Process Management:** Leverages `fork()`, `execvp()`, and `waitpid()` to execute external system binaries concurrently.
* **Dynamic Prompting:** Automatically queries the kernel via `getcwd()` to display the active working directory in real-time.
* **Robust Built-ins:** 
  * `cd [path]` (supports relative navigation and defaults to `HOME` when invoked alone).
  * `exit` (cleanly terminates the REPL session).
* **I/O Redirection:** Supports both input (`<`) and output (`>`) file redirection using `open()`, `dup2()`, and file descriptor manipulation.
* **Background Execution (`&`):** Allows non-blocking asynchronous process execution, returning control to the prompt instantly while printing background PIDs.
* **POSIX Piping (`|`):** Implements bidirectional process chaining using `pipe()`, routing the standard output of a left-hand command directly into the standard input of a right-hand command.

---

## ⚙️ Getting Started & Compilation

### Prerequisites
* A Unix-based environment (macOS or Linux)
* GCC (GNU Compiler Collection)

### Quick Start
1. Compile the shell:
   ```bash
   gcc shell.c -o shell
   ```
2. Run your custom shell:
   ```bash
   ./shell
   ```
