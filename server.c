#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include "utils.h"
#include "logged_in_linkedlist.h"
#include "datasizes.h"
#include "serializer_client.h"
#include "file_management.h"

#define BUFFER_SIZE 1024

int verbose;

pthread_mutex_t loggedInUsersMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t filesMutex = PTHREAD_MUTEX_INITIALIZER;

int server_tcp_socket;
int server_udp_socket;
Node *loggedInUsers = NULL;


// Struct to hold UDP client information
struct UdpClientInfo {
    int server_udp_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE];
};

// Signal handler function
void handle_sigint(int sig) {
    printf("Received CTRL+C. Closing socket and exiting.\n");
    close(server_tcp_socket);
    close(server_udp_socket);
    exit(EXIT_SUCCESS);
}




// Handle client requests functions

char* handleLogin(char* UID, char* password){
    if(!isValidUsername(UID) || !isValidPassword(password)){
        return "";
    }
    pthread_mutex_lock(&loggedInUsersMutex);
    if(isUserLoggedIn(loggedInUsers, UID)){
        pthread_mutex_unlock(&loggedInUsersMutex);
        return "RLI ALR";
    }
    pthread_mutex_unlock(&loggedInUsersMutex);
    int status = checkCredentials(UID, password);
    if(status == 0){
        registerUser(UID, password);

        pthread_mutex_lock(&loggedInUsersMutex);
        addLoggedInUser(&loggedInUsers, UID);
        pthread_mutex_unlock(&loggedInUsersMutex);

        return "RLI REG";
    } else if(status == 1){

        pthread_mutex_lock(&loggedInUsersMutex);
        addLoggedInUser(&loggedInUsers, UID);
        pthread_mutex_unlock(&loggedInUsersMutex);

        return "RLI OK";
    } else if(status == 2){
        return "RLI NOK";
    } else{
        fprintf(stderr, "There was an unexpected error.\n");
        exit(EXIT_FAILURE);
    }
}


char* handleLogout(char* UID, char* password){
    if(!isValidUsername(UID) || !isValidPassword(password) || checkCredentials(UID, password)!=1){
        return "";
    }
    pthread_mutex_lock(&loggedInUsersMutex);
    if(!isUserLoggedIn(loggedInUsers, UID)){
        pthread_mutex_unlock(&loggedInUsersMutex);
        return "RLO NOK";
    }else{
        if(checkCredentials(UID, password)==1){
            removeLoggedInUser(&loggedInUsers, UID);        
            pthread_mutex_unlock(&loggedInUsersMutex);
            return "RLO OK";
        } else{
            pthread_mutex_unlock(&loggedInUsersMutex);
            return "RLO UNR";
        }
    }
    pthread_mutex_unlock(&loggedInUsersMutex);
}


char* handleUnregister(char* UID, char* password){
    if(!isValidUsername(UID) || !isValidPassword(password) || checkCredentials(UID, password)!=1){
        return "";
    }
    pthread_mutex_lock(&loggedInUsersMutex);
    if(!isUserLoggedIn(loggedInUsers, UID)){
        return "RUR NOK";
    }else{
        if(checkCredentials(UID, password)==1){
            removeLoggedInUser(&loggedInUsers, UID);
            unregisterUser(UID);
            return "RUR OK";
        } else{
            return "RUR UNR";
        }
    }
    pthread_mutex_unlock(&loggedInUsersMutex);
}


char* handleListMyAuctions(char* UID){
    if(!isValidUsername(UID)){
        return "";
    } 
    pthread_mutex_lock(&loggedInUsersMutex);
    if(!isUserLoggedIn(loggedInUsers, UID)){
        pthread_mutex_unlock(&loggedInUsersMutex);
        return "RMA NLG";
    } else{
        pthread_mutex_unlock(&loggedInUsersMutex);
        return listMyAuctions(UID);
    }
}

char* handleListMyBids(char* UID){
    if(!isValidUsername(UID)){
        return "";
    } 
    pthread_mutex_lock(&loggedInUsersMutex);
    if(!isUserLoggedIn(loggedInUsers, UID)){
        pthread_mutex_unlock(&loggedInUsersMutex);
        return "RMB NLG";
    } else{
        pthread_mutex_unlock(&loggedInUsersMutex);
        return listMyBids(UID);
    }
}

char * handleOpen(char* host_UID, char* password, char* auction_name, char* start_value, char* timeactive, char* asset_fname, char* asset_size, char* asset_data){
    if(!isValidUsername(host_UID) || !isValidPassword(password) || !isValidAuctionName(auction_name) || !isValidAuctionStartValue(start_value) || !isValidAuctionTimeActive(timeactive) || !isValidAssetFilename(asset_fname) || !isValidFileSize(asset_size)|| checkCredentials(host_UID, password)!=1){
        return "";
    }
    
    pthread_mutex_lock(&loggedInUsersMutex);
    if(!isUserLoggedIn(loggedInUsers, host_UID)){
        pthread_mutex_unlock(&loggedInUsersMutex);
        return "ROA NOK";
    }
    pthread_mutex_unlock(&loggedInUsersMutex);
    
    char* AID = createAuction(auction_name, host_UID, asset_fname, asset_data, asset_size, start_value, timeactive);
    if(AID == NULL){
        return "ROA NOK";
    }else{
        char*  output = (char*) malloc(3 + 1 + AID_MAX_LEN + 1);
        output[0] = '\0';
        strcat(output, "ROA OK ");
        strcat(output, AID);
        return output;
    }
}

