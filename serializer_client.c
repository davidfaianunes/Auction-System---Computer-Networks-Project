#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

void getWords(char command[], char*** words, int* numWords) {
    // Split string using space as delimiter
    char* token = strtok(command, " ");
    
    int count = 0;
    int capacity = 5;  // Initial capacity for words array, adjusted dynamically

    // Allocate memory for the words array
    *words = (char**)malloc(capacity * sizeof(char*));

    // Loop through tokens and store them in the words array
    while (token != NULL) {
        // Check if we need to reallocate memory for the words array
        if (count == capacity) {
            capacity += 3;  // Increase the capacity
            *words = (char**)realloc(*words, capacity * sizeof(char*));
        }

        // Allocate memory for the current token
        (*words)[count] = (char*)malloc((strlen(token) + 1) * sizeof(char));
        
        // Copy the token into the allocated memory
        strcpy((*words)[count], token);

        // Check and remove newline character if it exists
        size_t len = strlen((*words)[count]);
        if (len > 0 && (*words)[count][len - 1] == '\n') {
            (*words)[count][len - 1] = '\0';  // Remove the newline character
        }

        count++;
        token = strtok(NULL, " ");
    }

    // Set the number of words
    *numWords = count;
}

void freeWords(char** words, int numWords) {
    // Free the memory allocated for each word
    for (int i = 0; i < numWords; i++) {
        free(words[i]);
    }

    // Free the memory allocated for the words array
    free(words);
}

int getDataAndSize(const char *path, char **data, char **size) {
    // Open the file in binary mode
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);  // Reset the file position to the beginning

    // Allocate memory to store the file contents
    char *buffer = (char *)malloc(fileSize);
    if (buffer == NULL) {
        perror("Error allocating memory for data");
        fclose(file);
        return 1;
    }

    // Read the file into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);

    if (bytesRead != fileSize) {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return 1;
    }

    // Convert fileSize to a string
    char *fileSizeString = (char *)malloc(20);  // Assuming a reasonable limit for the size string
    if (fileSizeString == NULL) {
        perror("Error allocating memory for size");
        free(buffer);
        fclose(file);
        return 1;
    }

    sprintf(fileSizeString, "%ld", fileSize);

    // Assign the values to the output pointers
    *data = buffer;
    *size = fileSizeString;

    // Close the file
    fclose(file);

    return 0;
}