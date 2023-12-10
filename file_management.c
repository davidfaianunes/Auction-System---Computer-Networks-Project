#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "utils.h"
#include "datasizes.h"
#include "file_management.h"
#include "serializer_client.h"


void checkIfClosingAuction(char* AID){
    if(strcmp(getAuctionStatus(AID),"0")){ //if auction is not yet closed, check if it's time to do so
        if(atoi(getAuctionTimeactive(AID)) <= (atoi(computeTimeDifference(getCurrentDateTime(), getAuctionStartDateTime(AID))))){
            closeAuction(AID);
        }
    }
}

void setupWorkspace() {
    // Create or open the aid_counter.txt file in read mode
    FILE *file = fopen("aid_counter.txt", "r");

    if (file == NULL) {
        // File doesn't exist, create it
        file = fopen("aid_counter.txt", "w");
        if (file == NULL) {
            perror("Error opening file");
            return;
        }

        // Write "000" to the file
        fprintf(file, "000");

        // Close the file
        fclose(file);
    } else {
        // File already exists, close it
        fclose(file);
    }

    // Continue with the rest of the directory setup...
    if (access("Clients", F_OK) != 0) {
        if (mkdir("Clients", 0755)) {
            perror("Error creating directory");
            return;
        }
    }
    if (access("Auctions", F_OK) != 0) {
        if (mkdir("Auctions", 0755)) {
            perror("Error creating directory");
            return;
        }
    }
    if (access("Assets", F_OK) != 0) {
        if (mkdir("Assets", 0755)) {
            perror("Error creating directory");
            return;
        }
    }
}



// gets next AID, and stores newest AID for later reference
char* getNextAID() {
    // Open the file for reading and writing
    FILE *file = fopen("aid_counter.txt", "r+");
    if (file == NULL) {
        perror("Error opening aid_counter.txt");
        return NULL;
    }

    // Read the current AID value from the file
    char currentAID[AID_MAX_LEN + 1]; // 3 digits + null terminator
    fgets(currentAID, sizeof(currentAID), file);

    // Check if the current AID is "end"
    if (strcmp(currentAID, "end") == 0) {
        // Maximum number of auctions created
        fclose(file);
        return NULL;
    }

    // Convert the current AID to an integer
    int aidValue = atoi(currentAID);

    // Check if the current value is the maximum
    if (aidValue == MAX_AID_VALUE) {
        fseek(file, 0, SEEK_SET);
        fprintf(file, "end");
    } else{
        // Increment the AID value
        int next_aidValue = aidValue + 1;

        // Write the new AID value back to the file
        fseek(file, 0, SEEK_SET);
        fprintf(file, "%03d", next_aidValue);
    }


    // Close the file
    fclose(file);

    // Allocate memory for the result and convert the new AID value to a string
    char *result = (char *)malloc(AID_MAX_LEN + 1);
    sprintf(result, "%03d", aidValue);

    return result;
}

int checkCredentials(char* UID, char* password){
    /*
    0 -> UNR (register ou não faz nada)
    1-> OK (password bem -> login, logout)
    2-> NOK(password mal -> não faz nada, não faz nada)
    */


    char* path = pathClientMain(UID);
    if(access(path, F_OK) != 0){ // client isnt registered
        free(path);
        return 0;

    } else{
        char* true_password = getClientPassword(UID);

        if(!strcmp(true_password, password)){ // client is registered, and password is correct
            free(path);
            free(true_password);
            return 1;

        } else{                                // client is registered, but incorrect password
            free(path);
            free(true_password);
            return 2;
        }
    }
}

int auctionIsOwnedBy(char*UID, char* AID){
    return !strcmp(getAuctionHostUID(AID), UID);
}