char* handleClose(char* UID, char* password, char* AID){
    if(!isValidUsername(UID) || !isValidPassword(password) || !isValidAID(AID)){
        return "";
    }
    pthread_mutex_lock(&loggedInUsersMutex);
    if(!isUserLoggedIn(loggedInUsers, UID) || checkCredentials(UID, password)!=1){
        pthread_mutex_unlock(&loggedInUsersMutex);
        return "RCL NLG";
    }
    pthread_mutex_unlock(&loggedInUsersMutex);

    if(!auctionExists(AID)){
        return "RCL EAU";
    }
    if(!auctionIsOwnedBy(UID, AID)){
        return "RCL EOW";
    }
    if(getAuctionStatus(AID) == 0){
        return "RCL END";
    }
    closeAuction(AID);
    return "RCL OK";
}

char* handleBid(char* UID, char* password, char* AID, char* value){
    if(!isValidUsername(UID) || !isValidPassword(password) || !isValidAID(AID) || checkCredentials(UID, password)!=1){
        return "";
    }
    pthread_mutex_lock(&loggedInUsersMutex);
    if(!isUserLoggedIn(loggedInUsers, UID)){
        pthread_mutex_unlock(&loggedInUsersMutex);
        return "RBD NLG";
    }
    pthread_mutex_unlock(&loggedInUsersMutex);
    
    int status = bid(AID, value, UID);
    
    if(status==2){
        return "RBD REF";
    }
    if(status==3){
        return "RBD ILG";
    }
    if(status==4){
        return "RBD CLO"; //new response for closed auction
    }
    return "RBD ACC";
}

char* handleShowAsset(char* AID){
    if(auctionExists(AID)){
        return showAsset(AID);
    }else{
        return "RSA NOK";
    }
}

char * handleShowRecord(char* AID){
    return showRecord(AID);
}

// END_OF_SECTION: Handle client requests functions


// Function to handle TCP client
void *handle_tcp_client(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        if(verbose) printf("(verbose)RECEIVED: ~%s~\n", buffer);


        char** words;
        int numWords;
        getWords(buffer, &words, &numWords);
        
        char* response = malloc(256*sizeof(char));
        response[0] = '\0';

        if(numWords == 2 && !strcmp(words[0],"SAS")){ //show asset
            strcat(response, handleShowAsset(words[1]));

        } else if(numWords == 4 && !strcmp(words[0],"CLS")){ //close auction
            strcat(response, handleClose(words[1],words[2],words[3]));
            
        } else if(numWords == 5 && !strcmp(words[0],"BID")){ //make a bid
            strcat(response, handleBid(words[1],words[2],words[3],words[4]));

        } else if(numWords == 9 && !strcmp(words[0],"OPA")){ //open auction
            strcat(response, handleOpen(words[1],words[2],words[3],words[4],words[5],words[6],words[7],words[8]));
            
        } else{
            fprintf(stderr, "Client's request is invalid.\n" );
        }
        freeWords(words, numWords);

        


        send(client_socket, response, strlen(response), 0);
        if(verbose) printf("(verbose) SENT: ~%s~\n", response);

        if(verbose) printf("(verbose) Received: %s\n", buffer);
    }

    close(client_socket);
    free(arg);
    return NULL;
}

// Function to manage TCP connections
void * manage_tcp_connections(void *arg) {
    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(server_tcp_socket, (struct sockaddr *)&client_address, &client_address_len);

        if (*client_socket == -1) {
            perror("Error accepting client connection");
            free(client_socket);
            continue;
        }

        if(verbose) printf("(verbose) Connection accepted from %s:%d\n",
               inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_tcp_client, (void *)client_socket) != 0) {
            perror("Error creating thread");
            close(*client_socket);
            free(client_socket);
            continue;
        }

        pthread_detach(thread);
    }
}

// functions for udp

