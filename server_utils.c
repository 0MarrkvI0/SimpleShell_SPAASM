#include "server_utils.h"
#include "shell.h"

// Function to create a server connection based on arguments passed by the user
// Arguments:
//  - args: Command-line arguments that specify server settings (e.g., port, IP, socket type)
ServerConnection *create_server(char **args) 
{
    int i = 0;
    ServerConnection *server = malloc(sizeof(ServerConnection));  // Allocate memory for the server structure


    // Check if memory allocation was successful
    if (!server) 
    {
        perror("malloc");
        exit(1);
    }

    memset(server, 0, sizeof(ServerConnection));  // Initialize the server structure to zero

    server->time_limit = 60;  // Set default time limit to 60 seconds for connection timeout

    // Parse the arguments to set server settings
    while (args[i] != NULL) 
    {
        if (strcmp(args[i], "-p") == 0 && args[i + 1] != NULL)
        {
            server->use_tcp = 1;  // Use TCP for the server
            server->port = atoi(args[++i]);  // Set the server port
            strncpy(server->ip, "127.0.0.1", MAX_IP_LEN);  // Default IP address
            server->unix_path[0] = '\0';  // No UNIX socket path
        } 
        else if (strcmp(args[i], "-ip") == 0 && args[i + 1] != NULL) 
        {
            server->use_tcp = 1;  // Use TCP for the server
            strncpy(server->ip, args[++i], MAX_IP_LEN - 1);  // Set the server IP
            server->port = -1;  // No port specified
            server->unix_path[0] = '\0';  // No UNIX socket path
        } 
        else if (strcmp(args[i], "-u") == 0 && args[i + 1] != NULL) 
        {
            server->use_tcp = 0;  // Use UNIX socket
            server->port = -1;  // No port for UNIX socket
            server->ip[0] = '\0';  // No IP for UNIX socket
            strncpy(server->unix_path, args[++i], MAX_UNIX_PATH - 1);  // Set the UNIX socket path
        } 
        else if (strcmp(args[i], "-t") == 0 && args[i + 1] != NULL)
        {
            server->time_limit = atoi(args[++i]);  // Set the time limit for client connections
        }
        i++;
    }

    return server;  // Return the configured server structure
}

// Function to bind the server socket based on the configuration in the server structure
// Arguments:
//  - server: The server connection object containing the socket configuration
void bind_server_socket(ServerConnection *server) 
{
    int s;

    if (server->use_tcp)  // If using TCP, create a TCP socket
    {
        struct sockaddr_in addr;
        s = socket(AF_INET, SOCK_STREAM, 0);  // Create the socket (TCP)
        if (s == -1) 
        {
            perror("socket");
            exit(1);
        }

        memset(&addr, 0, sizeof(addr));  // Zero out the sockaddr_in structure
        addr.sin_family = AF_INET;
        
        // If IP address is provided, use it; otherwise, bind to INADDR_ANY
        if (server->ip[0] != '\0') 
        {
            if (inet_pton(AF_INET, server->ip, &addr.sin_addr) <= 0) 
            {
                perror("Invalid IP address");
                exit(1);
            }
        } 
        else 
        {
            addr.sin_addr.s_addr = INADDR_ANY;  // Bind to any available interface
        }

        addr.sin_port = htons(server->port);  // Set the server's port

        // Bind the socket to the specified address
        if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) 
        {
            perror("bind");
            exit(1);
        }

        // Start listening for incoming connections
        if (listen(s, 5) == -1) 
        {
            perror("listen");
            exit(1);
        }

        printf("Server is listening on IP %s, port %d...\n", server->ip[0] ? server->ip : "ANY", server->port);
    } 
    else  // If using UNIX socket, create and bind a UNIX socket
    {
        struct sockaddr_un addr;
        s = socket(AF_UNIX, SOCK_STREAM, 0);  // Create the socket (UNIX)
        if (s == -1) 
        {
            perror("socket");
            exit(1);
        }

        unlink(server->unix_path);  // Remove any existing socket file
        memset(&addr, 0, sizeof(addr));  // Zero out the sockaddr_un structure
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, server->unix_path, sizeof(addr.sun_path) - 1);  // Set UNIX socket path

        // Bind the socket to the specified UNIX path
        if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) 
        {
            perror("bind");
            exit(1);
        }

        // Start listening for incoming connections
        if (listen(s, 5) == -1) 
        {
            perror("listen");
            exit(1);
        }

        printf("Server is listening on UNIX socket %s...\n", server->unix_path);
    }

    server->listening_socket = s;  // Store the socket descriptor for later use
}

