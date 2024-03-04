#include <stdio.h>
#include <stdlib.h>
#include "libraryCodec.h"

// Function to check if a file exists
int fileExists(const char *filename) {
    FILE *file = fopen(filename, "r"); // Try to open the file in read mode
    if (file) {
        fclose(file); // Close the file if it exists
        return 1; // Return 1 to indicate file exists
    }
    return 0; // Return 0 to indicate file does not exist
}

// Function to read the content of a file into memory
char *readFile(const char *filename, long *fileSize) {
    FILE *file = fopen(filename, "r"); // Open the file in read mode
    if (!file) {
        perror("Error opening file"); // Print error if file cannot be opened
        return NULL;
    }

    fseek(file, 0, SEEK_END); // Go to the end of the file
    *fileSize = ftell(file); // Get the size of the file
    fseek(file, 0, SEEK_SET); // Reset the file pointer to the beginning

    char *content = (char *)malloc(*fileSize + 1); // Allocate memory for the file content
    if (!content) {
        perror("Error allocating memory"); // Print error if memory allocation fails
        fclose(file); // Close the file
        return NULL;
    }

    fread(content, 1, *fileSize, file); // Read the content of the file
    content[*fileSize] = '\0'; // Null-terminate the content
    fclose(file); // Close the file
    return content; // Return the file content
}

// Function to write content to a file
int writeFile(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w"); // Open the file in write mode
    if (!file) {
        perror("Error opening file for writing"); // Print error if file cannot be opened
        return 0;
    }

    fputs(content, file); // Write the content to the file
    fclose(file); // Close the file
    return 1; // Return 1 to indicate success
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <source_file> <destination_file>\n", argv[0]); // Print usage instructions
        return 1;
    }

    if (!fileExists(argv[1])) {
        printf("Source file does not exist.\n"); // Check if source file exists
        return 1;
    }

    void *cipher = createCodec(keyEncode); // Create a cipher using the keyEncode function
    if (!cipher) {
        printf("Failed to create cipher!\n"); // Print error if cipher creation fails
        return 1;
    }

    long fileSize;
    char *fileContent = readFile(argv[1], &fileSize); // Read the content of the source file
    if (!fileContent) {
        printf("Failed to read file.\n"); // Print error if file read fails
        freeCodec(cipher); // Free the codec
        return 1;
    }

    char *decodedContent = (char *)malloc(fileSize + 1); // Allocate memory for the decoded content
    if (!decodedContent) {
        perror("Error allocating memory"); // Print error if memory allocation fails
        free(fileContent); // Free the file content
        freeCodec(cipher); // Free the codec
        return 1;
    }

    decode(fileContent, decodedContent, fileSize, cipher); // Decode the file content

    if (!writeFile(argv[2], decodedContent)) { // Write the decoded content to the destination file
        printf("Failed to write to destination file.\n"); // Print error if write fails
        free(fileContent); // Free the file content
        free(decodedContent); // Free the decoded content
        freeCodec(cipher); // Free the codec
        return 1;
    }

    // Clean up resources
    free(fileContent);
    free(decodedContent);
    freeCodec(cipher);
    printf("Decoding successful.\n"); // Print success message
    return 0;
}