int auctionExists(char* AID){
    /*
    0 -> UNR (register ou não faz nada)
    1-> OK (password bem -> login, logout)
    2-> NOK(password mal -> não faz nada, não faz nada)
    */


    char* path = pathAuctionMain(AID);
    if(access(path, F_OK) != 0){ // auction doesnt exist
        free(path);
        return 0;

    } else{ // auction exists
        return 1;
    }
}

void registerUser(char* UID, char* password) {
    if (access(pathClientFolder(UID), F_OK) != 0) {
        if (mkdir(pathClientFolder(UID), 0755)) {
            perror("Error creating directory");
            return;
        }
    }
    char* path = pathClientMain(UID);
    FILE* file = fopen(path, "w");

    if (file != NULL) {
        // Write the content to the file
        fprintf(file, "%s", password);

        // Close the file
        fclose(file);

        // File written successfully
    } else {
        perror("Error opening file");
        return;
    }

    file = fopen(pathClientBidder(UID), "w");
    if (file == NULL) {
        perror("Error creating file");
        return;
    }

    // Close the file
    fclose(file);

    file = fopen(pathClientHost(UID), "w");
    if (file == NULL) {
        perror("Error creating file");
        return;
    }

    // Close the file
    fclose(file);

    free(path); // Free the allocated memory
}

void unregisterUser(char* UID) {
    // Delete main file
    if (remove(pathClientMain(UID)) != 0) {
        perror("Error deleting main file");
        exit(EXIT_FAILURE);
    }

    // Delete bidder file
    if (remove(pathClientBidder(UID)) != 0) {
        perror("Error deleting bidder file");
        exit(EXIT_FAILURE);
    }

    // Delete host file
    if (remove(pathClientHost(UID)) != 0) {
        perror("Error deleting host file");
        exit(EXIT_FAILURE);
    }

    // Delete the folder
    if (rmdir(pathClientFolder(UID)) != 0) {
        perror("Error deleting folder");
        exit(EXIT_FAILURE);
    }
}

char* listMyAuctions(char* UID) {
    FILE* file = fopen(pathClientHost(UID), "r");
    
    if (file != NULL) {
        int character;
        int first_iteration = 1;

        char* output = malloc(256 * sizeof(char));

        if (output != NULL) {
            // Initialize the output string
            strcpy(output, "RMA OK");

            do {
                // Read a line at a time
                char* line = malloc(256 * sizeof(char)); // Adjust size as needed

                if (line != NULL) {
                    int index = 0;

                    do {
                        // Check if it's a newline character
                        character = fgetc(file);

                        if(first_iteration){
                            if(character == EOF){
                                return "RMA NOK";
                            }
                            first_iteration = 0;
                        }

                        // Check if it's a newline character
                        if (character == '\n' || character == EOF) {
                            break;  // Exit the inner loop on newline or end of file
                        }

                        // Append the character to the line
                        line[index++] = character;

                    } while (index < 255);

                    // Null-terminate the line
                    line[index] = '\0';

                    // Concatenate the line to the output string
                    strcat(output, " ");
                    strcat(output, line);
                    strcat(output, " ");
                    if(!strcmp("", line)){
                        break; //ignore empty strings
                    }
                    strcat(output, getAuctionStatus(line));

                    // Free the memory allocated for the line
                    free(line);
                } else {
                    perror("Error allocating memory");
                    free(output);
                    break;  // Exit the outer loop if memory allocation fails
                }

            } while (character != EOF);

            // Print or process the final output string
            fclose(file);
            return output;

        } else {
            perror("Error allocating memory");
            return NULL;
        }
    } else {
        perror("Error opening file");
        return NULL;
    }
}

