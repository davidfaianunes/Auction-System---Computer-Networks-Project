#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 58000 + 3

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void handle_request(int server_socket) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Receive message from the client
    ssize_t bytes_received = recvfrom(server_socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_len);
    if (bytes_received < 0) {
        error("Error receiving message from client");
    }

    // Process the received message (you need to implement this based on your protocol)
    // For testing purposes, let's just print the received message
    buffer[bytes_received] = '\0';  // Null-terminate the received data
    printf("Received message from client: %s\n", buffer);

    // Send a response back to the client (for testing purposes)
    const char *response = "Server received your message";
    if (sendto(server_socket, response, strlen(response), 0, (struct sockaddr *)&client_addr, client_len) < 0) {
        error("Error sending response to client");
    }
}

int main() {
    int server_socket;
    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        error("Error opening socket");
    }

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Error binding socket");
    }

    printf("Server is listening on port %d...\n", SERVER_PORT);

    while (1) {
        // Handle the client's request
        handle_request(server_socket);
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
