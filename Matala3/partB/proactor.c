#include "proactor.h"    // Include the proactor header for Proactor pattern
#include <pthread.h>     // Include for POSIX threads
#include <stdio.h>       // Include for standard input/output functions
#include <stdlib.h>      // Include for standard library functions like malloc
#include <unistd.h>      // Include for POSIX API like close function

// Structure representing a node in the linked list for socket descriptors
typedef struct SocketNode {
    int socketDescriptor;           // Socket descriptor
    ProactorCallback callbackFunction; // Callback function for the socket
    struct SocketNode* nextNode;    // Pointer to the next node in the list
} SocketNode;

static SocketNode* headNode = NULL; // Head node of the linked list
static pthread_mutex_t socketListMutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread-safe operations on the list

// Helper function to find and remove a socket node from the list
static SocketNode* extractSocketNode(int socket) {
    SocketNode *currentNode = headNode, *previousNode = NULL;
    // Iterate through the list to find the node
    while (currentNode != NULL && currentNode->socketDescriptor != socket) {
        previousNode = currentNode;
        currentNode = currentNode->nextNode;
    }
    // If node not found, return NULL
    if (currentNode == NULL) return NULL;
    // Remove the node from the list
    if (previousNode == NULL) headNode = currentNode->nextNode;
    else previousNode->nextNode = currentNode->nextNode;
    return currentNode; // Return the removed node
}

// Thread function to process socket data
void* processSocketThread(void* arg) {
    int socket = *(int*)arg; // Extract the socket descriptor from the argument
    ProactorCallback assignedCallback = NULL;

    // Lock the mutex to safely modify the linked list
    pthread_mutex_lock(&socketListMutex);
    // Extract the node for the given socket
    SocketNode* node = extractSocketNode(socket);
    if (node != NULL) {
        // Get the callback function assigned to the socket
        assignedCallback = node->callbackFunction;
        free(node); // Free the memory of the node
    }
    // Unlock the mutex
    pthread_mutex_unlock(&socketListMutex);

    // If a callback is assigned, call it
    if (assignedCallback != NULL) {
        assignedCallback(socket);
    }
    close(socket); // Close the socket
    return NULL; // End the thread
}

// Function to initialize the proactor
void initializeProactor() {
    // Initialize the mutex for thread safety
    pthread_mutex_init(&socketListMutex, NULL);
}

// Function to clean up resources used by the proactor
void cleanupProactor() {
    // Destroy the mutex
    pthread_mutex_destroy(&socketListMutex);
}

// Function to register a socket and its callback
void registerSocket(int socket, ProactorCallback callback) {
    // Allocate memory for a new SocketNode
    SocketNode* newNode = (SocketNode*)malloc(sizeof(SocketNode));
    // Initialize the new node with socket descriptor and callback
    newNode->socketDescriptor = socket;
    newNode->callbackFunction = callback;
    newNode->nextNode = NULL;

    // Lock the mutex to safely modify the linked list
    pthread_mutex_lock(&socketListMutex);
    // Insert the new node at the head of the list
    newNode->nextNode = headNode;
    headNode = newNode;
    // Unlock the mutex
    pthread_mutex_unlock(&socketListMutex);

    // Create a new thread to process the socket
    pthread_t thread;
    if (pthread_create(&thread, NULL, processSocketThread, &newNode->socketDescriptor) != 0) {
        perror("Error creating thread"); // Handle thread creation error
        // Safely remove the node if thread creation fails
        pthread_mutex_lock(&socketListMutex);
        extractSocketNode(socket);
        pthread_mutex_unlock(&socketListMutex);
        close(socket); // Close the socket
    } else {
        pthread_detach(thread); // Detach the thread
    }
}
