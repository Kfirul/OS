#include <stdio.h>       // Include for standard input/output functions
#include <stdlib.h>      // Include for standard library functions
#include <string.h>      // Include for string manipulation functions
#include <unistd.h>      // Include for POSIX operating system API
#include <sys/types.h>   // Include for data types
#include <sys/socket.h>  // Include for socket functions
#include <netinet/in.h>  // Include for Internet address family
#include <netdb.h>       // Include for network database operations
#include <pthread.h>     // Include for POSIX threads

#define BUFFER_SIZE 1024 // Define buffer size for messages

// Function to monitor keyboard input and relay messages to the server
void* keyboardListener(void *pClientSocket)
{
    int clientSocket = *((int*)pClientSocket); // Cast and dereference pClientSocket to get the client socket
    char buffer[BUFFER_SIZE]; // Buffer for storing user input

    // Continuously scan keyboard input and dispatch to the server
    while(1) {
        bzero(buffer, BUFFER_SIZE); // Clear the buffer
        printf("Please input your message (or type SIGNOUT to leave):\n"); // Prompt user for input
        fflush(stdout); // Flush the stdout buffer to ensure prompt is displayed
        fgets(buffer, BUFFER_SIZE, stdin); // Read user input from stdin

        // Dispatch the input to the server
        if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
            perror("Error: Unable to dispatch message to server"); // Handle send error
            exit(EXIT_FAILURE); // Exit the program on error
        }

        // Check for user request to leave
        if (strncmp(buffer, "SIGNOUT", 7) == 0) {
            printf("Signing out...\n"); // Notify user of sign out
            close(clientSocket); // Close the client socket
            exit(0); // Exit the program
        }
    }

    return NULL; // Return NULL (standard for pthread functions)
}

int main()
{
    const char *serverAddress = "127.0.0.1"; // Define server address
    const char *serverPort = "8080"; // Define server port

    // Create a socket for the client
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Error: Unable to establish socket connection"); // Handle socket creation error
        exit(EXIT_FAILURE); // Exit the program on error
    }

    // Retrieve host information for the server address
    struct hostent *server = gethostbyname(serverAddress);
    if (server == NULL) {
        fprintf(stderr, "Error: Host not found\n"); // Handle host not found error
        close(clientSocket); // Close the client socket
        exit(EXIT_FAILURE); // Exit the program on error
    }

    struct sockaddr_in serverAddressInfo; // Declare structure for server address
    memset(&serverAddressInfo, 0, sizeof(serverAddressInfo)); // Clear the structure
    serverAddressInfo.sin_family = AF_INET; // Set address family to Internet
    bcopy((char *)server->h_addr, (char *)&serverAddressInfo.sin_addr.s_addr, server->h_length); // Copy server address to sockaddr_in structure
    serverAddressInfo.sin_port = htons(atoi(serverPort)); // Convert and set server port

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo)) < 0) {
        perror("Error: Unable to connect to server"); // Handle connection error
        close(clientSocket); // Close the client socket
        exit(EXIT_FAILURE); // Exit the program on error
    }

    // Initialize a thread to monitor keyboard input
    pthread_t thread; // Declare a thread
    pthread_create(&thread, NULL, keyboardListener, (void*)&clientSocket); // Create a thread to run keyboardListener

    // The primary thread listens for server messages
    char serverResponse[BUFFER_SIZE]; // Buffer for server response
    while(1) {
        bzero(serverResponse, BUFFER_SIZE); // Clear the buffer
        ssize_t bytesReceived = recv(clientSocket, serverResponse, BUFFER_SIZE, 0); // Receive message from server
        serverResponse[bytesReceived] ='\0'; // Null-terminate the response
        if (bytesReceived < 0) {
            perror("Error: Unable to read message from server"); // Handle recv error
            exit(EXIT_FAILURE); // Exit the program on error
        } else if (bytesReceived == 0) {
            printf("Server connection closed\n"); // Notify user that server connection is closed
            break; // Break the loop to end program
        }
        printf("%s\n", serverResponse); // Print the server response
        fflush(stdout); // Flush the stdout buffer
    }

    // Cleanup resources
    close(clientSocket); // Close the client socket
    pthread_cancel(thread); // Terminate the keyboard monitoring thread
    pthread_join(thread, NULL); // Wait for the thread to terminate

    return 0; // Return 0 to indicate successful execution
}
