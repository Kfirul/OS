#ifndef PROACTOR_H
#define PROACTOR_H

// Callback function type for handling socket operations
typedef void (*ProactorCallback)(int);

// Initializes the proactor system
void initializeProactor();

// Cleans up resources used by the proactor system
void cleanupProactor();

// Registers a socket and its associated callback function
void registerSocket(int socket, ProactorCallback callback);

#endif // PROACTOR_H
