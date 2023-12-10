#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "serializer_client.h"
#include "utils.h"
#include "datasizes.h"

#define CONTINUE_COMMAND 1
#define EXIT_COMMAND 0

char username[UID_LEN + 1] = "";
char password[USER_PASSWORD_LEN + 1] = "";

char server_ip[IP_LENGTH];
int server_port;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void set_args(int argc, char*argv[]){
    int n_input = 0;
    int p_input = 0;
    if(argc==3 || argc == 5){ //parse first input argument
        if(!strcmp(argv[1],"-n")){
            strcpy(server_ip, argv[2]);
            n_input = 1;
        } else if(!strcmp(argv[1],"-p")){
            if(!strcmp(argv[3],"-p")){
                fprintf(stderr, "Usage: %s -n <ASIP> -p <ASport>\n", argv[0]);
                exit(EXIT_FAILURE);
            }else{
                server_port = atoi(argv[2]);
                p_input = 1;
            }
        } else{
            fprintf(stderr, "Usage: %s -n <ASIP> -p <ASport>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        if(argc==5){ //parse second input argument
            if(!strcmp(argv[3],"-p")){
                server_port = atoi(argv[4]);
                p_input = 1;
            } else{
                fprintf(stderr, "Usage: %s -n <ASIP> -p <ASport>\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }

    }
    else if(argc!=1){
        fprintf(stderr, "Usage: %s -n <ASIP> -p <ASport>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(!n_input){
        strcpy(server_ip, STDARD_SERVER_IP);
    }
    if(!p_input){
        server_port = STDARD_SERVER_PORT;
    }
}

char* tcp_connection(const char *message) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening socket");
    }

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        close(sockfd);
        error("Error converting IP address");
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        error("Error connecting to server");
    }

    // Send message to server
    if (send(sockfd, message, strlen(message), 0) < 0) {
        close(sockfd);
        error("Error sending message to server");
    }

    // Receive response from server
    char *buffer = (char*)malloc(1024 * sizeof(char));  // Allocate memory dynamically
    if (buffer == NULL) {
        close(sockfd);
        error("Error allocating memory for buffer");
    }

    memset(buffer, 0, 1024);
    if (recv(sockfd, buffer, 1023, 0) < 0) {
        close(sockfd);
        free(buffer);  // Don't forget to free allocated memory in case of an error
        error("Error receiving response from server");
    }

    // Close the connection
    close(sockfd);

    return buffer;
}

char* udp_connection(const char *message) {
    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &serverAddr.sin_addr);

    // Send the message to the server
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    // Receive the response from the server
    char *buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));  // Allocate memory dynamically
    if (buffer == NULL) {
        perror("Error allocating memory for buffer");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    socklen_t serverAddrLen = sizeof(serverAddr);
    ssize_t bytesRead = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&serverAddr, &serverAddrLen);
    if (bytesRead == -1) {
        perror("Error receiving response");
        close(sockfd);
        free(buffer);  // Don't forget to free allocated memory in case of an error
        exit(EXIT_FAILURE);
    }

    // Null-terminate the received data
    buffer[bytesRead] = '\0';
    // Close the socket
    close(sockfd);

    return buffer;
}




int isLoggedIn(){
    return strcmp("", username) && strcmp("", password); //both shall be different from empty string, for user to be logged in
}

// handle each user command (send request, receive response, print status description)

int handleLogin(char* _username, char* _password) {
    if(!isValidUsername(_username)){
        fprintf(stderr, "Entered username isn't valid.\n");
        return 0;
    }

    if(!isValidPassword(_password)){
        fprintf(stderr, "Entered password isn't valid.\n");
        return 0;
    }

    char request[3 + 1 + UID_LEN + 1 + USER_PASSWORD_LEN + 1 + 1];
    sprintf(request, "LIN %s %s\n", _username, _password);
    char* response = udp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);

    if (numWords_response == 2 && !strcmp(words_response[0], "RLI")) {
        // Process response
        if (!strcmp(words_response[1], "OK")) {
            printf("Successful login.\n");
            return 1;
        } else if (!strcmp(words_response[1], "NOK")) {
            printf("Incorrect login.\n");
            return 0;
        } else if (!strcmp(words_response[1], "ALR")) {
            printf("Someone's already logged-in in that account.\n");
            return 0;
        } else if (!strcmp(words_response[1], "REG")) {
            printf("New user registered.\n");
            return 1;
        } else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }
}

