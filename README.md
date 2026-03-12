# Unix shell
A custom Unix shell supporting job control, pipes, signal handling and sequential/conditional command execution. Written entirely from scratch in C. 

## Overview
This project is a command-line shell that allows users to execute commands in a terminal. The primary highlight of this shell is its support for **job control**, enabling multiple processes to run in the background while the shell continues executing other program in the foreground.

This shell was developed as an educational project to gain a deeper understanding of:
- The process model used in Unix and Unix-like operating systems.
- The programming interfaces provided by these operating systems.
- The behavior and structure of the terminal subsystem.
Through building this project, I explored concepts such as process creation, interprocess communication, signal handling, foreground and background job management and interaction with the terminal.

**Note**: All of the code in this project was written manually for learning purposes. **No chatbots or large language models (LLMs) were used in generating the code (except for the Makefile, which was produced by AI).**

## Installation
Make sure you've [GNU make](https://www.gnu.org/software/make/) and a C compiler installed. The default flag for the Makefile is set to use [GCC](https://gcc.gnu.org/) as the compiler, but you can use any other C compiler as well. Just don't forget to update the flag in the Makefile.

Firstly, clone the repo locally.

``` sh
git clone git@github.com:the-raghav-17/unix-shell-v2.git ~/unix-shell && cd ~/unix-shell
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

## Features
1. Multiple process execution using `&&`, `||` and `;`
2. Interprocess communication using pipes `|`
3. Background execution using `&`
4. Suspension of running processes by pressing `Ctrl-z`
5. Termination of running processes by pressing `Ctrl-c`
6. Asynchronous process handling (job control); 
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

If a command (or sequence of commands) is appended by a `&`, then that command (or command sequence) is launched in the background as a background job. The commands in that job will be properly executed by a subshell that the parent shell spawns. The parent shell interacts 

## What I learned from the project
