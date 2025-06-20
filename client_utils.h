#ifndef CLIENT_H
#define CLIENT_H

// Standard I/O and memory management headers
#include <stdio.h>      // Standard input/output functions
#include <stdlib.h>     // General utilities (e.g., malloc, exit)
#include <string.h>     // String manipulation functions

// Headers for socket programming and inter-process communication
#include <sys/socket.h>     // Core socket APIs
#include <sys/un.h>         // UNIX domain socket structures
#include <sys/select.h>     // select() API for I/O multiplexing
#include <sys/wait.h>       // For waitpid() used in process control

#include <unistd.h>         // POSIX API (read, write, fork, etc.)
#include <netinet/in.h>     // Structures for TCP/IP socket communication
#include <arpa/inet.h>      // Functions like inet_addr() for IP conversion
#include <fcntl.h>          // File descriptor control (e.g., non-blocking)
#include <signal.h>         // Signal handling (e.g., SIGCHLD)

// Maximum allowed size for an IP address string (e.g., "127.000.000.001\0")
#define MAX_IP_LEN 16

// Maximum allowed length for a UNIX socket path (filesystem limit)
#define MAX_UNIX_PATH 108

// Maximum message buffer size for communication
#define MAX_MSG_LEN 64

// Structure to hold all necessary information for a client connection
typedef struct {
    int use_tcp;                        // Flag indicating whether TCP (1) or UNIX (0) is used
    int port;                           // TCP port number (valid only if use_tcp == 1)
    char ip[MAX_IP_LEN];               // IP address for TCP connection
    char unix_path[MAX_UNIX_PATH];     // Filesystem path for UNIX socket (used if use_tcp == 0)
    int socket;                         // File descriptor for the connected socket
    int time_limit;                     // Timeout in seconds for inactivity (optional feature)
} ClientConnection;

// Parses command-line arguments and returns a pointer to a dynamically allocated ClientConnection
ClientConnection* create_client(char **args);

// Establishes a socket connection to the server based on the client settings (TCP or UNIX)
void bind_client_socket(ClientConnection *client);

// Manages background client tasks such as receiving messages from the server or timing out
void handle_client_background(ClientConnection *client);

// Initializes the file descriptor set for the select() system call to monitor I/O
void init_fd_set(int s, fd_set *rs);

// Reads and handles a message sent by the server to the client
void handle_server_response(int s);

// Sends user-provided input (message) to the connected server
void handle_user_input(int s, const char *msg);

#endif // CLIENT_H
