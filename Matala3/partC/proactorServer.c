#include <stdio.h>       // Standard input/output library
#include <stdlib.h>      // Standard library for memory allocation, process control, etc.
#include <unistd.h>      // POSIX operating system API
#include <string.h>      // String manipulation functions
#include <sys/socket.h>  // Socket API
#include <netinet/in.h>  // Internet address family
#include <pthread.h>     // POSIX threads library
#include "/home/roy/Desktop/OperationS3/partB/proactor.h" // Include the proactor header file

#define PORT 8080        // Define the port number for server to listen on
#define MAX_CLIENTS 100  // Maximum number of clients that can be connected
#define BUFFER_SIZE 1024 // Define buffer size for messages

int client_sockets[MAX_CLIENTS]; // Array to store client socket descriptors
int num_clients = 0;             // Counter for the number of connected clients
pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread-safe client list operations

// Function to add a new client socket to the list
void add_client_socket(int client_socket) {
    pthread_mutex_lock(&client_list_mutex); // Lock the mutex for safe operation
    if (num_clients < MAX_CLIENTS) {
        client_sockets[num_clients++] = client_socket; // Add new client socket to the list
    } else {
        printf("Max clients reached. Cannot add more.\n"); // Print message if max clients reached
    }
    pthread_mutex_unlock(&client_list_mutex); // Unlock the mutex
}

// Function to remove a client socket from the list
void remove_client_socket(int client_socket) {
    pthread_mutex_lock(&client_list_mutex); // Lock the mutex for safe operation
    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] == client_socket) {
            client_sockets[i] = client_sockets[num_clients - 1]; // Replace the socket with the last in the list
            num_clients--; // Decrement the client count
            break; // Exit the loop
        }
    }
    pthread_mutex_unlock(&client_list_mutex); // Unlock the mutex
}

// Function to broadcast a message to all clients except the sender
void broadcast_message(int sender_socket, const char* message) {
    pthread_mutex_lock(&client_list_mutex); // Lock the mutex for safe operation
    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] != sender_socket) {
            char full_message[BUFFER_SIZE]; // Buffer to store the full message
            snprintf(full_message, BUFFER_SIZE, "Client %d: %s", sender_socket, message); // Format the message
            write(client_sockets[i], full_message, strlen(full_message)); // Send the message to client
        }
    }
    pthread_mutex_unlock(&client_list_mutex); // Unlock the mutex
}

// Callback function to handle socket activity
void socketCallback(int sockfd) {
    while (1) {
        char buffer[BUFFER_SIZE]; // Buffer for storing received messages
        int readBytes = read(sockfd, buffer, BUFFER_SIZE); // Read data from socket
        if (readBytes <= 0) {
            printf("Client %d disconnected or error occurred\n", sockfd); // Print disconnect message
            remove_client_socket(sockfd); // Remove client from list
            close(sockfd); // Close the client socket
            break; // Exit the loop
        } else {
            buffer[readBytes] = '\0'; // Null-terminate the received message
            printf("Received message from Client %d: %s\n", sockfd, buffer); // Print the message
            broadcast_message(sockfd, buffer); // Broadcast the message to other clients
        }
    }
}

// Wrapper function for pthread_create compatibility
void *socketCallbackWrapper(void *arg) {
    int sockfd = (int)(intptr_t)arg; // Convert argument back to socket descriptor
    socketCallback(sockfd); // Call the socket callback function
    return NULL; // Return NULL as required by pthread_create
}

int main() {
    int server_fd, new_socket; // Socket descriptors for the server and new client
    struct sockaddr_in address; // Structure for the server address
    int addrlen = sizeof(address); // Length of the server address

    // Initialize Proactor
    initializeProactor();

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed"); // Print error message if socket creation fails
        exit(EXIT_FAILURE); // Exit with failure status
    }

    // Set server address parameters
    address.sin_family = AF_INET; // Set address family to Internet
    address.sin_addr.s_addr = INADDR_ANY; // Allow any IP to connect
    address.sin_port = htons(PORT); // Set the port number (convert to network byte order)

    // Bind the server socket to the specified address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed"); // Print error message if bind fails
        exit(EXIT_FAILURE); // Exit with failure status
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen"); // Print error message if listen fails
        exit(EXIT_FAILURE); // Exit with failure status
    }

    printf("Server started on port %d\n", PORT); // Print server start message

    while (1) {
        // Accept a new client connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept"); // Print error message if accept fails
            continue; // Continue to the next iteration
        }

        printf("Client %d connected\n", new_socket); // Print message indicating new client connected
        add_client_socket(new_socket); // Add new client socket to the list

        // Create a thread for each client
        pthread_t thread; // Declare a thread variable
        pthread_create(&thread, NULL, socketCallbackWrapper, (void *)(intptr_t)new_socket); // Create a thread for the client
    }

    // Cleanup
    cleanupProactor(); // Clean up resources used by Proactor

    return 0; // Return success status
}