char* listMyBids(char* UID){
    FILE* file = fopen(pathClientBidder(UID), "r");
    
    if (file != NULL) {
        int character;
        int first_iteration = 1;

        char* output = malloc(256 * sizeof(char));

        if (output != NULL) {
            // Initialize the output string
            strcpy(output, "RMB OK");

            do {
                // Read a line at a time
                char* line = malloc(256 * sizeof(char)); // Adjust size as needed

                if (line != NULL) {
                    int index = 0;

                    do {
                        // Check if it's a newline character
                        character = fgetc(file);

                        if(first_iteration){
                            if(character == EOF){
                                return "RMB NOK";
                            }
                            first_iteration = 0;
                        }

                        // Check if it's a newline character
                        if (character == '\n' || character == EOF) {
                            break;  // Exit the inner loop on newline or end of file
                        }

                        // Append the character to the line
                        line[index++] = character;

                    } while (index < 255);

                    // Null-terminate the line
                    line[index] = '\0';

                    // Concatenate the line to the output string
                    strcat(output, " ");
                    strcat(output, line);
                    strcat(output, " ");
                    if(!strcmp("", line)){
                        break; //ignore empty strings
                    }
                    strcat(output, getAuctionStatus(line));

                    // Free the memory allocated for the line
                    free(line);
                } else {
                    perror("Error allocating memory");
                    free(output);
                    break;  // Exit the outer loop if memory allocation fails
                }

            } while (character != EOF);

            // Print or process the final output string
            fclose(file);
            return output;

        } else {
            perror("Error allocating memory");
            return NULL;
        }
    } else {
        perror("Error opening file");
        return NULL;
    }
}

char* listAllAuctions(){
    DIR* dir = opendir("Auctions");

    if (dir != NULL) {
        // Read the first entry in the directory
        struct dirent* entry = readdir(dir);

        if (entry == NULL) {
            // Directory is empty
            closedir(dir);
            return "RLS NOK";
        }


        char* output = malloc(256*sizeof(char));
        output[0] = '\0';
        strcat(output, "RLS OK");
        do {
            // Ignore "." and ".." entries
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {

                if(!strcmp(getAuctionStatus(entry->d_name),"1")){ //if auction is active
                    strcat(output, " ");
                    strcat(output, entry->d_name);
                    strcat(output, " 1");
                }
            }

            // Read the next entry in the directory
            entry = readdir(dir);

        } while (entry != NULL);

        // Close the directory
        closedir(dir);

        return output;
    } else {
        perror("Error opening directory");
        return NULL;
    }
}

void createAsset(char* filename, char* data, char* size){
    char* path = pathAsset(filename);
    FILE* file = fopen(path, "wb");

    if (file != NULL) {
        // Write the content to the file
        fprintf(file, "%s\n%s", size, data);

        // File written successfully
    } else {
        perror("Error opening file");
        return;
    }

    // Close the file
    fclose(file);

    free(path); // Free the allocated memory 
}

char* createAuction(char* auction_name, char* host_UID, char* asset_fname, char* asset_data, char* asset_size, char* start_value, char* timeactive){
    char* aid = getNextAID();
    if(aid==NULL){
        perror("Maximum number of auctions reached.");
        return NULL;
    }

    if (access(pathAuctionFolder(aid), F_OK) != 0) {
        if (mkdir(pathAuctionFolder(aid), 0755)) {
            perror("Error creating directory");
            return NULL;
        }
    }
    
    FILE* file = fopen(pathAuctionMain(aid), "w");

    if (file != NULL) {
        // Write the content to the file
        fprintf(file, "%s\n%s\n%s\n%s\n%s\n%s", host_UID, asset_fname, start_value, getCurrentDateTime(), timeactive, auction_name);

        // Close the file
        fclose(file);

        // File written successfully
    } else {
        perror("Error opening file");
        return NULL;
    }

    file = fopen(pathAuctionEnd(aid), "w");

    if (file != NULL) {
        // Write the content to the file
        fprintf(file, "1");

        // Close the file
        fclose(file);

        // File written successfully
    } else {
        perror("Error opening file");
        return NULL;
    }


    file = fopen(pathAuctionBids(aid), "w");
    if (file == NULL) {
        perror("Error creating file");
        return NULL;
    }
    else{
        // Close the file
        fclose(file);
    }


    // Add info to host User's auctions
    file = fopen(pathClientHost(host_UID), "a");

    if (file != NULL) {
        // Write the content to the file
        fprintf(file, "%s\n", aid);

        // Close the file
        fclose(file);

        // File written successfully
    } else {
        perror("Error opening file");   
        return NULL;
    }

    createAsset(asset_fname, asset_data, asset_size);
    return aid;
}

