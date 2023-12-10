#ifndef SERIALIZER_CLIENT_H
#define SERIALIZER_CLIENT_H

void getWords(char command[], char*** words, int* numWords);

void freeWords(char** words, int numWords);

int getDataAndSize(const char *path, char **data, char **size);

// validators

int isValidUsername(char* username);

int isValidPassword(char* password);

int isValidAuctionName(char *auction_name);

int isValidAssetFilename(const char *filename);

int isValidAuctionStartValue(const char* startvalue);

int isValidAuctionTimeActive(const char* startvalue);

int isValidAID(const char* AID);

int isValidBidValue(const char* bid_value);

int isValidFileSize(char * file_size);

#endif