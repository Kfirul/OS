#include <openssl/bio.h> // Include for OpenSSL BIO functions (Basic I/O abstract interface)
#include <openssl/evp.h> // Include for OpenSSL's high-level cryptographic functions (EVP)
#include <string.h> // Include for string handling functions, such as strlen
#include <stdio.h> // Include for standard input/output functions, such as FILE operations
#include <math.h> // Include for mathematical operations, such as ceil
#include <stdlib.h> // Include for standard library functions, such as malloc, free, and exit
#include <unistd.h> // Include for POSIX API, such as read, write, close functions
#include <netinet/in.h> // Include for Internet Protocol family, such as sockaddr_in
#include <sys/socket.h> // Include for socket functions and definitions
#include <sys/types.h> // Include for system data types, such as socket types
#include <sys/wait.h> // Include for process control, such as waitpid
#include <signal.h> // Include for signal handling, such as signal function
#include <errno.h> // Include for error number definitions, useful for error handling
#include <arpa/inet.h> // Include for functions related to internet operations (inet_ntop, inet_pton)
#include <netdb.h> // Include for network database operations (getnameinfo, getaddrinfo)
#include <fcntl.h> // Include for file control options (fcntl function)
#include <sys/stat.h> // Include for file status (used for mkdir function)

#define PORT "8080" // Define the port number for the server
#define BACKLOG 100 // Define the maximum number of pending connections

// Function to handle client requests
void handle_client(int socket_client, char *home_path) {
    char *encoded_str;
    char buffer[1024]; // Buffer to store data from client
    char msg[1024]; // Buffer to store messages to be sent to client
    struct flock fl = { // File lock structure
        .l_type = F_WRLCK, // Type of lock: Write lock
        .l_whence = SEEK_SET, // Relative to the start of the file
        .l_start = 0, // Start of the lock
        .l_len = 0, // Length of the lock (0 = until EOF)
    };

    int len = recv(socket_client, buffer, sizeof(buffer) - 1, 0); // Receive data from client
    buffer[len] = '\0'; // Null-terminate the received string
    printf("Received Request: %s\n", buffer);
    
    // Check if the request starts with "POST"
    if (strncmp(buffer, "POST", 4) == 0) {
        // Extract the file path from the request
        char *path = strtok(buffer + 5, "\r\n"); // Extract the path from the request
        // Construct the complete file path including the home directory
        char *file_path = (char *)malloc(strlen(home_path) + strlen(path) + 1); // Allocate memory for the file path
        sprintf(file_path, "%s%s", home_path, path); // Concatenate the home path with the file path
        printf("Requested File Path: %s\n", file_path);
        
        // Create the directory structure if necessary
        char *dir_path = strdup(file_path); // Duplicate the file path
        char *last_slash = strrchr(dir_path, '/'); // Find the last '/' character
        if (last_slash != NULL) {
            *last_slash = '\0'; // Set it to null to get the directory path
            mkdir(dir_path, 0755); // Create the directory with permissions
        }
        free(dir_path); // Free the directory path
        
        // Open the file for writing
        int fd = open(file_path, O_WRONLY | O_CREAT, 0644); // Open or create the file
        
        // Return an error if the file could not be opened
        if (fd == -1) {
            sprintf(msg, "HTTP/1.1 404 Not Found\r\n\r\n"); // Prepare the error message
            send(socket_client, msg, strlen(msg), 0); // Send the error message to the client
            free(file_path); // Free the file path
            exit(1); // Exit with error
        }
        
        // Acquire a write lock on the file
        if (fcntl(fd, F_SETLK, &fl) == -1) {
            sprintf(msg, "HTTP/1.1 500 Internal Server Error\r\n\r\n"); // Prepare the error message
            send(socket_client, msg, strlen(msg), 0); // Send the error message to the client
            free(file_path); // Free the file path
            exit(1); // Exit with error
        }

        // Process and write data to the file
        char *data = buffer + 5 + strlen(path) + 2; // Move data pointer to start of content
        len = len - (data - buffer); // Adjust len based on the new start position
        
        while (1) {
            // Ensure null termination before processing
            data[len] = '\0';
            
            // Check for end of data
            if ((len >= 4) && strcmp(data + len - 4, "\r\n\r\n") == 0) {
                len -= 4; // Adjust length to exclude "\r\n\r\n"
                data[len] = '\0'; // Null-terminate the data
                write(fd, data, strlen(data)); // Write the data to the file
                break; // Break the loop
            }
            else {
                write(fd, data, strlen(data)); // Write the data to the file
                len = recv(socket_client, buffer, sizeof(buffer) - 1, 0); // Receive more data
                data = buffer; // Reset pointer to the start of the buffer for new data
                data[len] = '\0'; // Null-terminate the received data
            }
        }

        // Release the lock
        fl.l_type = F_UNLCK; // Set the lock type to unlock
        fcntl(fd, F_SETLK, &fl); // Release the lock
        
        // Clean up and send response to client
        free(file_path); // Free the file path
        close(fd); // Close the file
        
        sprintf(msg, "HTTP/1.1 200 OK\r\n\r\n"); // Prepare the success message
        send(socket_client, msg, strlen(msg), 0); // Send the success message to the client
    }
    else if (strncmp(buffer, "GET", 3) == 0) {
        // Extract the file path from the request
        char *path = strtok(buffer + 4, "\r\n\r\n"); // Extract the path from the request
        
        // Construct the complete file path including the home directory
        char *file_path = (char *)malloc(strlen(home_path) + strlen(path) + 1); // Allocate memory for the file path
        sprintf(file_path, "%s%s", home_path, path); // Concatenate the home path with the file path        
        
        // Open the file for reading
        int fd = open(file_path, O_RDONLY); // Open the file for reading
        
        // Return an error if the file could not be opened
        if (fd == -1) {
            sprintf(msg, "HTTP/1.1 404 Not Found\r\n\r\n"); // Prepare the error message
            perror("open"); // Print the error message to stderr
            send(socket_client, msg, strlen(msg), 0); // Send the error message to the client
            free(file_path); // Free the file path
            exit(1); // Exit with error
        }
        
        // Put a read lock on the file
        fl.l_type = F_RDLCK; // Set the lock type to read lock
        if (fcntl(fd, F_SETLKW, &fl) == -1) {
            sprintf(msg, "HTTP/1.1 500 Internal Server Error\r\n\r\n"); // Prepare the error message
            perror("fcntl"); // Print the error message to stderr
            send(socket_client, msg, strlen(msg), 0); // Send the error message to the client
            free(file_path); // Free the file path
            exit(1); // Exit with error
        }
        
        // Read and send file contents
        while (1) {
            len = read(fd, buffer, sizeof(buffer)); // Read data from the file
            if (len <= 0) break; // Break the loop if end of file or error
            send(socket_client, buffer, len, 0); // Send the data to the client
        }

        // Release the lock
        fl.l_type = F_UNLCK; // Set the lock type to unlock
        fcntl(fd, F_SETLK, &fl); // Release the lock
        
        // Clean up and close the file
        free(file_path); // Free the file path
        close(fd); // Close the file
    }
    else {
        // Handle invalid request
        sprintf(msg, "HTTP/1.1 500 Internal Server Error\r\n\r\n"); // Prepare the error message
        printf("Invalid request received\n"); // Print to stdout
        send(socket_client, msg, strlen(msg), 0); // Send the error message to the client
        exit(1); // Exit with error
    }
}