int handleLogout(){
    char request[3 + 1 + UID_LEN + 1+ USER_PASSWORD_LEN + 1 + 1];
    sprintf(request, "LOU %s %s\n", username, password);

    char* response = udp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);

    if (numWords_response == 2 && !strcmp(words_response[0], "RLO")) {
        // Process response
        if (!strcmp(words_response[1], "OK")) {
            printf("Successful logout.\n");
            return 1;
        } else if (!strcmp(words_response[1], "NOK")) {
            printf("User not logged in.\n");
            return 0;
        } else if (!strcmp(words_response[1], "UNR")) {
            printf("Unknown user.\n");
            return 0;
        } else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }
}

int handleUnregister(){
    char request[3 + 1 + UID_LEN + 1+ USER_PASSWORD_LEN + 1 + 1];
    sprintf(request, "UNR %s %s\n", username, password);

    char* response = udp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);
    if (numWords_response == 2 && !strcmp(words_response[0], "RUR")) {
        // Process response
        if (!strcmp(words_response[1], "OK")) {
            printf("Successful unregister.\n");
            return 1;
        } else if (!strcmp(words_response[1], "NOK")) {
            printf("Incorrect unregister attempt.\n");
            return 0;
        } else if (!strcmp(words_response[1], "UNR")) {
            printf("Unknown user.\n");
            return 0;
        } else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }
    
}

int handleExit(){
    if(!strcmp(username, "") && !strcmp(password, "")){ // if loggedout
        return 1;
    } else{
        printf("Please execute the logout command before exiting the application.\n");
        return 0;
    }
}

void handleOpen(char* name, char* asset_fname, char* start_value, char* timeactive){
    if(!isValidAuctionName(name)){
        fprintf(stderr, "Entered auction name isn't valid.\n");
        return;
    }

    if(!isValidAssetFilename(asset_fname)){
        fprintf(stderr, "Entered asset file name isn't valid.\n");
        return;
    }

    if(!isValidAuctionStartValue(start_value)){
        fprintf(stderr, "Entered auction start value isn't valid.\n");
        return;
    }

    if(!isValidAuctionTimeActive(timeactive)){
        fprintf(stderr, "Entered auction time active isn't valid.\n");
        return;
    }

    char* asset_data;
    char* asset_size;
    
    if(getDataAndSize(asset_fname, &asset_data, &asset_size)==1){
        printf("The specified file was not found.\n");
        return;
    }
    
    int asset_size_int;
    char *request;
    if(asset_size_int = isValidFileSize(asset_size)){
        request = (char *)malloc(3 + 1 + UID_LEN + 1 + USER_PASSWORD_LEN + 1 + AUCTION_NAME_MAX_LEN + 1 + AUCTION_START_VALUE_LEN + 1 + AUCTION_TIME_ACTIVE_LEN + 1 + ASSET_FILENAME_MAX_LEN + 1 + ASSET_SIZE_MAX_LEN + 1 + asset_size_int + 1 + 1);

    } else{
        printf("The asset file can't be bigger than 10MB.\n");
        return;
    }
    sprintf(request, "OPA %s %s %s %s %s %s %s %s\n",
    username, password, name, start_value, timeactive, asset_fname, asset_size, asset_data);

    char* response = tcp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);
    if (!strcmp(words_response[0], "ROA")) {
        if(numWords_response == 2 && !strcmp(words_response[1], "NOK")){
            printf("Auction could not be started.");
        } else if(numWords_response == 3 && !strcmp(words_response[1], "OK")){
            printf("An auction with the AID %s was opened.\n", words_response[2]);
        } else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }  
}

void handleClose(char* AID){
    if(!isValidAID(AID)){
        fprintf(stderr, "Entered AID isn't valid.\n");
        return;
    }


    char request[3 + 1 + UID_LEN + 1 + USER_PASSWORD_LEN + 1 + AID_MAX_LEN + 1 + 1];
    sprintf(request, "CLS %s %s %s\n", username, password, AID);

    char* response = tcp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);
    if (numWords_response == 2 && !strcmp(words_response[0], "RCL")) {
        // Process response
        if (!strcmp(words_response[1], "OK")) {
            printf("Auction %s successfully closed.\n", AID);
            return;
        } else if (!strcmp(words_response[1], "EAU")) {
            printf("Auction %s doesn't exist.\n", AID);
            return;
        } else if (!strcmp(words_response[1], "END")) {
            printf("Auction %s was already closed.\n", AID);
            return;
        } else if (!strcmp(words_response[1], "EOW")) {
            printf("Auction %s isn't yours to close.\n", AID);
            return;
        } else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }

}

