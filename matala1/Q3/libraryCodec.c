#include <stdio.h>
#include <stdlib.h>
#include "libraryCodec.h"
#include <string.h>

// Global array of characters for the key and encoded key
char globalKey[62] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
char keyEncode[62] = "cdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ab";

// Function to create a codec for encoding and decoding
void *createCodec(char key[62]) {
    // Ensure no duplicate characters in the key
    for (int i = 0; i < 62; i++) {
        for (int j = i + 1; j < 62; j++) {
            if (key[i] == key[j]) {
                printf("Duplicate found, code is incorrect!");
                return NULL;
            }
        }
    }

    // Allocate memory for the codec
    char* ans = (char*)malloc(125 * sizeof(char));
    if (!ans) {
        return NULL;
    }

    // Copy the globalKey and keyEncode to the codec
    memcpy(ans, globalKey, 62 * sizeof(char));
    memcpy(ans + 62, key, 62 * sizeof(char));
    ans[124] = '\0'; // Null-terminate the codec

    return ans;
}

// Function to encode text using the codec
int encode(char *textin, char *textout, int len, void *codec) {
    if (!textin || !textout || !codec) {
        return -1;
    }

    int count = 0;
    char *key = (char *)codec;

    // Encode each character in the input text
    for (int i = 0; i < len; i++) {
        int index = -1;
        for (int j = 0; j < 62; j++) {
            if (textin[i] == key[j]) {
                index = j;
                break;
            }
        }

        if (index != -1) {
            textout[i] = key[62 + index]; // Replace with encoded character
            count++;
        } else {
            textout[i] = textin[i]; // Keep character unchanged if not found in codec
        }
    }

    return count; // Return the count of encoded characters
}

// Helper function to find the index of a character in the codec
int findCharacterIndex(char textChar, char *key) {
    for (int j = 61; j < 124; j++) {
        if (textChar == key[j]) {
            return j - 62;
        }
    }
    return -1; // Return -1 if character is not found
}

// Function to decode text using the codec
int decode(char *textin, char *textout, int len, void *codec) {
    if (!textin || !textout || !codec) {
        return -1;
    }

    int count = 0;
    char *key = (char *)codec;

    // Decode each character in the input text
    for (int i = 0; i < len; i++) {
        int index = findCharacterIndex(textin[i], key);

        if (index != -1) {
            textout[i] = key[index]; // Replace with decoded character
            count++;
        } else {
            textout[i] = textin[i]; // Keep character unchanged if not found in codec
        }
    }

    return count; // Return the count of decoded characters
}

// Function to free the codec memory
void freeCodec(void *codec) {
    free(codec);
}
