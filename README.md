# Custom UNIX Shell - Assignment 1

## Overview

This project is a custom UNIX command-line shell developed as part of the Operating Systems Lab assignment. The shell interprets user commands, handles I/O redirection and pipes, manages background processes, and implements a set of built-in commands. The assignment involves incrementally building versions with specific features to simulate a fully functional command-line interface.

## Project Versions and Features

### Version 01
- **Prompt Display**: Shows a prompt (`SULEMANshell:-`) for user commands.
- **Command Execution**: Parses commands with options and arguments, forks a process, and executes the command.
- **Parent-Child Process Control**: The parent process waits for the child process to complete.
- **Exit with CTRL+D**: Allows the user to quit the shell by pressing `<CTRL+D>`.

### Version 02
- **I/O Redirection**: Supports input (`<`) and output (`>`) redirection.
  - Example: `mycmd < infile > outfile` reads input from `infile` and writes output to `outfile`.
- **Pipes**: Allows piping of commands, e.g., `cat /etc/passwd | wc`.

### Version 03
- **Background Execution**: Enables background execution of commands with `&`.
  - Example: `find / -name f1.txt &` runs the command in the background.
- **Signal Handling**: Uses `SIGCHLD` to handle zombie processes from background tasks.

### Version 04
- **Command History**: Implements command history recall using `!number`.
  - Example: `!-1` repeats the last command, `!1` repeats the first command in history.
- **History Management**: Tracks the last 10 commands, overwriting older entries as necessary.

### Version 05
- **Built-in Commands**: Adds several built-in commands:
  - `cd`: Changes the working directory.
  - `exit`: Exits the shell.
  - `jobs`: Lists background jobs.
  - `kill`: Terminates a background job.
  - `help`: Displays available built-in commands.

## Compilation and Execution

To compile a specific version:
```bash
gcc versionX.c -o myshell
```

To run the shell:
```bash
./myshell
```

Replace `versionX.c` with the version file you want to compile (e.g., `version01.c`, `version02.c`, etc.).