void handleMyBids(){
    char request[3 + 1 + UID_LEN + 1 + 1];
    sprintf(request, "LMB %s\n", username);

    char* response = udp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);
    if (!strcmp(words_response[0], "RMB")) {
        // Process response
        if (numWords_response == 2 && !strcmp(words_response[1], "NOK")) {
            printf("You have no ongoing bids.\n");
            return;
        }else if (!strcmp(words_response[1], "OK")) {
            printf("List of auctions you've bidded on:\n");
            for(int i = 2; i<numWords_response-1;i+=2){
                printf("AID: %s ; State: %s \n", words_response[i], words_response[i+1]);
            }
            return;
        } else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }

}

void handleMyAuctions(){
    char request[3 + 1 + UID_LEN + 1+1];
    sprintf(request, "LMA %s\n", username);

    char* response = udp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);
    if (!strcmp(words_response[0], "RMA")) {
        // Process response
        if (numWords_response == 2 && !strcmp(words_response[1], "NOK")) {
            printf("You have no ongoing auctions.\n");
            return;
        }else if (!strcmp(words_response[1], "OK")) {
            printf("List of auctions you created:\n");
            for(int i = 2; i<numWords_response-1;i+=2){
                printf("AID: %s ; State: %s \n", words_response[i], words_response[i+1]);
            }
            return;
        } else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }

}

void handleList(){
    char request[3+1+1];
    sprintf(request, "LST\n");
    char* response = udp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);
    if (!strcmp(words_response[0], "RLS")) {
        // Process response
        if (numWords_response == 2 && !strcmp(words_response[1], "NOK")) {
            printf("No auction was yet started.\n");
            return;
        }else if ((numWords_response % 2 == 0) && !strcmp(words_response[1], "OK")) {
            printf("List of all auctions:\n");
            for(int i = 2; i<numWords_response-1;i+=2){
                printf("AID: %s ; State: %s \n", words_response[i], words_response[++i]);
            }
            return;
        } else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }
}

void handleShowRecord(char* AID){
    char request[3 + 1 + AID_MAX_LEN + 2];
    sprintf(request, "SRC %s", AID);
    char* response = udp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);
    if (!strcmp(words_response[0], "RRC")) {
        // Process response
        if (numWords_response == 2 && !strcmp(words_response[1], "NOK")) {
            printf("Auction %s doesn't exist.\n", AID);
        }else if (!strcmp(words_response[1], "OK" )) {
            int index = 1;
            while(index < numWords_response){
                if(!strcmp(words_response[index],"B")){
                    index++;
                    printf("Bid:\n  bidder_UID:%s\n  bid_value:%s\n  bid_date-time:%s %s\n  bid_sec_time:%s\n",
                    words_response[index],words_response[index+1],
                    words_response[index+2],words_response[index+3],words_response[index+4]);
                    index+=4;
                } else if(!strcmp(words_response[index],"E")){
                    index++;
                    printf("This auction ended at %s %s - it lasted %s seconds.\n",
                    words_response[index],words_response[index+1],words_response[index+2]);
                    index+=2;
                    break; //"end" should be the last part of the response, if not the server is not following the standards
                } else{
                    index++;
                    printf("host_UID:%s\n  auction_name:%s\n  asset_fname:%s\n  start_value:%s\n  start_data-time:%s %s\n  timeactive:%s\n",
                    words_response[index],words_response[index+1],words_response[index+2],words_response[index+3],
                    words_response[index+4],words_response[index+5],words_response[index+6]);
                    index+=6;
                }
                index++;
            }
        } else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }
}

void handleShowAsset(char* AID){
    if(!isValidAID(AID)){
        fprintf(stderr, "Entered AID isn't valid.\n");
        return;
    }

    char request[3+1+AID_MAX_LEN+1+1];
    sprintf(request, "SAS %s\n", AID);
    char* response = tcp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);
    if (!strcmp(words_response[0], "RSA")) {
        // Process response
        if (numWords_response == 2 && !strcmp(words_response[1], "NOK")) {
            printf("There is not file to be shown, or there was another type of problem.\n");
        }else if ((numWords_response == 5) && !strcmp(words_response[1], "OK")) {
            printf("Asset related to the auction %s:\n", AID);
            printf("File name: %s; \nFile size: %s; \nFile data:\n %s\n", words_response[2], words_response[3], words_response[4]);
        } else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }
}

