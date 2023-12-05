#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "utils.h"
#include "file_management.h"
#include "serializer_client.h"

int main() {
    char *data;
    char *size;

    getDataAndSize("testFileAsset.txt", &data, &size);

    printf("%s\n%s\n", data, size);

    // Don't forget to free the allocated memory
    free(data);
    free(size);

    return 0;
}