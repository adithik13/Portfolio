#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>
#include "Messages.h" 

#define PORT 5514 // set an initial default port 
#define MAX_MESSAGE_LENGTH 80 // max message size from spec 
#define MAX_MESSAGES 20

struct Message { // separate name and message
    std::string username;
    std::string content;
}; 

std::vector<Message> messages; // where we will hold all the messages for the wall 

void command(int client_socket); 
void sendContent(int client_socket);
void clear(int client_socket);
void post(int client_socket);

int main(int argc, char const* argv[]) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Default port and queue size
    int port = PORT;  // Default port defined by PORT macro
    int queue_size = 3;  // Default queue size

    if (argc == 1){
        printf("Wall server running on port %d.\n", port);
    }
    // handling user overrides :
    if (argc == 2) {
        queue_size = std::stoi(argv[1]);  // override port if specified
        printf("Wall server running on port %d with queue size %d.\n", port, queue_size);

    }
    if (argc == 3) {
        queue_size = std::stoi(argv[1]);  // ovverride queue size if specified
        port = std::stoi(argv[2]);
        printf("Wall server running on port %d with queue size %d.\n", port, queue_size);
    }
    if (argc >3) {
        printf("Too many args what r u doing\n");
    }

    // Create the socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }
    
    // Set socket options 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE); 
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);  // Use the specified or default port

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { 
        perror("bind failed");
        exit(EXIT_FAILURE); 
    }

    // Start listening with the specified queue size
    if (listen(server_fd, queue_size) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //printf("Wall server running on port %d with queue size %d.\n", port, queue_size);

    // Main loop to handle incoming connection requests
    while (1) { 
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        command(client_socket);
        close(client_socket);  // Close the client socket after handling the command
    }
    close(server_fd);  // Close the server socket when done
    return 0;
}

void command(int client_socket) {
    char buffer[1024] = { 0 };
    while (1) {
        sendContent(client_socket); // show current wall content

        // add an extra newline before the "Enter command" prompt
        send(client_socket, "\n", 1, 0);
        send(client_socket, COMMAND_PROMPT, strlen(COMMAND_PROMPT), 0);

        ssize_t valread = read(client_socket, buffer, 1024); 
        buffer[valread - 1] = '\0'; // remove newline

        if (strcmp(buffer, "clear") == 0) {
            clear(client_socket);
        } else if (strcmp(buffer, "post") == 0) {
            post(client_socket);
        } else if (strcmp(buffer, "quit") == 0) {
            send(client_socket, QUIT_MESSAGE, strlen(QUIT_MESSAGE), 0);
            break;
        } else if (strcmp(buffer, "kill") == 0) {
            send(client_socket, KILL_MESSAGE, strlen(KILL_MESSAGE), 0);
            exit(0); // terminate the server
        } else {
            send(client_socket, "Unknown command.\n", strlen("Unknown command.\n"), 0);
        }
    }
}

void sendContent(int client_socket) {
    if (messages.empty()) {
        send(client_socket, WALL_HEADER, strlen(WALL_HEADER), 0);
        send(client_socket, EMPTY_MESSAGE, strlen(EMPTY_MESSAGE), 0);
    } else {
        send(client_socket, WALL_HEADER, strlen(WALL_HEADER), 0);
        for (const auto& msg : messages) {
            // make sure the whole message is on one line 
            std::string fullMsg = msg.username + ": " + msg.content + "\n";
            send(client_socket, fullMsg.c_str(), fullMsg.length(), 0);
        }
    }
    // send an extra newline after the wall contents
    send(client_socket, "\n", 1, 0);
}

void clear(int client_socket) {
    messages.clear();  
    send(client_socket, CLEAR_MESSAGE, strlen(CLEAR_MESSAGE), 0);
}

void post(int client_socket) {
    char name[50] = {0};
    char message[70] = {0};

    // ask for name and read message input
    send(client_socket, NAME_PROMPT, strlen(NAME_PROMPT), 0);
    ssize_t name_len = read(client_socket, name, 50);
    if (name_len > 0 && name[name_len - 1] == '\n') {
        name[name_len - 1] = '\0';  // delete the newline character
    }

    // prompt for message length and read input
    std::string prompt = POST_PROMPT1 + std::to_string(MAX_MESSAGE_LENGTH - strlen(name) - 2) + POST_PROMPT2;
    send(client_socket, prompt.c_str(), prompt.length(), 0);
    ssize_t message_len = read(client_socket, message, 70);
    if (message_len > 0 && message[message_len - 1] == '\n') {
        message[message_len - 1] = '\0';  // remove the newline character
    }

    // check message length
    if (strlen(name) + strlen(message) > MAX_MESSAGE_LENGTH) {
        send(client_socket, ERROR_MESSAGE, strlen(ERROR_MESSAGE), 0);
    } else {
        // add the message to the wall
        messages.push_back({name, message});
        if (messages.size() > MAX_MESSAGES) messages.erase(messages.begin());  // remove oldest if full
        send(client_socket, SUCCESS_MESSAGE, strlen(SUCCESS_MESSAGE), 0);
    }
}
