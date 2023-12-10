#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include "datasizes.h"

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
    char *fileSizeString = (char *)malloc(FILE_SIZE_MAX_LEN + 1);  // Assuming a reasonable limit for the size string
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

// util function

int isMadeUpOfDigits(const char* str) {
    while (*str != '\0') {
        if (!isdigit(*str)) {
            return 0;  // Not made up of digits
        }
        str++;
    }
    return 1;  // Made up of digits
}

int isAlphanumeric(const char* str) {
    while (*str != '\0') {
        if (!isalnum(*str)) {
            return 0;  // Not made up of digits
        }
        str++;
    }
    return 1;  // Made up of digits
}

// validation functions

int isValidUsername(char* username){
    return isMadeUpOfDigits(username) && strlen(username) == UID_LEN;
}

int isValidPassword(char* password){
    return isAlphanumeric(password) && strlen(password) == USER_PASSWORD_LEN;
}

int isValidAuctionName(char *auction_name) {
    return strlen(auction_name) <= AUCTION_NAME_MAX_LEN && isAlphanumeric(auction_name);
}

int isValidAssetFilename(const char *filename) {
    // check total length
    if (strlen(filename) > 24) {
        return 0; // Invalid
    }

    // check if it ends with a 3-letter extension
    size_t len = strlen(filename);
    if (len < 4 || filename[len - 4] != '.') {
        return 0; // Invalid
    }

    // check if extension contains only alphabets
    for (int i = len - 3; i < len; ++i) {
        if (!isalpha(filename[i])) {
            return 0; // Invalid
        }
    }

    // check if the rest of the filename contains only alphanumerical, '-', '_', and '.'
    for (int i = 0; i < len - 4; ++i) {
        if (!isalnum(filename[i]) && filename[i] != '-' && filename[i] != '_' && filename[i] != '.') {
            return 0; // Invalid
        }
    }

    return 1;
}

int isValidAuctionStartValue(const char* startvalue){
    return strlen(startvalue) == AUCTION_START_VALUE_LEN && isMadeUpOfDigits(startvalue);
}

int isValidAuctionTimeActive(const char* startvalue){
    return strlen(startvalue) == AUCTION_TIME_ACTIVE_LEN && isMadeUpOfDigits(startvalue);
}

int isValidAID(const char* AID){
    return strlen(AID) == AID_MAX_LEN && isMadeUpOfDigits(AID);
}

int isValidBidValue(const char* bid_value){
    return strlen(bid_value) <=BID_VALUE_MAX_LEN && isMadeUpOfDigits(bid_value);
}

int isValidFileSize(char * file_size){
    if(strlen(file_size)<=FILE_SIZE_MAX_LEN && isMadeUpOfDigits(file_size)){
        return atoi(file_size);
    }else{
        return 0;
    }
}
