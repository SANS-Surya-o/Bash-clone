#include "shell.h"
#include "builtins.h"
#include <arpa/inet.h>
#include <netdb.h>



#define HOST "man.he.net"
#define PORT 80


#define PACKET_LEN 4096


//// Copilot generated :: 
void strip_tags_and_print(const char *str) {
    int in_tag = 0;  // Flag to check if we are inside a tag

    // Iterate through each character in the string
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '<') {
            in_tag = 1;  // Start of a tag
        } else if (str[i] == '>') {
            in_tag = 0;  // End of a tag
        } else if (!in_tag) {
            putchar(str[i]);  // Print the character if we are not inside a tag
        }
    }
}


const char* SUB_SECTION = "/?topic=%s&section=all";

int fetch_man_page(char *pageName) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char name[256];
    sprintf(name, SUB_SECTION, pageName);
    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // Get server's IP address
    server = gethostbyname(HOST);
    if (server == NULL) {

        fprintf(stderr,RED"No such host\n"RESET);
        return -1;
    }

    // Initialize server_addr struct
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,
          server->h_length);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0) {
        // perror("Error connecting to server");
        printf(RED"Could not connect to server\n"RESET);
        return -1;
    }

    // Send the HTTP GET request
    char request[PACKET_LEN];
    sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", name, HOST);
    if (send(sockfd, request, strlen(request), 0) < 0) {
        // perror("Error sending request");
        printf(RED"Could not send request"RESET);
        return -1;
    }

    // Receive and print the response
    char response[PACKET_LEN];
    int pkts_recv = 0;
    
    while (1) {
        pkts_recv++;
        int n_bytes = recv(sockfd, response, sizeof(response) - 1, 0);
        if (n_bytes < 0) {
            
            fprintf(stderr,RED"Could not receive response\n"RESET);
            return -1;
        } else if (n_bytes == 0) {
            break;  // End of response
        } else {
            response[n_bytes] = '\0';  // Null-terminate the response
            // Check for "NAME"; the first word
            if (pkts_recv == 1) {
                char *ptr = strstr(response, "NAME\n");
                if (ptr == NULL) {
                    printf(RED"Could not find man page\n"RESET);
                    return -1;
                } else {
                    // Strip tags and print the content of the first part
                    strip_tags_and_print(ptr);
                }
            } else {
                // Strip tags and print the rest of the content
                strip_tags_and_print(response);
            }
        }
    }

    // Close the socket
    close(sockfd);

    return -1;
}



int iMan(cmd* command)
{
    if (command==NULL || command->tokens==NULL || command->tokens[0]==NULL)
    {
        return -1;
    }
    if (command->tokens[1]==NULL)
    {
        printf(RED"iMan: missing operand\n"RESET);
        return -1;
    }
    fetch_man_page(command->tokens[1]);
    return 1;
}


