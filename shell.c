#include "shell.h"
#include "client_utils.h"

void run_shell(int socket, int isClient)
{
     // Declare a character buffer to hold a line of user input
     char line[MAX_LINE];

     // Enter an infinite loop to keep the shell running until the user exits (e.g., with Ctrl+D or a built-in command)
     while (1) {
         display_shell_prompt();
 
         // Read a line of input from standard input (stdin)
         // fgets reads up to MAX_LINE characters or until newline
 
         // If fgets returns NULL (e.g., Ctrl+D / EOF), break the loop to exit the shell
         if (!fgets(line, MAX_LINE, stdin)) break;
 
         // --- Handle multi-line input ---
         // If the line contains a backslash ('\') and it is at the end of the line (before the newline),
         // this means the user wants to continue the command on the next line
         // For example:
         //    echo hello \ world
         // should be interpreted as "echo hello world"
         while (strchr(line, '\\') && line[strlen(line) - 2] == '\\') 
         {
 
             // Remove the backslash and newline character ('\n') at the end of the line
             // This is done by replacing the backslash with a null terminator, effectively shortening the string
             line[strlen(line) - 2] = '\0'; 
             printf("> "); 
             
             char next_line[MAX_LINE];
             // Read the next line of input into a temporary buffer.
             if (!fgets(next_line, MAX_LINE, stdin)) break;
 
             // Check if there's enough space in 'line' to concatenate 'next_line'
             if (strlen(line) + strlen(next_line) < MAX_LINE) 
             {
                 strcat(line, next_line);  // Append next_line to line
             }
             else 
             {
                 // If there's not enough space, print an error and stop reading further input
                 fprintf(stderr, "Error: Input too long, buffer overflow risk.\n");
                 break;
             }
         }
         process_line(line,socket, isClient);
     }
}

// Display the shell prompt with current time, username, and hostname
void display_shell_prompt() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    struct passwd *pw = getpwuid(getuid());
    char *username = pw->pw_name;

    time_t rawtime;
    struct tm *timeinfo;
    char time_str[6];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "%H:%M", timeinfo);

    printf("%s %s@%s# ", time_str, username, hostname);
}

// Handle line with multiple commands separated by ';' (semicolon)
void process_line(char *line, int socket, int isClient) 
{
    char *command;  // Pointer to store each individual command extracted from the line
    char *rest = line;  // Pointer to keep track of the remaining part of the line after extracting each command

    // Loop through the line, extracting commands that are separated by semicolons (';')
    while ((command = strsep(&rest, ";")) != NULL) 
    {
        // For each extracted command, execute the command (via run_command function)
        run_command(command,socket, isClient);
    }
}


// Execute a single command with checks for built-ins, pipes, redirection, and external commands
void run_command(char *command, int socket, int isClient) 
{
    // Trim newline and any leading/trailing whitespace characters from the command
    command[strcspn(command, "\n")] = '\0';  // Remove newline character
    while (*command == ' ') command++;  // Remove leading spaces

    // Remove comments from the command (anything after '#' is ignored)
    char *comment = strchr(command, '#');
    if (comment) *comment = '\0';  // Null-terminate the command at the comment character

    // If the command is empty after trimming and removing comments, return
    if (strlen(command) == 0) return;

    // Pipeline Handling
    // Find '|' in command
    char *pipe_pos = strchr(command, '|');
    if (pipe_pos) {
        *pipe_pos = '\0';  // Null-terminate the command before the pipe
        pipe_pos++;  // Point to the part after the pipe
        handle_pipeline(command, pipe_pos, socket, isClient);  // Handle the pipe logic
        return;  // Return after handling the pipeline
    }

    // Redirection Handling
    if (strchr(command, '<') || strchr(command, '>')) {
        handle_redirection(command);  // Handle input/output redirection
        return;
    }

    // Command Parsing (if no pipe or redirection, parse the command normally)
    char *argv[MAX_ARGS];  // Array to store parsed arguments
    int argc = 0;  // Argument count
    char *token = strtok(command, " \t");  // Tokenize command based on spaces or tabs
    while (token) {
        argv[argc++] = token;  // Store token as an argument
        token = strtok(NULL, " \t");  // Get the next token
    }
    argv[argc] = NULL;  // Null-terminate the argument array

    // Handle Built-in Commands
    
    // "help" command: Display help message
    if (strcmp(argv[0], "help") == 0) {
        printf("Simple Shell Help:\n");
        printf("  cd [dir]     - change directory\n");
        printf("  exit         - exit shell\n");
        printf("  help         - show this help message\n");
        printf("  quit           - Gracefully quit the shell\n");
        printf("  halt           - Immediately quit the shell\n");
        if (isClient)
        {
            printf("  send [msg]     - Send a message to the server\n");
        }
        
        printf("Supports:\n  Piping (|), Redirection (<, >), Multiple cmds (;), Comments (#)\n");
        return;
    }

    // "exit" command: Exit the shell
    if (strcmp(argv[0], "exit") == 0) exit(0);

    // "cd" command: Change the current directory
    if (strcmp(argv[0], "cd") == 0) {
        if (argv[1]) chdir(argv[1]);  // Change directory if argument is provided
        else fprintf(stderr, "cd: missing argument\n");  // Print error if no argument
        return;
    }
    // "send" && client != NULL
    if (strcmp(argv[0], "send") == 0 && isClient) {
        if (argv[1] != NULL) {
            // Concatenate all arguments (excluding "send")
            char msg[MAX_MSG_LEN] = {0}; // Make sure to initialize the message buffer
            
            // Start concatenating from argv[1] onward
            for (int i = 1; i < argc; i++) {
                strcat(msg, argv[i]);
                if (i < argc - 1) {
                    strcat(msg, " ");  // Add a space between words
                }
            }
    
            // Send the concatenated message
            handle_user_input(socket, msg);
        } else {
            printf("Error: No message provided to send.\n");
        }
        return;
    }

    if (strcmp(argv[0], "quit") == 0) 
    {
        write(socket, "disconnecting ...\n", 19);
        close(socket);
    
        const char *exit_command = "exit\n";
        
        // Write to stdin (File descriptor 0)
        if (write(STDIN_FILENO, exit_command, strlen(exit_command)) == -1) 
        {
            perror("Error writing to stdin");
            exit(1);
        }

        // Optionally, you can also trigger the shell to process the exit by explicitly calling exit
        exit(0);  // Exit the shell or program
    }
    

    if (strcmp(argv[0], "halt") == 0) 
    {
        kill(0, SIGTERM); // Terminate the whole process group (optional)
        exit(1); // Final quit
    }
    

    // Execute External Command (non-built-in)
    pid_t pid = fork();  // Create a new process
    if (pid == 0) {  // Child process
        execvp(argv[0], argv);  // Execute the external command
        perror("execvp");  // Print error if execvp fails
        exit(EXIT_FAILURE);  // Exit child process if command execution fails
    } else {  // Parent process
        waitpid(pid, NULL, 0);  // Wait for the child process to finish
    }
}



