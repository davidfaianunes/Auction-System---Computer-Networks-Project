#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024
#define IP_LENGTH 39

char server_ip[] = "127.0.0.1";
int server_port = 58000 + 3;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void tcp_connection(const char *message) {
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
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    if (recv(sockfd, buffer, sizeof(buffer)-1, 0) < 0) {
        close(sockfd);
        error("Error receiving response from server");
    }

    printf("Server response: %s\n", buffer);

    // Close the connection
    close(sockfd);
}

void udp_connection(const char *message) {
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
    char buffer[BUFFER_SIZE];
    socklen_t serverAddrLen = sizeof(serverAddr);
    ssize_t bytesRead = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&serverAddr, &serverAddrLen);
    if (bytesRead == -1) {
        perror("Error receiving response");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Null-terminate the received data
    buffer[bytesRead] = '\0';

    // Print the received message
    printf("Received response from server: %s\n", buffer);

    // Close the socket
    close(sockfd);
}

void setupWorkspace(){
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
}

int main(int argc, char *argv[]) {

    setupWorkspace();
    // Get input from the user and send to the server
    char buffer[BUFFER_SIZE];
    while (1) {
        printf("Enter message (type 'exit' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        
        udp_connection("olá");
    }


    return 0;
}