// Function to handle the server's background operations, such as accepting connections
// Arguments:
//  - server: The server connection object containing the socket configuration
//  - console: Flag to indicate whether the server is running interactively (1) or as a background process (0)
void handle_server_background(ServerConnection *server, int console) 
{
    struct timeval timeout;
    fd_set read_fds;
    int result;

    FD_ZERO(&read_fds);  // Clear the file descriptor set
    FD_SET(server->listening_socket, &read_fds);  // Add the listening socket to the set

    // Set the timeout for the select system call
    timeout.tv_sec = server->time_limit;
    timeout.tv_usec = 0;

    // Use select to check if a client is trying to connect
    result = select(server->listening_socket + 1, &read_fds, NULL, NULL, &timeout);

    if (result == -1) 
    {
        perror("select");
        exit(EXIT_FAILURE);
    }

    if (result == 0) 
    {
        // If no connection attempts within the time limit, print a message and return
        printf("No incoming connection. Shutting down.\n");
        cleanup(server);
        return;
    }

    // If a connection attempt is detected, accept the connection
    server->connecting_socket = accept(server->listening_socket, NULL, NULL);
    if (server->connecting_socket == -1) 
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Handle communication in the foreground if console flag is set
    if (console == 1)
    {
        handle_server_communication(server);
        return;
    }

    // If running in the background, fork a new process to handle the client
    pid_t pid = fork();

    if (pid == -1) 
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) 
    {
        // Child process: handle client communication
        handle_server_communication(server);
        close(server->connecting_socket);  // Close the socket after handling
        exit(0);
    } 
    else 
    {
        // Parent process: run the shell for additional commands
        run_shell(server->connecting_socket, 0); 
        close(server->connecting_socket);  // Close the socket after the command execution
        cleanup(server);  // Cleanup server resources
        return;
    }
}

// Function to handle communication with a connected client
// Arguments:
//  - server: The server connection object containing the socket configuration
void handle_server_communication(ServerConnection* server) 
{
    int r;
    char buff[MAX_BUFF_LEN];  // Buffer for reading data from the client

    const char *banner = "Hello from server\nSend a string and I'll send you back the upper case...\n";

    // Send a welcome message to the client
    if (write(server->connecting_socket, banner, strlen(banner)) == -1) 
    {
        perror("\nError sending banner to client");
        close(server->connecting_socket);  // Close socket on error
        return;
    }

    // Communication loop: read data from the client and send back in uppercase
    while ((r = read(server->connecting_socket, buff, sizeof(buff) - 1)) > 0) 
    {
        buff[r] = '\0';  // Null-terminate the received data
        printf("\nReceived message: %s", buff);  // Print the received message
        printf("\nReceived (%d bytes): %s\n", r, buff);

        // Convert the message to uppercase
        for (int i = 0; i < r; i++) 
        {
            buff[i] = toupper(buff[i]);
        }

        printf("Sending back: %s\n", buff);

        // Send the converted uppercase message back to the client
        if (write(server->connecting_socket, buff, r) == -1) 
        {
            perror("\nError sending back data to client");
            break;  // Stop if the write failed (client may have disconnected)
        }
        display_shell_prompt();  // Display the shell prompt
        fflush(stdout);
    }

    // Handle client disconnection or read error
    if (r == 0) 
    {
        printf("\nClient closed the connection.\n");
        fflush(stdout);
        display_shell_prompt();
        handle_server_background(server, 1);  // Continue accepting new connections
    } 
    else 
    {
        perror("read");
    }
    close(server->connecting_socket);  // Close the client connection
}

// Function to clean up the server resources, including closing the listening socket
// Arguments:
//  - server: The server connection object containing the socket configuration
void cleanup(ServerConnection *server) 
{
    close(server->listening_socket);  // Close the server's listening socket
    if (!server->use_tcp) 
    {
        unlink(server->unix_path);  // Remove the UNIX socket file if necessary
    }
}
