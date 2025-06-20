#include "client_utils.h"  // Header file containing definitions and functions for client operations
#include "shell.h"         // Header file for shell-related functions used in client mode

// Function to create and initialize a client connection structure based on command-line arguments
ClientConnection* create_client(char **args) 
{
    int i = 0;

    // Dynamically allocate memory for a new ClientConnection struct
    // This struct holds all necessary info about the client's configuration
    ClientConnection *client = malloc(sizeof(ClientConnection));

    // Check if memory allocation failed
    if (client == NULL) 
    {
        // Print error and exit if malloc fails
        perror("malloc");
        exit(1);
    }

    // Initialize the contents of the struct to zero
    // This ensures no garbage values exist before assignment
    memset(client, 0, sizeof(ClientConnection));

    // Set a default time limit (in seconds) for inactivity before disconnection
    client->time_limit = 60;


    // Parse each argument passed to the client program
    while (args[i] != NULL) 
    {
        // If "-p" is provided, use TCP and set the port number
        if (strcmp(args[i], "-p") == 0 && args[i + 1] != NULL) 
        {
            client->use_tcp = 1;  // Indicate that TCP will be used
            client->port = atoi(args[++i]);  // Convert port string to integer
            strncpy(client->ip, "127.0.0.1", MAX_IP_LEN);  // Default to localhost IP
            client->unix_path[0] = '\0';  // Clear UNIX path since not used
        }
        // If "-ip" is provided, set the IP address (TCP is still used)
        else if (strcmp(args[i], "-ip") == 0 && args[i + 1] != NULL) 
        {
            client->use_tcp = 1;  // TCP will be used
            strncpy(client->ip, args[++i], MAX_IP_LEN - 1);  // Copy provided IP
            client->port = -1;  // Port may be set later or unused
            client->unix_path[0] = '\0';  // Clear UNIX path
        }
        // If "-u" is provided, use UNIX domain socket
        else if (strcmp(args[i], "-u") == 0 && args[i + 1] != NULL) 
        {
            client->use_tcp = 0;  // Set to use UNIX socket
            client->port = -1;  // Port is irrelevant for UNIX sockets
            client->ip[0] = '\0';  // Clear IP field
            strncpy(client->unix_path, args[++i], MAX_UNIX_PATH - 1);  // Set UNIX socket path
        }
        // If "-t" is provided, update the time limit for inactivity
        else if (strcmp(args[i], "-t") == 0 && args[i + 1] != NULL)
        {
            client->time_limit = atoi(args[++i]);  // Convert timeout to integer
        }

        // Move to the next argument
        i++;
    }

    // Return pointer to initialized client structure
    return client;
}

// Function to create and connect a client socket based on TCP or UNIX configuration
void bind_client_socket(ClientConnection *client) 
{
    int s;

    // If TCP is being used
    if (client->use_tcp) 
    {
        struct sockaddr_in addr;  // Structure to hold TCP socket info

        // Create a TCP socket (IPv4, stream-based)
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == -1) 
        {
            perror("socket");  // Handle socket creation error
            exit(1);
        }

        // Clear the address structure and set the relevant fields
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;  // Use IPv4
        addr.sin_addr.s_addr = inet_addr(client->ip);  // Set IP address
        addr.sin_port = htons(client->port);  // Set port (convert to network byte order)

        // Attempt to connect to the server using TCP
        printf("Running as TCP client, connecting to %s:%d...\n", client->ip, client->port);
        if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) 
        {
            perror("connect");  // Handle connection error
            exit(2);
        }
    } 
    else 
    {
        struct sockaddr_un addr;  // Structure to hold UNIX socket info

        // Create a UNIX domain socket
        s = socket(AF_UNIX, SOCK_STREAM, 0);
        if (s == -1) 
        {
            perror("socket");  // Handle socket creation error
            exit(1);
        }

        // Clear and set UNIX socket address structure
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;  // Use UNIX domain
        strncpy(addr.sun_path, client->unix_path, sizeof(addr.sun_path) - 1);  // Set path

        // Attempt to connect to the UNIX socket
        printf("Running as UNIX client, connecting to %s...\n", client->unix_path);
        if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) 
        {
            perror("connect");  // Handle connection error
            exit(2);
        }
    }

    // Save the created socket in the client structure for future use
    client->socket = s;
}

// Utility function to initialize the file descriptor set for select()
void init_fd_set(int s, fd_set *rs) 
{
    FD_ZERO(rs);      // Clear the set
    FD_SET(0, rs);    // Add stdin (file descriptor 0) for keyboard input
    FD_SET(s, rs);    // Add server socket for incoming data
}

// Handles the server’s response received through the socket
void handle_server_response(int s) 
{
    char msg[MAX_MSG_LEN];  // Buffer to hold received message
    int r = read(s, msg, sizeof(msg) - 1);  // Read from socket

    // If data was received
    if (r > 0) 
    {
        msg[r] = '\0';  // Null-terminate the message
        printf("\n");
        write(1, msg, r);  // Write the message to stdout
    }
    else 
    {
        // If no data received, assume server disconnected
        printf("Server closed connection.\n");
        close(s);  // Close client-side socket
    }
}

// Handles background communication from server while user interacts with shell
void handle_client_background(ClientConnection *client) 
{
    pid_t pid = fork();  // Create a child process

    // Handle fork failure
    if (pid == -1) 
    {
        perror("fork failed");
        exit(1);
    } 
    // Parent process will handle user shell
    else if (pid > 0) 
    {
        run_shell(client->socket, 1);  // Launch shell in client mode
    } 
    // Child process handles asynchronous server responses
    else 
    {
        fd_set rs;  // Read set for select()
        struct timeval timeout;  // Timeout interval for inactivity

        while (1) 
        {
            // Set timeout duration for select()
            timeout.tv_sec = client->time_limit;
            timeout.tv_usec = 0;

            FD_ZERO(&rs);
            FD_SET(client->socket, &rs);  // Monitor the socket for incoming data

            // Wait for data or timeout
            int ready = select(client->socket + 1, &rs, NULL, NULL, &timeout);

            // Handle select() error
            if (ready < 0) 
            {
                perror("select");
                break;
            } 
            // If timeout occurred with no activity
            else if (ready == 0) 
            {
                printf("\nNo activity for 30 seconds. Closing connection.\n");
                close(client->socket);  // Close the connection
                printf("Connection closed (child).\n");
                break;
            } 
            // If socket is ready, handle server response
            else if (FD_ISSET(client->socket, &rs)) 
            {
                handle_server_response(client->socket);
                fflush(stdout);  // Ensure message is printed immediately
            }
        }

        // Clean up and exit the child process
        close(client->socket);
        exit(0);
    }
}

// Sends user input (a message string) to the server through the socket
void handle_user_input(int s, const char *msg) 
{
    if (msg != NULL) 
    {
        // Send the entire string to the server
        write(s, msg, strlen(msg));
    }
}
