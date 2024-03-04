#ifndef PROACTOR_H
#define PROACTOR_H

#include <stddef.h> // For size_t

// Structure for callback arguments
typedef struct {
    int socket;
    void *data; // Additional data that can be passed to the callback
    size_t data_len; // Length of the data
} ProactorEventInfo;

// Callback function type for handling socket operations
typedef void (*ProactorCallback)(ProactorEventInfo*);

// Initializes the proactor system
void initializeProactor();

// Cleans up resources used by the proactor system
void cleanupProactor();

// Registers a socket and its associated callback function
void registerSocket(int socket, ProactorCallback callback);

#endif // PROACTOR_H
