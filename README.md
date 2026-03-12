# Unix shell
A custom Unix shell supporting job control, pipes, signal handling and sequential/conditional command execution. Written entirely from scratch in C. 

## Overview
This is a commandline shell that allows you to execute commands in the terminal. The main highlight of this shell is that it supports job control, i.e, multiple processes can execute in the background while the shell itself is executing some other program in foreground.

This is an educational project I created to understand the process model and programming interface of Unix and unix-like operating systems. It also gave me a better understanding of the terminal subsystem.

All of the code is written entirely in C without any help from AI chatbots and LLMs. All of the modules are written from scratch without use of any external libraries.

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

If a command (or sequence of commands) is appended by a `&`, then that command (or command sequence) is launched in the background as a background job. The commands in that job will be properly executed by a subshell that the parent shell spawns. The parent shell interacts 

## What I learned from the project