// Handle two commands connected with a pipe ('|'), redirecting output from the first command to the input of the second
void handle_pipeline(char *left_cmd, char *right_cmd, int socket, int isClient) {
    int pipefd[2];  // File descriptors for the pipe (pipefd[0] for reading, pipefd[1] for writing)
    pipe(pipefd);  // Create a pipe for communication between the two commands

    // Fork a new process for the first command (left_cmd)
    pid_t pid1 = fork();
    if (pid1 == 0) {
        // In the child process (first command)

        // Close the reading end of the pipe, as we only need to write to the pipe
        close(pipefd[0]);

        // Redirect stdout to the write-end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);

        // Close the write-end of the pipe after redirecting stdout
        close(pipefd[1]);

        // Run the left command (before the pipe)
        run_command(left_cmd, socket, isClient);

        // If run_command fails, exit the child process
        exit(EXIT_FAILURE);
    }

    // Fork another process for the second command (right_cmd)
    pid_t pid2 = fork();
    if (pid2 == 0) {
        // In the child process (second command)

        // Close the write-end of the pipe, as we only need to read from the pipe
        close(pipefd[1]);

        // Redirect stdin to the read-end of the pipe
        dup2(pipefd[0], STDIN_FILENO);

        // Close the read-end of the pipe after redirecting stdin
        close(pipefd[0]);

        // Run the right command (after the pipe)
        run_command(right_cmd,socket, isClient);

        // If run_command fails, exit the child process
        exit(EXIT_FAILURE);
    }

    // In the parent process: close both ends of the pipe
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}


// Handle I/O redirection for input ('<') and output ('>') in shell commands
void handle_redirection(char *command) 
{
    char *input_file = NULL, *output_file = NULL;

    // Output redirection: check if the command contains '>'
    if ((output_file = strchr(command, '>'))) 
    {
        // Split the command at '>'
        *output_file = '\0';  // Null-terminate the command before '>'
        output_file++;        // Move past the '>'
        
        // Skip any leading spaces after the '>'
        while (*output_file == ' ') output_file++;
    }

    // Input redirection: check if the command contains '<'
    if ((input_file = strchr(command, '<'))) 
    {
        // Split the command at '<'
        *input_file = '\0';  // Null-terminate the command before '<'
        input_file++;        // Move past the '<'

        // Skip any leading spaces after the '<'
        while (*input_file == ' ') input_file++;
    }

    // Parse the command and arguments (after handling redirection)
    char *argv[MAX_ARGS];
    int argc = 0;
    char *token = strtok(command, " \t");
    while (token) 
    {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }
    argv[argc] = NULL;  // Null-terminate the argument list

    // Fork a child process to execute the command
    pid_t pid = fork();
    if (pid == 0) 
    {
        // In the child process

        // Handle input redirection (if any)
        if (input_file) 
        {
            int fd = open(input_file, O_RDONLY);  // Open the input file for reading
            if (fd == -1) 
            { 
                perror("open input"); 
                exit(EXIT_FAILURE);  // Exit if opening the input file fails
            }
            dup2(fd, STDIN_FILENO);  // Redirect stdin to the input file
            close(fd);  // Close the file descriptor
        }

        // Handle output redirection (if any)
        if (output_file) 
        {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);  // Open the output file for writing (create if doesn't exist, clear file before reading and set 644 rights {owner - group - others})
            if (fd == -1) 
            { 
                perror("open output"); 
                exit(EXIT_FAILURE);  // Exit if opening the output file fails
            }
            dup2(fd, STDOUT_FILENO);  // Redirect stdout to the output file
            close(fd);  // Close the file descriptor
        }

        // Execute the command with arguments
        execvp(argv[0], argv);
        perror("execvp");  // If execvp fails, print an error
        exit(EXIT_FAILURE);  // Exit child process on failure
    } 
    else 
    {
        // In the parent process: wait for the child to finish
        waitpid(pid, NULL, 0);
    }
}
