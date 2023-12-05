#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include "utils.h"
#include "logged_in_linkedlist.h"

#define BUFFER_SIZE 1024
#define STANDARD_SERVER_PORT 58000 + 3

int server_tcp_socket;
int server_udp_socket;

// linkedlist to save logged-in users
Node *loggedInUsers = NULL;

// Signal handler function
void handle_sigint(int sig) {
    printf("Received CTRL+C. Closing socket and exiting.\n");

    // Close the server sockets
    close(server_tcp_socket);
    close(server_udp_socket);

    // Exit the program
    exit(EXIT_SUCCESS);
}


/*

    TCP CONNECTION FUNCTIONS

*/


void *handle_tcp_client(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Receive data from the client
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';

        // Check if the client wants to exit
        if (strcmp(buffer, "exit\n") == 0) {
            printf("Client requested to exit. Disconnecting.\n");
            break;
        }

        // Print the received data
        printf("Received from client: %s", buffer);

        // Send a response to the client
        send(client_socket, "Message received by server.\n", strlen("Message received by server.\n"), 0);
    }

    // Close the socket when done
    close(client_socket);
    free(arg);
    return NULL;
}


void manage_tcp_connections(){    // Accept and handle multiple clients using threads
    while (1) {

        // Set up client address information
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(server_tcp_socket, (struct sockaddr *)&client_address, &client_address_len);

        if (*client_socket == -1) {
            perror("Error accepting client connection");
            free(client_socket);
            continue;
        }

        printf("Connection accepted from %s:%d\n",
               inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Create a thread to handle the client
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_tcp_client, (void *)client_socket) != 0) {
            perror("Error creating thread");
            close(*client_socket);
            free(client_socket);
            continue;
        }

        // Detach the thread (allow it to clean up resources on exit)
        pthread_detach(thread);
    }
}

/*

    UDP CONNECTION FUNCTIONS

*/

void *manage_udp(void *arg) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while (1) {
        // Receive data from the UDP client
        bytes_received = recvfrom(server_udp_socket, buffer, BUFFER_SIZE - 1, 0,
                                  (struct sockaddr *)&client_address, &client_address_len);

        if (bytes_received == -1) {
            perror("Error receiving UDP data");
            continue;
        }

        buffer[bytes_received] = '\0';

        // Check if the client wants to exit
        if (strcmp(buffer, "exit\n") == 0) {
            printf("UDP Client requested to exit. Disconnecting.\n");
            break;
        }
    }

    return NULL;
}


/*

    MAIN FUNCTION

*/

int main(int argc, char *argv[]) {

    // Initial parameter setup 
    int port;
    if (argc == 1) {
        port = STANDARD_SERVER_PORT;
    } else if (argc == 3 && !strcmp(argv[1], "-p")) {
        port = atoi(argv[2]);
    } else {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create TCP socket
    server_tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_tcp_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Create UDP socket
    
    server_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);  // Use SOCK_DGRAM for UDP
    if (server_udp_socket == -1) {
        close(server_tcp_socket);
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup SIGINT handling
    signal(SIGINT, handle_sigint);

    // Bind the sockets
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_udp_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1 ||
        bind(server_tcp_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        
        perror("Socket binding failed");
        close(server_udp_socket);
        close(server_tcp_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections from both sockets
    if (listen(server_udp_socket, 5) == -1 || listen(server_tcp_socket, 5) == -1) {
        perror("Socket listening failed");
        close(server_udp_socket);
        close(server_tcp_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", port);

    manage_tcp_connections();
    pthread_t udp_thread;
    if (pthread_create(&udp_thread, NULL, manage_udp, NULL) != 0) {
        perror("Error creating UDP thread");
        return 1;
    }

    // Wait for udp management to finish (never reached, just here in case of bug that makes thread able to get out of manage_tcp_connections function)
    if (pthread_join(udp_thread, NULL) != 0) {
        perror("Error joining UDP thread");
        return 1;
    }

    // Close the server sockets (never reached, just here in case of bug that makes threads able to get out of "manage" functions)
    close(server_tcp_socket);

    return 0;
}