#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// The code creates a simple HTTP server that can handle GET and POST requests.

// Define constants for the server
#define PORT 8080  // The port number on which the server will listen.
#define MAX_PENDING_CONNECTIONS 10  // Maximum number of pending connections in the server's listening queue.
#define BUFFER_SIZE 2048  // Buffer size for reading and sending data.
#define PATH_SIZE 256  // Maximum length for the path of requested files.

// Function to handle GET requests
void handle_get_request(int client_socket, char *filepath) {
    // Try to open the requested file in binary read mode.
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        // If the file does not exist, inform the client with a 404 error message.
        send(client_socket, "404 FILE NOT FOUND\r\n\r\n", strlen("404 FILE NOT FOUND\r\n\r\n"), 0);
        close(client_socket);  // Close the client connection.
        return;
    } else {
        // If the file exists, send a 200 OK status to the client.
        send(client_socket, "200 OK\r\n\r\n", strlen("200 OK\r\n\r\n"), 0);

        // Read the file content and send it to the client in chunks.
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        while ((bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0) {
            if (send(client_socket, buffer, bytes_read, 0) == -1) {
                // If there is an error sending the file, log it and exit.
                perror("Send failed");
                close(client_socket);
                fclose(file);
                return;
            }
        }

        fclose(file);  // Close the file after sending it.
        close(client_socket);  // Close the client socket.
    }
}

// Function to handle POST requests
void handle_post_request(int client_socket, char *filename, char *data) {
    // Open (or create if not exists) the file in append mode to write data at the end.
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        // If there is an error opening the file, inform the client with a 500 error message.
        send(client_socket, "500 INTERNAL ERROR\r\n\r\n", strlen("500 INTERNAL ERROR\r\n\r\n"), 0);
        close(client_socket);
        return;
    } else {
        // Write the received data to the file.
        fwrite(data, strlen(data), 1, file);
        fclose(file);  // Close the file.

        // Inform the client that the data was received successfully with a 200 OK message.
        send(client_socket, "200 OK\r\n\r\n", strlen("200 OK\r\n\r\n"), 0);
        close(client_socket);  // Close the client socket.
    }
}

// Function to handle incoming client requests
void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // Receive the request from the client
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received == 0) {
        // If no data is received (client closed connection), close the socket and return.
        printf("Server: Client closed the connection.\n");
        close(client_socket);
        return;
    } else if (bytes_received < 0) {
        // If there is an error receiving data, log it, close the socket, and return.
        perror("Receive failed");
        close(client_socket);
        return;
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received data.

    // Extract the HTTP method and the file path from the received request.
    char method[10], path[PATH_SIZE];
    sscanf(buffer, "%s %s", method, path);

    // Print the received request for debugging.
    printf("Server: Received request: %s\n", buffer);

    // Construct the file path from the received path.
    char filepath[PATH_SIZE];
    strcpy(filepath, path);

    // Determine the type of request (GET or POST) and call the appropriate handler function.
    if (strcmp(method, "GET") == 0) {
        printf("Server: Handling GET request for file: %s\n", filepath);
        handle_get_request(client_socket, filepath);
    } else if (strcmp(method, "POST") == 0) {
        printf("Server: Handling POST request for file: %s\n", filepath);
        // Find the start of the data in the request.
        char *data_start = strstr(buffer, "\r\n\r\n") + strlen("\r\n\r\n");
        if (data_start == NULL) {
            // If no data is found, inform the client with a 400 error message and close the socket.
            send(client_socket, "400 BAD REQUEST\r\n\r\n", strlen("400 BAD REQUEST\r\n\r\n"), 0);
            close(client_socket);
            return;
        }
        handle_post_request(client_socket, filepath, data_start);
    } else {
        // If the method is neither GET nor POST, inform the client with a 400 error message and close the socket.
        send(client_socket, "400 BAD REQUEST\r\n\r\n", strlen("400 BAD REQUEST\r\n\r\n"), 0);
        close(client_socket);
        return;
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a socket for the server.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize the server address structure.
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;  // Use IPv4 addresses.
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Accept connections on any IP address of the machine.
    server_addr.sin_port = htons(PORT);  // Convert port number to network byte order.

    // Bind the socket to the server address.
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections.
    if (listen(server_socket, MAX_PENDING_CONNECTIONS) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server: Listening on port %d...\n", PORT);

    // Continuously accept and handle incoming connections.
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;  // Continue to the next iteration to accept new connections.
        }

        // Handle the request from the connected client.
        handle_request(client_socket);
    }

    close(server_socket);  // Close the server socket.

    return 0;
}

