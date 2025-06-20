#ifndef SHELL_H
#define SHELL_H

// Define constants for maximum input size and message length
#define MAX_LINE 1024          // Maximum allowed length for a command line input
#define MAX_ARGS 64            // Maximum number of arguments for a single command
#define MAX_MSG_LEN 64         // Maximum length for server/client messages

// Include necessary standard libraries for various functionalities
#include <stdio.h>             // Standard I/O functions (fgets, printf, fprintf, perror)
#include <stdlib.h>            // General utilities (exit, EXIT_FAILURE)
#include <string.h>            // String manipulation functions (strlen, strchr, strsep, strtok, strcmp)
#include <unistd.h>            // POSIX API (fork, execvp, getuid, chdir, close, dup2, gethostname)
#include <sys/wait.h>          // Process control (waitpid)
#include <fcntl.h>             // File control (open)
#include <time.h>              // Time-related functions (time, localtime, strftime)
#include <pwd.h>               // Password database (getpwuid, struct passwd for user info)


// Function prototypes:

// Main shell execution function that handles command input/output communication
// Arguments:
//  - socket: The socket file descriptor used for communication
//  - isClient: A flag indicating whether this instance is running as a client (1) or server (0)
void run_shell(int socket, int isClient);

// Displays the shell prompt which may include time, username, and hostname
void display_shell_prompt();

// Processes the input command line, splitting the command into arguments and handling special cases
// Arguments:
//  - line: The command line input from the user
//  - socket: The socket file descriptor used for communication
//  - isClient: A flag indicating whether this is a client (1) or server (0)
void process_line(char *line, int socket, int isClient);

// Executes the given command using the appropriate system call or custom execution logic
// Arguments:
//  - command: The full command to execute
//  - socket: The socket file descriptor used for communication
//  - isClient: A flag indicating whether this is a client (1) or server (0)
void run_command(char *command, int socket, int isClient);

// Handles pipeline execution (e.g., "cmd1 | cmd2"), running multiple commands in sequence
// Arguments:
//  - left_cmd: The command on the left side of the pipe
//  - right_cmd: The command on the right side of the pipe
//  - socket: The socket file descriptor used for communication
//  - isClient: A flag indicating whether this is a client (1) or server (0)
void handle_pipeline(char *left_cmd, char *right_cmd, int socket, int isClient);

// Handles I/O redirection in the shell (e.g., "cmd > file" or "cmd < file")
void handle_redirection(char *command);

#endif  // End of SHELL_H header guard
