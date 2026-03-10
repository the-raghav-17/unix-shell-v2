# Unix shell
A custom Unix shell written entirely from scratch in C. Supports job control, pipes, signal handling and sequential/conditional command execution.

## Overview
This is a commandline shell that allows you to execute commands in terminal. The main highlight of this shell is that it supports job control, i.e, multiple processes can execute in the background while the shell itself is executing some other program in foreground.

This is an educational project I created to understand the process model and programming interface of Unix and unix-like operating systems. It also gave me a better understanding of the terminal subsystem.

All of the code is written entirely in C without any help from AI chatbots and LLMs. All of the modules are written from scratch without use of any external libraries.

## Installation


## Features
1. Multiple process execution using `&&`, `||` and `;`
2. Interprocess communication using pipes `|`
3. Background execution using `&`
4. Suspension of running process by pressing `Ctrl-z`
5. Termination of running process by pressing `Ctrl-c`
6. Asynchronous waiting for jobs; suspension, termination or exiting of jobs executing in background is reported by the shell
7. Builtins
   1. `cd <dir>`    - Change directory (expansion not supported)
   2. `exit`        - Exit the shell
   3. `exec <prog>` - Launch an external program (arguments to program not supported)
   4. `jobs`        - List all jobs running in background
   5. `fg <job-no>` - Put job with number `<job-no>` into foreground and continue it. Simply use `fg n` instead of `fg %n`
   6. `bg <job-no>` - Continue job with number `<job-no>` in the background. Syntax like `fg`
8. Syntax error reporting

## Working of the shell (overview)
The shell roughly works like this:
1. Take user input
2. Tokenize the input into identifiable tokens
3. Parse the tokens into some intermediate representation
4. Execute the parsed representation
5. Repeat

In the execution step, the process is forked and the executable is searched in the PATH environment variable. After launching the executable, the shell then waits for it to finish. This is an example of synchronous process handling.

Background processes are handled asynchronously. Meaning the shell doesn't necessarily have to be blocked in order to wait for the background processes. If a state change is observed in any of the background processes (termination, suspension or exiting), the shell stops what it is doing and then reports the changes to the user.

For more details on internal working, check out `doc/internals.md`

## What I learned from the project
- Unix's process model:
  - Creation and execution of processes using `fork` and `exec` system calls
  - Interprocess communication between processes using anonymous pipes
  - How standard file descriptors of a process can be manipulated to change reading and writing files
  - How processes can be grouped together to manage them effectively.

- Monitoring child processes:
  - Synchronously waiting for child processes using `wait`, `waitpid` and `waitid` system calls
  - Exploitation of `SIGCHLD` signal to manage processes asynchronously by setting up a signal handler using `sigaction` and `signal` system calls

- Terminal subsystem
  - How terminals manage process groups by keeping only a single group in foreground and rest in the background.
  - Use of `tcgetpgrp` and `tcsetpgrp` system calls to query and change the foreground process group of a terminal respectively.

- Fundamentals of lexing and parsing

- Program management
  - Importance of well defined interfaces between the modules make life easier as the codebase grows.

- Debugging memory errors like segmentation fault, double free, dangling pointer dereference and NULL pointer dereference.