// Function to extract the client's IP address
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr); // Return IPv4 address
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr); // Return IPv6 address
}

// Function to handle SIGCHLD signal to prevent zombie processes
void sigchld_handler(int s) {
    int saved_errno = errno; // Save the current value of errno
    while(waitpid(-1, NULL, WNOHANG) > 0); // Wait for all child processes
    errno = saved_errno; // Restore errno
}

int main(int argc, char *argv[]) {
    int socket_server, socket_client; // Socket descriptors for server and client
    struct addrinfo hints, *serv_info, *p; // Structures for storing address information
    struct sockaddr_storage client_addr; // Client's address
    socklen_t addr_size; // Size of the client address
    struct sigaction sa; // Structure for signal action
    int yes = 1; // Used for setsockopt
    char ipstr[INET6_ADDRSTRLEN]; // String to store client's IP address
    int rv; // Return value for getaddrinfo

    // Check if the correct number of arguments are passed
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <home_directory>\n", argv[0]);
        exit(1);
    }

    char *home_path = argv[1]; // Get the home directory from command line arguments

    memset(&hints, 0, sizeof(hints)); // Clear the hints structure
    hints.ai_family = AF_UNSPEC; // Set the address family to unspecified
    hints.ai_socktype = SOCK_STREAM; // Set the socket type to stream
    hints.ai_flags = AI_PASSIVE; // Use the IP of the host

    // Get server's address information
    if ((rv = getaddrinfo(NULL, PORT, &hints, &serv_info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Loop through all results and bind to the first we can
    for (p = serv_info; p != NULL; p = p->ai_next) {
        socket_server = socket(p->ai_family, p->ai_socktype, p->ai_protocol); // Create a socket
        if (socket_server == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(socket_server, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_server);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(serv_info); // Free the address information

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(socket_server, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // Set up the signal handler to reap zombie processes
    sa.sa_handler = sigchld_handler; // Point to the handler function
    sigemptyset(&sa.sa_mask); // Clear the mask
    sa.sa_flags = SA_RESTART; // Automatically restart system calls
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("Server is waiting for connections on port %s...\n", PORT);

    // Main accept() loop
    while (1) {
        addr_size = sizeof(client_addr); // Set the size of the client address
        socket_client = accept(socket_server, (struct sockaddr *)&client_addr, &addr_size); // Accept a connection
        if (socket_client == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), ipstr, sizeof(ipstr)); // Get client's IP address
        printf("Incoming connection from: %s\n", ipstr); // Print client's IP address

        if (!fork()) { // This is the child process
            close(socket_server); // Child doesn't need the listener
            handle_client(socket_client, home_path); // Handle the client request
            close(socket_client); // Close the client socket
            exit(0); // Exit the child process
        }

        close(socket_client); // Parent doesn't need the client socket
    }
    return 0; // Return from the program
}