void *handle_udp_client(void *arg) {
    struct UdpClientInfo *udp_info = (struct UdpClientInfo *)arg;

    // Access necessary information
    int server_udp_socket = udp_info->server_udp_socket;
    struct sockaddr_in client_addr = udp_info->client_addr;
    socklen_t addr_len = udp_info->addr_len;

    char** words;
    int numWords;
    getWords(udp_info->buffer, &words, &numWords);
    
    char* response = malloc((MAX_RESPONSE_SIZE + 1)*sizeof(char));
    response[0] = '\0';

    if(numWords == 1 && !strcmp(words[0],"LST")){ //list all auctions
        strcat(response, listAllAuctions());
    } else if(numWords == 2){
        if(!strcmp(words[0],"LMA")){ //list my auctions
            strcat(response, handleListMyAuctions(words[1]));
        
        } else if(!strcmp(words[0],"LMB")){ //list my bids
            strcat(response, handleListMyBids(words[1]));
            
        } else if(!strcmp(words[0],"SRC")){ // show record
            strcat(response, handleShowRecord(words[1]));
            
        } else{
            fprintf(stderr, "Client's request is invalid.\n" );
        }
    } else if(numWords == 3){
        if(!strcmp(words[0],"LIN")){ //login
            strcat(response, handleLogin(words[1], words[2]));

        } else if(!strcmp(words[0],"LOU")){ //logout
            strcat(response, handleLogout(words[1], words[2]));
            
        } else if(!strcmp(words[0],"UNR")){ //unregisteruser
            strcat(response, handleUnregister(words[1], words[2]));
            
        } else{
            fprintf(stderr, "Client's request is invalid.\n" );
        }
    } else{
        fprintf(stderr, "Client's request is invalid.\n" );
    }
    strcat(response, "\n");
    ssize_t send_len = sendto(server_udp_socket, response, strlen(response), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));


    if(verbose) printf("(verbose)SENT: ~%s~\n", response);

    if (send_len == -1) {
        perror("Error sending data");
        return NULL;  // Continue listening for other packets
    }

    if(verbose) printf("(verbose) Reply sent to %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    free(response);
    freeWords(words, numWords);
    return NULL;
}

void *manage_udp_connections(void* arg) {
    int server_udp_socket = *((int *)arg);
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (1) {
        struct UdpClientInfo *udp_info = malloc(sizeof(struct UdpClientInfo));

        // Receive data from client
        memset(udp_info->buffer, 0, sizeof(udp_info->buffer));
        ssize_t recv_len = recvfrom(server_udp_socket, udp_info->buffer, sizeof(udp_info->buffer), 0, (struct sockaddr*)&client_addr, &addr_len);

        if(verbose) printf("(verbose) RECEIVED: ~%s~\n", udp_info->buffer);
        if (recv_len == -1) {
            perror("Error receiving data");
            continue;  // Continue listening for other packets
        } else {
            // Create struct to hold UDP client information
            udp_info->server_udp_socket = server_udp_socket;
            udp_info->client_addr = client_addr;
            udp_info->addr_len = addr_len;

            pthread_t thread_udp_slave;
            
            if (pthread_create(&thread_udp_slave, NULL, handle_udp_client, (void *)udp_info) != 0) {
                perror("Error creating thread");
                // Handle error as needed
                free(udp_info);
                continue;
            }

            pthread_detach(thread_udp_slave);
        }
    }
}

int main(int argc, char *argv[]) {
    setupWorkspace();
    int port;
    verbose = 0;
    if (argc == 1) {
        port = STDARD_SERVER_PORT;
    } else if (argc == 3 && !strcmp(argv[1], "-p")) {
        port = atoi(argv[2]);
    }  else if (argc == 4 && !strcmp(argv[1], "-p") && !strcmp(argv[3], "-v")) {
        port = atoi(argv[2]);
        verbose = 1;
    }  else if (argc == 4 && !strcmp(argv[1], "-v") && !strcmp(argv[2], "-p")) {
        port = atoi(argv[3]);
        verbose = 1;
    }  else if (argc == 2 && !strcmp(argv[1], "-v")) {
        verbose = 1;
        port = STDARD_SERVER_PORT;
    } else {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    server_tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_tcp_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_udp_socket == -1) {
        close(server_tcp_socket);
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_pton(AF_INET, STDARD_SERVER_IP, &server_address.sin_addr);

    if (bind(server_tcp_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        
        perror("Socket binding failed");
        close(server_udp_socket);
        close(server_tcp_socket);
        exit(EXIT_FAILURE);
    }
    if (bind(server_udp_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding socket");
        close(server_udp_socket);
        close(server_tcp_socket);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_sigint);

    if (listen(server_tcp_socket, 5) == -1) {
        perror("Socket listening failed");
        close(server_tcp_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on... \n");
    printf("  - Server address: %s\n", inet_ntoa(server_address.sin_addr));
    printf("  - Server port: %d\n", ntohs(server_address.sin_port));


    if(verbose) printf("\nVerbose mode turned on...\n");
    pthread_t tcp_thread;
    if (pthread_create(&tcp_thread, NULL, manage_tcp_connections, NULL) != 0) {
        perror("Error creating TCP thread");
        return 1;
    }

    pthread_t udp_thread;
    if (pthread_create(&udp_thread, NULL, manage_udp_connections, (void*)&server_udp_socket) != 0) {
        perror("Error creating UDP thread");
        return 1;
    }

    pthread_join(tcp_thread, NULL);
    pthread_join(udp_thread, NULL);

    close(server_tcp_socket);
    close(server_udp_socket);

    return 0;
}