# Unix shell
A custom Unix shell supporting job control, pipes, signal handling and sequential/conditional command execution. Written entirely from scratch in C. 

## Overview
This project is a command-line shell that allows users to execute commands in a terminal. The primary highlight of this shell is its support for **job control**, enabling multiple processes to run in the background while the shell continues executing other program in the foreground.

This shell was developed as an educational project to gain a deeper understanding of:
- The process model used in Unix and Unix-like operating systems.
- The programming interfaces provided by these operating systems.
- The behavior and structure of the terminal subsystem.

Through building this project, I explored concepts such as process creation, interprocess communication, signal handling, foreground and background job management and interaction with the terminal.

**Note**: All of the code in this project was written manually for learning purposes. **No chatbots or large language models (LLMs) were used in generating the shell code. The Makefile was generated with AI assistance.**

---
## Installation
> Note: This shell is designed for Unix-like systems and relies on POSIX APIs and libraries. It will not compile or run on non-Unix platforms such as Windows.

Make sure you've [GNU make](https://www.gnu.org/software/make/) and a C compiler installed. The default flag for the Makefile is set to use [GCC](https://gcc.gnu.org/) as the compiler, but you can use any other C compiler as well. Just don't forget to update the flag in the Makefile.

Firstly, clone the repo locally.

``` sh
git clone https://github.com/the-raghav-17/unix-shell-v2.git ~/unix-shell && cd ~/unix-shell
```

Then simply compile it using `make`.

``` sh
make
```

Run it using.

``` sh
./shell
```

To exit the shell, simply type `exit`.

``` sh
$> exit
[Exiting...]
```

---
## Features
1. **Command Chaining**
   - Supports conditional and sequential command execution using:
      - `&&` - execute next command only if the previous command succeeds.
      - `||` - execute next commandd only if the previous command fails.
      - `;`  - execute commands sequentially regardless of previous command's success.
2. **Pipelines**
   - Interprocess communication using pipes (`|`), allowing the output of one command to act as input to other.
3. **Background Execution**
   - Run processes in the background using `&`.
   - Background commands are executed within subshell, allowing the main shell to continue accepting and executing other commands.
4. **Process suspension and termination**
   - `Ctrl + z` to suspend a foreground process/pipeline.
   - `Ctrl + c` to terminate a foreground process/pipeline.
5. **Job control**
   - Asynchronous job handling with support for managing foreground and background jobs.
6. **Built-in commands**
   - `cd <dir>` – Change the current working directory (path expansion not supported).
   - `exit` - Exit the shell.
   - `exec <prog>` - Execute an external program (arguments to program not supported).
   - `jobs` - List all background jobs.
   - `fg <job-no>` - Bring the specified job to the foreground and resume execution.
      - Use `fg n` instead of `fg %n`.
   - `bg <job-no>` - Resume a stopped job in the background (syntax same as `fg`).
7. **Syntax error handling**
   - Detects and reports syntax errors in command input.

---
## Limitations
The following features are currently not supported:

- **No quoting support**
  - Single quotes (`'`) and double quotes (`"`) are not supported.
- **No input/output redirection**
  - File redirection operators, such as `<`, `>`, `>>` and `2>` are not implemented.
- **No shell variables**
  - No support for defining or using variables.
- **No shell expansions**
  - Command shell expansions are not impleneted.
- **No command history or input editing**
  - The shell doesn't support interactive line editing features such as navigating command history using up arrow or editing the typed command.
- **No tab auto-completion**

The reason for not implementing these features was that they don't add much educational value compared to the core concepts like job control and signal handling.

---
## Architecture and working
The overall workflow of the shell consists of **input handling**, **parsing**, **executing** and **job management**.

### 1. Input Handling
The shell reads a line from the terminal provided by the user.

### 2. Tokenization
The tokenizer then splits the string into recognizable tokens, such as:
- Comamnds
- Arguments
- Operators (`|`, `&&`, `||`, `;`, `&`)

These tokens serve as input for the parser.

### 3. Parsing
The parser processes the tokens and:
- Detects commands and arguments
- Identifies pipelines
- Identifies conditional operators (`&&` and `||`)
- Identifies command sequences (`;`)
- Detects background execution (`&`)

The parser constructs an **Abstract Syntax Tree (AST)** representing the structure of the command sequence the user entered.
This intermediate representations produced by the parser acts as input for the executor.

### 4. Execution
The executor traverses and evaluates the AST to run commands.

#### a. AST traversal
The executor traverses the AST representing the parsed command sequence.
Each leaf node of the AST represents a pipeline.

