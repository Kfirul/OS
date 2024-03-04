#include <stdio.h>
#include <stdlib.h>
#include "libraryCodec.h"

// Function to check if a file exists
int fileExists(const char *filename) {
    FILE *file = fopen(filename, "r"); // Open the file in read mode
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

    fseek(file, 0, SEEK_END); // Move to the end of the file to determine its size
    *fileSize = ftell(file); // Get the size of the file
    fseek(file, 0, SEEK_SET); // Reset the file pointer to the beginning

    char *content = (char *)malloc(*fileSize + 1); // Allocate memory for the file content
    if (!content) {
        perror("Error allocating memory"); // Print error if memory allocation fails
        fclose(file); // Close the file
        return NULL;
    }

    if (fread(content, 1, *fileSize, file) != *fileSize) {
        perror("Error reading file"); // Print error if file read fails
        free(content); // Free the allocated memory for content
        fclose(file); // Close the file
        return NULL;
    }

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

    if (fputs(content, file) == EOF) { // Write content to file and check for errors
        perror("Error writing to file"); // Print error if write operation fails
        fclose(file); // Close the file
        return 0;
    }

    fclose(file); // Close the file
    return 1; // Return 1 to indicate success
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <source_file> <destination_file>\n", argv[0]); // Print usage instructions
        return 1;
    }

    // Check if source file exists
    if (!fileExists(argv[1])) {
        printf("Source file '%s' does not exist.\n", argv[1]); // Print error if source file does not exist
        return 1;
    }

    // Create codec for encoding
    void *codec = createCodec(keyEncode);
    if (!codec) {
        printf("Failed to create codec!\n"); // Print error if codec creation fails
        return 1;
    }

    long fileSize;
    char *fileContent = readFile(argv[1], &fileSize); // Read file content
    if (!fileContent) {
        printf("Failed to read file '%s'.\n", argv[1]); // Print error if file read fails
        freeCodec(codec); // Free the codec
        return 1;
    }

    char *encodedContent = (char *)malloc(fileSize + 1); // Allocate memory for encoded content
    if (!encodedContent) {
        perror("Error allocating memory"); // Print error if memory allocation fails
        free(fileContent); // Free the original file content
        freeCodec(codec); // Free the codec
        return 1;
    }

    encode(fileContent, encodedContent, fileSize, codec); // Encode file content

    // Write the encoded content to the destination file
    if (!writeFile(argv[2], encodedContent)) {
        printf("Failed to write to destination file '%s'.\n", argv[2]); // Print error if write operation fails
        free(fileContent); // Free the original file content
        free(encodedContent); // Free the encoded content
        freeCodec(codec); // Free the codec
        return 1;
    }

    // Free allocated memory and close codec
    free(fileContent); // Free the original file content
    free(encodedContent); // Free the encoded content
    freeCodec(codec); // Free the codec

    printf("Encoding successful.\n"); // Print success message
    return 0;
}