void closeAuction(char* AID){
    FILE* file = fopen(pathAuctionEnd(AID), "w");
    

    if (file != NULL) {
        // Write the content to the file
        fprintf(file, "0\n%s\n%s\n", getCurrentDateTime(), computeTimeDifference(getCurrentDateTime(), getAuctionStartDateTime(AID))); //computing seconds auction was opened

        // Close the file
        fclose(file);

        // File written successfully
    } else {
        perror("Error opening file");
        return;
    }
}

char* showAsset(char* AID){ //FIXME
    char* output = (char *)malloc(6 + 1 + ASSET_FILENAME_MAX_LEN + 1 + ASSET_SIZE_MAX_LEN + 2);
    output[0] = '\0';
    strcat(output, "RSA OK ");
    strcat(output, getAuctionAssetFName(AID));
    strcat(output, " ");


    // compute how much more memory to allocate to output buffer
    char* assetsize_str = getAssetSize(AID);
    int assetsize_int = atoi(assetsize_str);


    strcat(output, assetsize_str);

    // allocate more memory to output buffer
    int newSize = strlen(output) + assetsize_int;
    output = (char*)realloc(output, newSize + 2);


    strcat(output, " ");
    strcat(output, getAssetData(AID));
    return output;  
}

int bid(char* AID, char* value, char* UID){
    if(auctionIsOwnedBy(UID, AID)){
        return 3;
    }

    if(!strcmp(getAuctionStatus(AID),"0")){
        return 4;
    }
    if(validateBid(AID, value)){
        // Add info to host User's bids
        FILE* file = fopen(pathClientBidder(UID), "a");

        if (file != NULL) {
            // Write the content to the file
            fprintf(file, "%s\n", AID);

            // Close the file
            fclose(file);

            // File written successfully
        } else {
            perror("Error opening file");   
            return 0;
        }
        file = fopen(pathAuctionBids(AID), "a");

        if (file != NULL) {
            // Write the content to the file
            fprintf(file, "%s\n%s\n%s\n%s\n", UID, value, getCurrentDateTime(), computeTimeDifference(getCurrentDateTime(), getAuctionStartDateTime(AID)));

            // Close the file
            fclose(file);
            return 1;

            // File written successfully
        } else {
            perror("Error opening file");   
            return 0;
        }
    } else{
        return 2;
    }
}

char* showRecord(char* AID){
    if(auctionExists(AID)){
        char* output = malloc((MAX_RESPONSE_SIZE)*sizeof(char));
        output[0] = '\0';

        //status
        strcat(output, "RRC OK ");

        //main.txt info

        strcat(output, getAuctionHostUID(AID));
        strcat(output, " ");
        strcat(output, getAuctionName(AID));
        strcat(output, " ");
        strcat(output, getAuctionAssetFName(AID));
        strcat(output, " ");
        strcat(output, getAuctionStartValue(AID));
        strcat(output, " ");
        strcat(output, getAuctionStartDateTime(AID));
        strcat(output, " ");
        strcat(output, getAuctionTimeactive(AID));

        //bids info
        
        strcat(output, " ");
        strcat(output, getBidsInfo(AID));

        //end info

        if(!strcmp(getAuctionStatus(AID),"0")){ //if auction ended, add end info
            strcat(output, " E ");
            strcat(output, getAuctionEndDatetime(AID));
            strcat(output, " ");
            strcat(output, getAuctionRealDuration(AID));
        }
        return output;

    } else{
        return "RRC NOK";
    }
}
