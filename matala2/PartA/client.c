#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"  // Define server IP address as localhost
#define PORT 8080  // Define the port number for the server connection
#define BUFFER_SIZE 1024  // Define the size of the buffer for data transfer

// Function to handle client-side operations for connecting to a server and sending data
void handle_client(char *filename, char *request_type) {
    int client_socket; // Socket descriptor for the client
    struct sockaddr_in server_addr; // Struct to hold server address information
    char buffer[BUFFER_SIZE]; // Buffer for storing data to be sent/received

    // Create a socket for the client
    // AF_INET: Address family (IPv4)
    // SOCK_STREAM: Socket type (TCP)
    // 0: Default protocol (TCP)
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed"); // Error handling if socket creation fails
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr)); // Clear structure
    server_addr.sin_family = AF_INET; // Set address family to IPv4
    server_addr.sin_port = htons(PORT); // Set port number, converting to network byte order
    // Convert the IP address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported"); // Error handling if IP address conversion fails
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    printf("Client: Trying to connect to server...\n");
    // Establish a connection to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed"); // Error handling if connection fails
        close(client_socket); // Close socket
        exit(EXIT_FAILURE);
    }
    printf("Client: Connection to the server established!\n");

    // Send a request to the server
    printf("Client: Initiating request transmission...\n");
    // Format the request string and send it to the server
    sprintf(buffer, "%s %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %lu\r\n\r\n", request_type, filename, SERVER_IP, strlen(filename));
    if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
        perror("Send failed"); // Error handling if sending fails
        close(client_socket); // Close socket
        exit(EXIT_FAILURE);
    }

    // Open the file to be sent
    FILE *file = fopen(filename, "rb"); // Open file in binary read mode
    if (file == NULL) {
        perror("Error opening file"); // Error handling if file opening fails
        close(client_socket); // Close socket
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END); // Go to the end of the file
    long file_size = ftell(file); // Get the size of the file
    rewind(file); // Go back to the beginning of the file
    char *file_data = (char *)malloc(file_size); // Allocate memory for file data
    fread(file_data, sizeof(char), file_size, file); // Read file data into memory
    fclose(file); // Close the file

    // Send file data to the server
    if (send(client_socket, file_data, file_size, 0) == -1) {
        perror("Send failed"); // Error handling if sending file data fails
        close(client_socket); // Close socket
        free(file_data); // Free allocated memory for file data
        exit(EXIT_FAILURE);
    }
    printf("Client: File data sent successfully!\n");

    // Receive response from server
    printf("Client: Receiving response from server...\n");
    int bytes_received;
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, sizeof(char), bytes_received, stdout); // Write the received data to stdout
    }
    if (bytes_received == -1) {
        perror("Receive failed"); // Error handling if receiving data fails
    }

    // Clean up
    free(file_data); // Free allocated memory for file data
    close(client_socket); // Close the client socket
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <request_type>\n", argv[0]); // Check for correct usage
        exit(EXIT_FAILURE);
    }

    char *filename = argv[1]; // Filename from command line argument
    char *request_type = argv[2]; // Request type from command line argument

    // Check if the specified file exists
    if (access(filename, F_OK) == -1) {
        fprintf(stderr, "Error: File '%s' does not exist.\n", filename); // Error handling if file does not exist
        exit(EXIT_FAILURE);
    } else {
        printf("File '%s' exists.\n", filename);
    }

    // Execute client operations
    handle_client(filename, request_type);

    return 0;
}
