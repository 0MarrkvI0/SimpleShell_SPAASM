// ==================== Assignment No. 2 ====================
// Author: Martin Kvietok
//
// Description:
//This Unix-based shell runs on Linux and provides basic shell functionality with support for special character parsing and command execution. In addition to core shell features, it simulates networking behavior by acting as either a client or a server. Clients send text messages, and the server responds by converting the text to uppercase. The server also monitors active connections and handles client disconnections gracefully, maintaining continuous availability.
//
// Deadline: 20.4.2025
// Year, Semester, Program: [2025, 4, B_INFO]


// These are custom header files that likely define helper functions and structures
#include "server_utils.h"  // Contains functions and definitions for server setup and handling
#include "client_utils.h"  // Contains functions and definitions for client setup and handling
#include "shell.h"         // Likely defines shell behavior or interactive session features

// Define constants for maximum command line length and maximum number of arguments
#define MAX_LINE 1024  // Maximum number of characters in a single input line
#define MAX_ARGS 64    // Maximum number of command-line arguments supported

// Declare global pointers to hold information about the server and client connections
ClientConnection *client = NULL;  // Will point to the client connection data
ServerConnection *server = NULL;  // Will point to the server connection data

#define MAX_BUFF_LEN 64  // Define a maximum buffer length (not used in this file but may be used in shell.h)

// Function to run the server-side logic
void run_server(char **args)
{
    // Print basic information for debugging purposes
    printf("Server\n");
    printf("Arguments:\n");
    for (int i = 0; args[i] != NULL; i++) 
    {
        printf("  args[%d] = %s\n", i, args[i]);  // Print each command-line argument
    }

    // Step 1: Create the server connection based on input arguments
    server = create_server(args);

    // Step 2: Bind the server to its socket (either IP/port or UNIX domain)
    bind_server_socket(server);

    // Step 3: Begin handling server operations, likely in the background
    handle_server_background(server, 0);

    // Step 4: Free the allocated memory once done
    free(server);
}

// Function to run the client-side logic
void run_client(char **args)
{
    // Print basic information for debugging
    printf("Client\n");
    printf("Arguments:\n");
    for (int i = 0; args[i] != NULL; i++) 
    {
        printf("  args[%d] = %s\n", i, args[i]);  // Print each command-line argument
    }

    // Step 1: Create the client connection based on input arguments
    client = create_client(args);

    // Step 2: Connect or bind the client to the appropriate socket
    bind_client_socket(client);

    // Step 3: Start handling client logic, such as reading and sending data
    handle_client_background(client);

    // Step 4: Free the allocated memory once done
    free(client);
}

// Main function: program entry point
int main(int argc, char *argv[]) 
{
    // Check if the user provided enough arguments
    if (argc < 2)
    {
        printf("Usage: %s -s|-c [options]\n", argv[0]);
        return 1;
    }

    // If the first argument is "-s", launch the server
    if (strcmp(argv[1], "-s") == 0) 
    {
        run_server(&argv[2]);  // Pass the rest of the arguments to the server
    }
    // If the first argument is "-c", launch the client
    else if (strcmp(argv[1], "-c") == 0) 
    {
        run_client(&argv[2]);  // Pass the rest of the arguments to the client
    } 
    // If the argument is invalid, print an error
    else 
    {
        printf("Wrong arguments.\n");
        return 1;
    }

    return 0;  // Exit successfully
}


/* ==================== Evaluation ==================== */

/* Functionality:
 * - The program operates as a basic Unix shell on Linux with features including:
 *     - Command execution and parsing
 *     - Support for special characters: #, ;, <, >, |, and \
 *     - Custom prompt with username, hostname, and current time
 *     - Simulates networking behavior by acting as a client or server:
 *         - Server receives text, converts it to uppercase, and responds to the client
 *         - Server monitors and handles client disconnections
 *         - Client connects to the server and sends text input
 */

/* Development Environment:
 * - Written in C and compiled with GCC on a Unix-based Linux system
 * - Uses POSIX system calls and socket programming APIs
 * - Created with a custom Makefile
 */

/* Program Behavior:
 * - Accepts command-line arguments to specify mode (client/server) and address
 *     - -p for port, -u for UNIX socket, -i for IP address
 * - Shell and networking features work concurrently
 * - Handles errors in argument parsing, socket communication, and client/server logic
 * - Exits gracefully when errors occur or connections are closed
 * - Supports manual timeout configuration with -t [seconds]
 */

/* Assumptions for Correct Functioning:
 * - Only one of -p, -u, or -i is used at a time
 * - The system environment supports socket creation and file descriptor management
 * - Input commands are valid and available in the system PATH
 * - Only one client connects to the server at a time
 */

/* System Calls and Libraries:
 * - Shell logic uses: fork(), execvp(), pipe(), dup2(), wait()
 * - Networking uses: socket(), bind(), listen(), accept(), connect(), read(), write()
 * - Prompt uses: gethostname(), getlogin(), gettimeofday()
 */

/* Possible Improvements:
 * - Add support for multiple concurrent clients (e.g., using select(), poll(), or threads)
 * - Enhance input parsing for edge cases and escaped characters
 * - Implement advanced shell features such as command history, job control, and signal handling
 */

/* Algorithms Used:
 * - Custom parsing of user input, handling special characters and escape sequences
 * - Loop for handling networking I/O and converting incoming text to uppercase
 * - Prompt generation based on real-time system/user info
 */

/* Techniques and Tricks:
 * - Modular design to separate shell and networking functionality
 * - Child processes (via fork/exec) used for command execution without blocking shell input
 * - Pipes and file descriptor redirection used to support features like input/output redirection and pipelines
 */

/* Special Notes:
 * - Demonstrates the integration of basic shell behavior and client-server communication
 * - Showcases practical use of Unix system calls in a combined shell/networking environment
 */

/* Sources:
 * - Lectures and class materials
 * - Linux/POSIX manual pages
 * - Online references for socket and shell programming in C
 * - Example projects and tutorials on Unix shell and networking
 */

