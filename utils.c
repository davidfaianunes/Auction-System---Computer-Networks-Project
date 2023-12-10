#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "datasizes.h"

#define MAX_CLIENT_PATH_LEN 256


char* intToString(int num) {
    int digits = snprintf(NULL, 0, "%d", num);

    // Allocate memory for the string representation of the integer
    char *str = (char *)malloc(digits + 1); // +1 for the null terminator

    if (str == NULL) {
        // Handle memory allocation failure
        perror("Error allocating memory");
        exit(1);
    }

    // Convert the integer to a string
    snprintf(str, digits + 1, "%d", num);
    return str;
}

// "Clients/102890"
char* pathClientFolder(char* UID) {
    // Allocate memory for the path
    char *path = (char *)malloc(7 + 1 + UID_LEN + 1);

    if (path == NULL) {
        // Handle memory allocation failure
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Construct the path using intToString
    strcpy(path, "Clients/");
    strcat(path, UID);

    return path;
}

// "Clients/102890/main.txt"
char* pathClientMain(char* UID) {
    // Allocate memory for the path
    char *path = (char *)malloc(7 + 1 + UID_LEN + 1 + 8 + 1);

    if (path == NULL) {
        // Handle memory allocation failure
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Construct the path using intToString
    strcpy(path, "Clients/");
    strcat(path, UID);
    strcat(path, "/main.txt");

    return path;
}

// "Clients/102890/bidder.txt"
char* pathClientBidder(char* UID) {
    // Allocate memory for the path
    char *path = (char *)malloc(7 + 1 + UID_LEN + 1 + 10 + 1);

    if (path == NULL) {
        // Handle memory allocation failure
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Construct the path using intToString
    strcpy(path, "Clients/");
    strcat(path, UID);
    strcat(path, "/bidder.txt");

    return path;
}

// "Clients/102890/host.txt"
char* pathClientHost(char* UID) {
    // Allocate memory for the path
    char *path = (char *)malloc(7 + 1 + UID_LEN + 1 + 8 + 1);

    if (path == NULL) {
        // Handle memory allocation failure
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Construct the path using intToString
    strcpy(path, "Clients/");
    strcat(path, UID);
    strcat(path, "/host.txt");

    return path;
}






// "Auctions/auction1"
char* pathAuctionFolder(char* AID) {
    // Allocate memory for the path
    char *path = (char *)malloc(8 + 1 + AID_MAX_LEN + 1);

    if (path == NULL) {
        // Handle memory allocation failure
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Construct the path using intToString
    strcpy(path, "Auctions/");
    strcat(path, AID);

    return path;
}

// "Auctions/auction1/main.txt"
char* pathAuctionMain(char* AID) {
    // Allocate memory for the path
    char *path = (char *)malloc(8 + 1 + AID_MAX_LEN + 1 + 8 + 1);

    if (path == NULL) {
        // Handle memory allocation failure
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Construct the path using intToString
    strcpy(path, "Auctions/");
    strcat(path, AID);
    strcat(path, "/main.txt");

    return path;
}

// "Auctions/auction1/bids.txt"
char* pathAuctionBids(char* AID) {
    // Allocate memory for the path
    char *path = (char *)malloc(8 + 1 + AID_MAX_LEN + 1 + 10 + 1);

    if (path == NULL) {
        // Handle memory allocation failure
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Construct the path using intToString
    strcpy(path, "Auctions/");
    strcat(path, AID);
    strcat(path, "/bidder.txt");

    return path;
}

// "Auctions/auction1/end.txt"
char* pathAuctionEnd(char* AID) {
    // Allocate memory for the path
    char *path = (char *)malloc(8 + 1 + AID_MAX_LEN + 1 + 7 + 1);

    if (path == NULL) {
        // Handle memory allocation failure
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Construct the path using intToString
    strcpy(path, "Auctions/");
    strcat(path, AID);
    strcat(path, "/end.txt");

    return path;
}


// "Assets/asset1.txt"
char* pathAsset(char*asset_fname){
    // Allocate memory for the path
    char *path = (char *)malloc(6 + 1 + ASSET_FILENAME_MAX_LEN + 4 + 1);

    if (path == NULL) {
        // Handle memory allocation failure
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Construct the path using intToString
    strcpy(path, "Assets/");
    strcat(path, asset_fname);
    strcat(path, ".txt");

    return path;
}





char* getCurrentDateTime() {
    // Get current time
    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Format the time into a string
    char* datetime = (char*)malloc((DATE_TIME_FORMAT_LEN + 1) * sizeof(char)); // +1 for the null terminator

    if (datetime == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    strftime(datetime, DATE_TIME_FORMAT_LEN + 1, "%Y-%m-%d %H:%M:%S", timeinfo);

    return datetime;
}

char* computeTimeDifference(char* datetime1, char* datetime2) {
    // Parse datetime strings
    struct tm tm1, tm2;
    memset(&tm1, 0, sizeof(struct tm));
    memset(&tm2, 0, sizeof(struct tm));

    if (sscanf(datetime1, "%d-%d-%d %d:%d:%d",
               &tm1.tm_year, &tm1.tm_mon, &tm1.tm_mday,
               &tm1.tm_hour, &tm1.tm_min, &tm1.tm_sec) != 6 ||
        sscanf(datetime2, "%d-%d-%d %d:%d:%d",
               &tm2.tm_year, &tm2.tm_mon, &tm2.tm_mday,
               &tm2.tm_hour, &tm2.tm_min, &tm2.tm_sec) != 6) {
        perror("Error parsing datetime");
        return NULL;
    }

    // Adjust year and month values
    tm1.tm_year -= 1900;
    tm1.tm_mon -= 1;
    tm2.tm_year -= 1900;
    tm2.tm_mon -= 1;

    // Convert to time_t (seconds since epoch)
    time_t time1 = mktime(&tm1);
    time_t time2 = mktime(&tm2);

    if (time1 == -1 || time2 == -1) {
        perror("Error converting datetime to time_t");
        return NULL;
    }

    // Compute the difference in seconds
    long diff_seconds = labs((long)(time2 - time1));

    // Convert the result to string
    char* result = (char*)malloc(DATE_TIME_FORMAT_LEN + 1);  // Adjust the size as needed
    if (result != NULL) {
        snprintf(result, 20, "%ld", diff_seconds);
        return result;
    } else {
        perror("Error allocating memory");
        return NULL;
    }
}

// getters for Auctions

char* getAuctionHostUID(char* AID) {
    FILE* file = fopen(pathAuctionMain(AID), "r");
    if (file != NULL) {
        // Allocate memory for the buffer to store the content of the first line
        char* buffer = (char*)malloc(UID_LEN + 1); // Adjust the size as needed

        if (buffer != NULL) {
            // Read the first line into the buffer
            if (fgets(buffer, UID_LEN + 1, file) != NULL) {
                // Remove the newline character if present
                char* newline = strchr(buffer, '\n');
                if (newline != NULL) {
                    *newline = '\0';
                }

                // Close the file
                fclose(file);

                // Return the content of the first line
                return buffer;
            } else {
                perror("Error reading file");
            }

            // Free the allocated buffer if an error occurred
            free(buffer);
        } else {
            perror("Error allocating memory");
        }

        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}

char* getAuctionAssetFName(char* AID) {
    FILE* file = fopen(pathAuctionMain(AID), "r");
    if (file != NULL) {
        // Allocate memory for the buffer to store the content of the second line
        char* buffer = (char*)malloc(ASSET_FILENAME_MAX_LEN + 1); // Adjust the size as needed
        char* dummy = (char*)malloc(256);

        if (buffer != NULL) {
            // Skip the first line
            if (fgets(dummy, 256, file) != NULL) {
                // Read the second line into the buffer
                if (fgets(buffer, ASSET_FILENAME_MAX_LEN + 1, file) != NULL) {
                    // Remove the newline character if present
                    char* newline = strchr(buffer, '\n');
                    if (newline != NULL) {
                        *newline = '\0';
                    }

                    // Close the file
                    fclose(file);

                    // Return the content of the second line
                    return buffer;
                } else {
                    perror("Error reading file");
                }
            } else {
                perror("Error reading file");
            }

            // Free the allocated buffer if an error occurred
            free(buffer);
        } else {
            perror("Error allocating memory");
        }

        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}

char* getAuctionStartValue(char* AID) {
    FILE* file = fopen(pathAuctionMain(AID), "r");

    if (file != NULL) {
        // Allocate memory for the buffer to store the content of the third line
        char* buffer = (char*)malloc(AUCTION_START_VALUE_LEN + 1); // Adjust the size as needed
        char* dummy = (char*)malloc(256);

        // Skip the first two lines
        for (int i = 0; i < 2; ++i) {
            if (fgets(dummy, 256, file) == NULL) {
                // Error reading file or not enough lines
                perror("Error reading file or not enough lines");
                fclose(file);
                free(buffer);
                return NULL;
            }
        }

        // Read the content of the third line into the buffer
        if (fgets(buffer, AUCTION_START_VALUE_LEN + 1, file) != NULL) {
            // Remove the newline character if present
            char* newline = strchr(buffer, '\n');
            if (newline != NULL) {
                *newline = '\0';
            }

            // Close the file
            fclose(file);

            // Return the content of the third line
            return buffer;
        } else {
            perror("Error reading file");
        }

        // Free the allocated buffer if an error occurred
        free(buffer);
        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}

char* getAuctionStartDateTime(char* AID) {
    FILE* file = fopen(pathAuctionMain(AID), "r");

    if (file != NULL) {
        // Allocate memory for the buffer to store the content of the fourth line
        char* buffer = (char*)malloc(DATE_TIME_FORMAT_LEN + 1); // Adjust the size as needed
        char* dummy = (char*)malloc(256);

        if (buffer != NULL) {
            // Read the first three lines and discard them
            for (int i = 0; i < 3; ++i) {
                if (fgets(dummy, 256, file) == NULL) {
                    perror("Error reading file");
                    free(buffer);
                    fclose(file);
                    return NULL;
                }
            }

            // Read the fourth line into the buffer
            if (fgets(buffer, DATE_TIME_FORMAT_LEN + 1, file) != NULL) {
                // Remove the newline character if present
                char* newline = strchr(buffer, '\n');
                if (newline != NULL) {
                    *newline = '\0';
                }

                // Close the file
                fclose(file);

                // Return the content of the fourth line
                return buffer;
            } else {
                perror("Error reading file");
            }

            // Free the allocated buffer if an error occurred
            free(buffer);
        } else {
            perror("Error allocating memory");
        }

        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}

char* getAuctionTimeactive(char* AID) {
    FILE* file = fopen(pathAuctionMain(AID), "r");

    if (file != NULL) {
        // Allocate memory for the buffer to store the content of the fifth line
        char* buffer = (char*)malloc(AUCTION_TIME_ACTIVE_LEN + 1); // Adjust the size as needed
        char* dummy = (char*)malloc(256);

        // Read and discard the first four lines
        for (int i = 0; i < 4; ++i) {
            if (fgets(dummy, 256, file) == NULL) {
                perror("Error reading file");
                fclose(file);
                free(buffer);
                return NULL;
            }
        }

        // Read the content of the fifth line into the buffer
        if (fgets(buffer, AUCTION_TIME_ACTIVE_LEN + 1, file) != NULL) {
            // Remove the newline character if present
            char* newline = strchr(buffer, '\n');
            if (newline != NULL) {
                *newline = '\0';
            }

            // Close the file
            fclose(file);

            // Return the content of the fifth line
            return buffer;
        } else {
            perror("Error reading file");
        }

        // Free the allocated buffer if an error occurred
        free(buffer);
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}

char* getAuctionName(char* AID) {
    FILE* file = fopen(pathAuctionMain(AID), "r");

    if (file != NULL) {
        // Allocate memory for the buffer to store the content of the sixth line
        char* buffer = (char*)malloc(AUCTION_NAME_MAX_LEN + 1); // Adjust the size as needed
        char* dummy = (char*)malloc(256);

        if (buffer != NULL) {
            // Skip the first five lines
            for (int i = 0; i < 5; ++i) {
                if (fgets(dummy, 256, file) == NULL) {
                    // Error reading file or not enough lines
                    perror("Error reading file or not enough lines");
                    fclose(file);
                    free(buffer);
                    return NULL;
                }
            }

            // Read the content of the sixth line into the buffer
            if (fgets(buffer, AUCTION_NAME_MAX_LEN + 1, file) != NULL) {
                // Remove the newline character if present
                char* newline = strchr(buffer, '\n');
                if (newline != NULL) {
                    *newline = '\0';
                }

                // Close the file
                fclose(file);

                // Return the content of the sixth line
                return buffer;
            } else {
                perror("Error reading file");
            }

            // Free the allocated buffer if an error occurred
            free(buffer);
        } else {
            perror("Error allocating memory");
        }

        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}

char* getAuctionStatus(char* AID) {
    FILE* file = fopen(pathAuctionEnd(AID), "r");
    if (file != NULL) {

        // Allocate memory for the buffer to store the content of the first line
        char* buffer = (char*)malloc(2); // Adjust the size as needed

        if (buffer != NULL) {
            // Read the first line into the buffer
            if (fgets(buffer, 2, file) != NULL) {

                // Remove the newline character if present
                char* newline = strchr(buffer, '\n');
                if (newline != NULL) {
                    *newline = '\0';
                }

                // Close the file
                fclose(file);

                // Return the content of the first line
                return buffer;
            } else {
                perror("Error reading file");
            }

            // Free the allocated buffer if an error occurred
            free(buffer);
        } else {
            perror("Error allocating memory");
        }

        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}

char* getAuctionEndDatetime(char* AID)  {
    FILE* file = fopen(pathAuctionEnd(AID), "r");
    if (file != NULL) {
        // Allocate memory for the buffer to store the content of the second line
        char* buffer = (char*)malloc(DATE_TIME_FORMAT_LEN + 1); // Adjust the size as needed
        char* dummy = (char*)malloc(256);

        if (buffer != NULL) {
            // Skip the first line
            if (fgets(dummy, 256, file) != NULL) {
                // Read the second line into the buffer
                if (fgets(buffer, DATE_TIME_FORMAT_LEN + 1, file) != NULL) {
                    // Remove the newline character if present
                    char* newline = strchr(buffer, '\n');
                    if (newline != NULL) {
                        *newline = '\0';
                    }

                    // Close the file
                    fclose(file);

                    // Return the content of the second line
                    return buffer;
                } else {
                    perror("Error reading file");
                }
            } else {
                perror("Error reading file");
            }

            // Free the allocated buffer if an error occurred
            free(buffer);
        } else {
            perror("Error allocating memory");
        }

        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}

char* getAuctionRealDuration(char* AID) {
    FILE* file = fopen(pathAuctionEnd(AID), "r");
    if (file != NULL) {
        // Allocate memory for the buffer to store the content of the third line
        char* buffer = (char*)malloc(AUCTION_START_VALUE_LEN + 1); // Adjust the size as needed
        char* dummy = (char*)malloc(256);

        if (buffer != NULL) {
            // Skip the first two lines
            if (fgets(dummy, 256, file) != NULL && fgets(dummy, 256, file) != NULL) {
                // Read the third line into the buffer
                if (fgets(buffer, AUCTION_START_VALUE_LEN + 1, file) != NULL) {
                    // Remove the newline character if present
                    char* newline = strchr(buffer, '\n');
                    if (newline != NULL) {
                        *newline = '\0';
                    }

                    // Close the file
                    fclose(file);

                    // Return the content of the third line
                    return buffer;
                } else {
                    perror("Error reading file");
                }
            } else {
                perror("Error reading file");
            }

            // Free the allocated buffer if an error occurred
            free(buffer);
        } else {
            perror("Error allocating memory");
        }

        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}


// getters for Client

char* getClientPassword(char* UID) {
    FILE* file = fopen(pathClientMain(UID), "r");
    if (file != NULL) {
        // Allocate memory for the buffer to store the content of the first line
        char* buffer = (char*)malloc(USER_PASSWORD_LEN + 1); // Adjust the size as needed

        if (buffer != NULL) {
            // Read the first line into the buffer
            if (fgets(buffer, USER_PASSWORD_LEN + 1, file) != NULL) {
                // Remove the newline character if present
                char* newline = strchr(buffer, '\n');
                if (newline != NULL) {
                    *newline = '\0';
                }

                // Close the file
                fclose(file);

                // Return the content of the first line
                return buffer;
            } else {
                perror("Error reading file");
            }

            // Free the allocated buffer if an error occurred
            free(buffer);
        } else {
            perror("Error allocating memory");
        }

        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}

// bid validation

int getLastBidValue(char* AID) {
    FILE* file = fopen(pathAuctionBids(AID), "r");

    if (file != NULL) {
        // Seek to the end of the file
        fseek(file, 0, SEEK_END);

        // Check if the file is empty
        long fileSize = ftell(file);
        if (fileSize == 0) {
            fclose(file);
            return atoi(getAuctionStartValue(AID)) - 1;
        }

        // Find the position of the penultimate newline character
        long position = fileSize - 1;
        while (position > 0) {
            fseek(file, --position, SEEK_SET);
            if (fgetc(file) == '\n') {
                break;
            }
        }
        while (position > 0) {
            fseek(file, --position, SEEK_SET);
            if (fgetc(file) == '\n') {
                break;
            }
        }
        while (position > 0) {
            fseek(file, --position, SEEK_SET);
            if (fgetc(file) == '\n') {
                break;
            }
        }
        position++;

        // Read line
        char buffer[BID_VALUE_MAX_LEN + 1];
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            // Remove the newline character if present
            char* newline = strchr(buffer, '\n');
            if (newline != NULL) {
                *newline = '\0';
            }

            // Close the file
            fclose(file);

            // Return the content of the penultimate line
            return atoi(strdup(buffer));
        } else {
            // Error reading the line
            fclose(file);
            return atoi(getAuctionStartValue(AID)) - 1;
        }
    } else {
        perror("Error opening file");
        return atoi(getAuctionStartValue(AID)) - 1;
    }
}

int validateBid(char* AID, char* value){
    return atoi(value)>getLastBidValue(AID);
}


// getters for Assets

char* getAssetSize(char* AID) {
    FILE* file = fopen(pathAsset(getAuctionAssetFName(AID)), "r");
    if (file != NULL) {
        // Allocate memory for the buffer to store the content of the first line
        char* buffer = (char*)malloc(ASSET_SIZE_MAX_LEN + 1); // Adjust the size as needed

        if (buffer != NULL) {
            // Read the first line into the buffer
            if (fgets(buffer, ASSET_SIZE_MAX_LEN + 1, file) != NULL) {
                // Remove the newline character if present
                char* newline = strchr(buffer, '\n');
                if (newline != NULL) {
                    *newline = '\0';
                }

                // Close the file
                fclose(file);

                // Return the content of the first line
                return buffer;
            } else {
                perror("Error reading file");
            }

            // Free the allocated buffer if an error occurred
            free(buffer);
        } else {
            perror("Error allocating memory");
        }

        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}


char* getAssetData(char* AID) {
    FILE* file = fopen(pathAsset(getAuctionAssetFName(AID)), "r");
    
    if (file != NULL) {
        // Skip the first line
        char buffer[256]; // Adjust the size as needed
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            // Calculate the length of the content after skipping the first line
            size_t contentLength = 0;
            while (fgets(buffer, sizeof(buffer), file) != NULL) {
                contentLength += strlen(buffer);
            }

            // Allocate memory for the entire content after skipping the first line
            char* content = (char*)malloc(contentLength + 1); // Add 1 for the null terminator
            
            if (content != NULL) {
                // Reset the file pointer to the beginning
                fseek(file, SEEK_SET, 0);

                // Skip the first line again
                fgets(buffer, sizeof(buffer), file);

                // Read the remaining content into the allocated buffer
                while (fgets(buffer, sizeof(buffer), file) != NULL) {
                    strcat(content, buffer);
                }

                // Close the file
                fclose(file);

                // Return the content after skipping the first line
                return content;
            } else {
                perror("Error allocating memory");
            }
        } else {
            perror("Error reading file");
        }

        // Close the file
        fclose(file);
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return NULL;
}

char* getBidsInfo(char* AID) {
    FILE* file = fopen(pathAuctionBids(AID), "r");
    int line_counter = 0;
    if (file != NULL) {

        char* buffer = (char*)malloc((999*(1 + 1 + UID_LEN + 1 + BID_VALUE_MAX_LEN + 1 + DATE_TIME_FORMAT_LEN + 1 + DURATION_LEN) + 1 + 1 + DATE_TIME_FORMAT_LEN + 1 + DURATION_LEN + 1 + 1)*sizeof(char));  // +1 for the null terminator


        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);


        if (fileSize > 0) {
            // Allocate memory to store the content of the file
            if (buffer != NULL) {
                int c;
                size_t index = 0;
                int last_char_was_b;

                buffer[index++] = 'B';
                buffer[index++] = ' ';

                // Read char by char
                while ((c = fgetc(file)) != EOF) {
                    last_char_was_b = 0;
                    // Replace newline characters with spaces
                    if (c == '\n') {
                        line_counter++;
                        if(line_counter%4==0){
                            buffer[index++] = ' ';
                            buffer[index++] = 'B';
                            buffer[index++] = ' ';
                            last_char_was_b = 1;
                        }else{
                            buffer[index++] = ' ';
                        }
                    } else {
                        buffer[index++] = (char)c;
                    }
                }
                if(last_char_was_b){
                    index-=3;
                    buffer[index++] = '\0';
                    buffer[index++] = '\0';
                    buffer[index] = '\0';
                }else{
                    buffer[index] = '\0';
                }

                // Close the file
                fclose(file);

                return buffer;
            } else {
                perror("Error allocating memory");
            }
        } else {
            // File is empty
            fclose(file);
            return "";
        }
    } else {
        perror("Error opening file");
    }

    // Return NULL if an error occurred
    return "";
}
