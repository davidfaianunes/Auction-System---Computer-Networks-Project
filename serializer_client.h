#ifndef SERIALIZER_CLIENT_H
#define SERIALIZER_CLIENT_H

void getWords(char command[], char*** words, int* numWords);

void freeWords(char** words, int numWords);

int getDataAndSize(const char *path, char **data, char **size);

#endif