#### b. Pipeline execution
Pipelines are launched in foreground by default. The shell waits for the pipeline to complete before proceeding.

#### c. Signal handling
When a pipeline runs in foreground, if:
- SIGINT (`ctrl + c`) is detected, the shell terminates the running pipeline.
- SIGTSTP (`ctrl + z`) is detected, the shell suspends the pipeline and
  - Creates a job for the suspended pipeline.
  - Adds it to the job list

#### d. Pipeline completion
If no signal interrupts execution, the shell waits for the pipeline to finish and collects its exit status.

#### e. Conditional execution
The exit status of the pipeline determines how the executor proceeds through the AST:

- `&&` - Execute the next pipeline only if the previous one succeeded.
- `||` - Execute the next pipeline only if the previous one failed.

#### f. Background execution
If the entire command sequence is marked for background execution (`&`):

- The shell creates a subshell
- The subshell executes the AST associated with the command sequence.
- The main shell:
  - Registers the subshell as a background job
  - Adds it to the job list

### 5. Background job monitoring
While the shell continues accepting input, it also monitors background jobs (thanks to SIGCHLD signal). When job state changes occur (completion, termination, suspension), the shell reports the appropriate status updates to the user.

### 6. Loop
All of this is done while the shell is in an indefinite loop, unless the user explicitly wants to exit by typing `exit` command.

---
## What I learned

### 1. Unix/Linux Programming interface
Gained deeper understanding of system calls, process management and the programming interfaces provided by Unix and Linux

### 2. Process creation and monitoring
Learned to create and manage processes using `fork()`, `exec()` family of system calls and `wait()`/`waitpid()`/`waitid()`.

### 3. File descriptor manipulation and IPC
Learned how to redirect input/output streams and implement interprocess communication using pipes.

### 4. Signals in Unix/Linux
Learned signal handling, blocking and queuing using `signal()` and `sigaction()`. Learned about various signals and signal types like `SIGINT`, `SIGTSTP` and `SIGCHLD`.

### 5. Terminal subsystem
Understood how terminals manage foreground and background process groups and how signals interact with them.

### 6. Process groups and terminal control
Learned to manage process groups to simplify signal delivery. Used `tcgetpgrp()` and `tcsetpgrp()` to manage terminal ownership.

### 7. Data structures
Applied linked lists, dynamic arrays and trees (ASTs) to manage jobs, command and pipelines efficiently.

### 8. Program management
Experienced the importance of modular code and clean interfaces as the shell grew in complexity.

### 9. Manual memory management and debugging memory errors
Used tools like Valgrind and GDB to find and debug various memory errors and leaks.

---
## References
- [A great blog to get an overview of terminals and where the shell sits](https://www.linusakesson.net/programming/tty/)
- [Another great series of blogs to get an overview of tty, pty and shells](https://dev.to/napicella/linux-terminals-tty-pty-and-shell-192e)
- [The official bash manual for getting details about bash's features](https://www.gnu.org/software/bash/manual/bash.html)
- [The official glibc documentation for getting an overview of various concepts like file descriptors, etc](https://sourceware.org/glibc/manual/latest/html_mono/libc.html)
- [The job control section from the glibc documentation to implement basic job control](https://sourceware.org/glibc/manual/latest/html_mono/libc.html#Job-Control)
- [The Linux Programming Interface (book)](https://www.oreilly.com/library/view/the-linux-programming/9781593272203/)
- [This tutorial from Stephen Brennon to create a basic shell](https://brennan.io/2015/01/16/write-a-shell-in-c/ )
- [This tutorial by Indradhanush to create a basic shell](https://github.com/indradhanush/rc-shell)
- [This tutorial on github on building a more complex shell](https://github.com/tokenrove/build-your-own-shell)
- [The official posix documentation for getting an idea of the shell's grammar](https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html)
- The Linux man pages for various system calls like `wait`, `exec`, `sigaction`, etc.
- Wikipedia articles on topics like job control, file descriptors, etc.
- [Crafting interpreters book to get an idea of lexing and parsing](https://craftinginterpreters.com/contents.html)
- Random google searches
- Various random stackoverflow threads
- Various random reddit threads

---
## Tools and technologies used
- GCC (Compiler)
- Make (Build system)
- GNU debugger
- Valgrind (Helps finding memory errors and bugs)
- ZSH (Shell)
- VS Codium (Open source code editor)
- (Doom) Emacs (Another editor)
- Linux Fedora (System)

---
## Bugs
- Trying to continue a stopped job in background using `bg` builtin leads to printing of job status notification multiple times (it should only print once).

---
## License
GPLv3
