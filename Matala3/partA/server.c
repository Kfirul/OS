#include <stdio.h>       // Standard input/output library
#include <stdlib.h>      // Standard library for memory allocation, exit function, etc.
#include <string.h>      // String manipulation functions
#include <pthread.h>     // POSIX threads library
#include <sys/types.h>   // Definitions of data types used in system calls
#include <sys/socket.h>  // Socket API
#include <netinet/in.h>  // Internet address family
#include <unistd.h>      // POSIX operating system API
#include <stdbool.h>     // Boolean data type

#define BUFFER_SIZE 1024 // Define buffer size for messages
#define MAX_CLIENTS 10000 // Define maximum number of clients

// Global variables for managing clients and mutex
int clientSockets[MAX_CLIENTS]; // Array to hold client sockets
int numClients = 0; // Counter for the number of connected clients
pthread_mutex_t clientMutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for synchronizing access to clientSockets
int clientCounter = 0; // Counter to assign client numbers

// Function to handle communication with a client
void *handleClient(void *pClientSocket);

// Function to send a message to all clients except the sender
void sendToAllClients(int senderSocket, char *message, int senderClientNumber)
{
    pthread_mutex_lock(&clientMutex); // Lock the mutex to protect shared resources
    char formattedMessage[BUFFER_SIZE]; // Buffer to hold the formatted message
    // Format message differently if it's a sign-out message
    if (strncmp(message, "SIGNOUT", 7) == 0) {
        snprintf(formattedMessage, BUFFER_SIZE, "Client %d left the chat", senderClientNumber);
    } else {
        snprintf(formattedMessage, BUFFER_SIZE, "Client %d says: %s", senderClientNumber, message);
    }
    for (int i = 0; i < numClients; i++) // Iterate through all clients
    {
        int clientSocket = clientSockets[i]; // Get client socket
        if (clientSocket != senderSocket) // Check if it's not the sender
        {
            send(clientSocket, formattedMessage, strlen(formattedMessage), 0); // Send message to client
        }
    }
    pthread_mutex_unlock(&clientMutex); // Unlock the mutex
}

// Function to handle communication with a client
void *handleClient(void *pClientSocket)
{
    int clientSocket = *(int *)pClientSocket; // Dereference the pointer to get client socket
    free(pClientSocket); // Free the allocated memory

    pthread_mutex_lock(&clientMutex); // Lock the mutex to protect shared resources
    int clientNumber = ++clientCounter; // Increment client counter and assign client number
    clientSockets[numClients++] = clientSocket; // Add new client socket to array
    pthread_mutex_unlock(&clientMutex); // Unlock the mutex

    char message[BUFFER_SIZE]; // Buffer to store client messages

    while (1) // Infinite loop to handle client communication
    {
        bzero(message, BUFFER_SIZE); // Clear the message buffer
        ssize_t bytesReceived = recv(clientSocket, message, BUFFER_SIZE, 0); // Receive message from client
        if (bytesReceived < 1) // Check for disconnection or error
        {
            printf("Client %d left the chat\n", clientNumber); // Print message indicating client left

            pthread_mutex_lock(&clientMutex); // Lock the mutex
            for (int i = 0; i < numClients; i++) // Find and remove client from array
            {
                if (clientSockets[i] == clientSocket) // Check if current socket matches client
                {
                    for (int j = i; j < numClients - 1; j++) // Shift remaining clients down
                    {
                        clientSockets[j] = clientSockets[j + 1];
                    }
                    numClients--; // Decrement number of clients
                    break;
                }
            }
            pthread_mutex_unlock(&clientMutex); // Unlock the mutex

            close(clientSocket); // Close the client socket
            pthread_exit(NULL); // Terminate the thread
        }
        message[bytesReceived - 1] = '\0'; // Null-terminate the received message
        printf("Received message from client %d: %s\n", clientNumber, message); // Print received message
        fflush(stdout); // Flush stdout buffer

        // Forward the message to all other clients
        sendToAllClients(clientSocket, message, clientNumber);
    }
    return NULL; // Return NULL (not reached due to infinite loop)
}

int main()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Create a socket for the server
    if (serverSocket < 0) // Check for socket creation error
    {
        perror("Error creating socket"); // Print error message
        exit(EXIT_FAILURE); // Exit with failure status
    }

    struct sockaddr_in serverAddress; // Structure to store server address
    serverAddress.sin_family = AF_INET; // Set address family to Internet
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Allow any IP to connect
    serverAddress.sin_port = htons(8080); // Set port number (converts to network byte order)

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Bind socket to address
    {
        perror("Error binding socket"); // Print error message
        close(serverSocket); // Close the server socket
        exit(EXIT_FAILURE); // Exit with failure status
    }

    if (listen(serverSocket, 5) < 0) // Listen for incoming connections (backlog of 5)
    {
        perror("Error listening for connections"); // Print error message
        close(serverSocket); // Close the server socket
        exit(EXIT_FAILURE); // Exit with failure status
    }
    printf("Server is up and listening...\n"); // Print server start message
    while (1) // Infinite loop to accept incoming connections
    {
        int clientSocket = accept(serverSocket, NULL, NULL); // Accept a client connection

        printf("Accepted client %d\n", clientCounter + 1); // Print message indicating new client
        fflush(stdout); // Flush stdout buffer
        pthread_t thread; // Declare thread variable
        int *pClientSocket = malloc(sizeof(int)); // Allocate memory for client socket pointer
        *pClientSocket = clientSocket; // Set the value of pointer to client socket
        pthread_create(&thread, NULL, handleClient, pClientSocket); // Create a new thread for the client
    }
}