void handleBid(char* AID, char* value){
    if(!isValidAID(AID)){
        fprintf(stderr, "Entered AID isn't valid.\n");
        return;
    }
    
    if(!isValidBidValue(value)){
        fprintf(stderr, "Entered bid value isn't valid.\n");
        return;
    }

    char request[3 + 1 + UID_LEN + 1 + USER_PASSWORD_LEN + 1 +AID_MAX_LEN+1+BID_VALUE_MAX_LEN+1+ 1];
    sprintf(request, "BID %s %s %s %s\n", username, password, AID, value);
    char* response = tcp_connection(request);

    char** words_response;
    int numWords_response;
    getWords(response, &words_response, &numWords_response);
    if (!strcmp(words_response[0], "RBD") && numWords_response == 2) {
        // Process response
        if (!strcmp(words_response[1], "NOK")) {
            printf("The auction %s is not active.\n", AID);
        }else if (!strcmp(words_response[1], "NLG")) {
            printf("User is not logged in.\n");
        }else if (!strcmp(words_response[1], "ACC")) {
            printf("Your bid was accepted: %s on auction %s.\n", value, AID);
        }else if (!strcmp(words_response[1], "REF")) {
            printf("Your bid was smaller then the current biggest bid on auction %s.\n", AID);
        }else if (!strcmp(words_response[1], "ILG")) {
            printf("You can't bid in your own auction.\n");
        }else if (!strcmp(words_response[1], "CLO")) {
            printf("You can't bid, because the auction is already closed.\n");
        }else {
            fprintf(stderr, "Server's response is invalid.\n");
        }
    } else {
        fprintf(stderr, "Server's response is invalid.\n");
    }
}

// END OF SECTION handle each user command

int handleUserCommand(char* command) {
    // Split the command into words
    char** words;
    int numWords;
    getWords(command, &words, &numWords);
    char* request = malloc(256*sizeof(char));
    request[0] = '\0';
    char* response = malloc(256*sizeof(char));
    printf("\n");
    if(isLoggedIn() && numWords==1){
        if(!strcmp(words[0],"logout")){
            if(handleLogout()==1){
                strcpy(username, "");
                strcpy(password, "");
            }

        } else if(!strcmp(words[0],"unregister")){
            if(handleUnregister()==1){
                strcpy(username, "");
                strcpy(password, "");
            }
            
        } else if(!strcmp(words[0],"myauctions") || !strcmp(words[0],"ma")){
            handleMyAuctions();
        } else if(!strcmp(words[0],"mybids") || !strcmp(words[0],"mb")){
            handleMyBids();
        } else if(!strcmp(words[0],"list") || !strcmp(words[0],"l")){
            handleList();
        } else if(strcmp(words[0],"exit")){
            printf("You need to logout before exiting the program.\n" );
        }
    } else if(isLoggedIn() && numWords == 2){
        if(!strcmp(words[0],"show_record") || !strcmp(words[0],"sr")){
            handleShowRecord(words[1]);
        } else if(!strcmp(words[0],"close")){
            handleClose(words[1]);
        } else if(!strcmp(words[0],"show_asset") || !strcmp(words[0],"sa")){
            handleShowAsset(words[1]);
        } else{
            fprintf(stderr, "You've entered an invalid command.\n" );
        }
    } else if(numWords == 3){
        if(!strcmp(words[0], "login")){
            if(handleLogin(words[1], words[2])==1){ // user now logged
                strcpy(username, words[1]);
                strcpy(password, words[2]);
            }
        } else if(isLoggedIn() && !strcmp(words[0],"bid") || !strcmp(words[0], "b")){
            handleBid(words[1], words[2]);
        } else{
            fprintf(stderr, "You've entered an invalid command.\n" );
        }

    } else if(isLoggedIn() && numWords == 5 && !strcmp(words[0], "open")){
        handleOpen(words[1], words[2], words[3], words[4]);

    } else if(!strcmp(words[0],"exit") && numWords == 1){
        if(handleExit()==1){
            return EXIT_COMMAND;
        }
    } else{

        fprintf(stderr, "You've entered an invalid command.\n" );
    }
    printf("\n");

    // Free the allocated memory
    freeWords(words, numWords);

    return CONTINUE_COMMAND;
}

int main(int argc, char *argv[]) {
    set_args(argc,argv); //set initial parameters
    // Get input from the user and send to the server
    char buffer[BUFFER_SIZE];
    while (1) {
        printf("Enter message (type 'exit' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        if(handleUserCommand(buffer)==EXIT_COMMAND){
            break;
        }
    }


    return 0;
}