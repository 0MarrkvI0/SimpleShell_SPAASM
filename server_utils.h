#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

// Standard library headers for input/output, memory management, string operations, etc.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// System headers for socket programming
#include <sys/socket.h>    // Main socket API definitions
#include <sys/un.h>        // UNIX domain sockets
#include <sys/types.h>     // Data types used in socket functions
#include <sys/select.h>    // For multiplexing I/O using select()

// Additional headers for TCP/IP socket functionality
#include <netinet/in.h>    // IP socket address structure and protocols
#include <arpa/inet.h>     // Functions for IP address conversion
#include <fcntl.h>         // File control options (e.g., non-blocking mode)

// Constant defining the maximum length of an IP address string (e.g., "255.255.255.255" + null)
#define MAX_IP_LEN 16

// Maximum length allowed for UNIX socket path (as per sockaddr_un.sun_path)
#define MAX_UNIX_PATH 108

// Maximum length for input/output buffers (e.g., read/write operations)
#define MAX_BUFF_LEN 64

// Definition of the ServerConnection struct, which holds the configuration and state of the server
typedef struct {
    int use_tcp;                    // Flag to indicate whether TCP (1) or UNIX socket (0) is used
    int port;                       // Port number used when operating over TCP
    char ip[MAX_IP_LEN];            // IP address for TCP communication
    char unix_path[MAX_UNIX_PATH];  // Filesystem path to the UNIX domain socket
    int listening_socket;           // Socket descriptor used by server to listen for new connections
    int connecting_socket;          // Socket descriptor representing an active connection with a client
    int time_limit;                 // Optional timeout value (in seconds) for inactivity or session management
} ServerConnection;

// Function prototype: Creates and initializes a ServerConnection struct using provided arguments
ServerConnection* create_server(char **args);

// Function prototype: Binds the server socket based on TCP or UNIX socket settings
void bind_server_socket(ServerConnection *server);

// Function prototype: Manages background server behavior (e.g., accepting connections)
void handle_server_background(ServerConnection *server, int console);

// Function prototype: Handles client-server communication (data transmission and reception)
void handle_server_communication(ServerConnection *server);

// Function prototype: Performs resource cleanup (e.g., closing sockets, removing UNIX socket file)
void cleanup(ServerConnection *server);

#endif // SERVER_UTILS_H
