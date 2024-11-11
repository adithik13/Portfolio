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
//#define MAX_MESSAGES 20

struct Message { // separate name and message
    std::string username;
    std::string content;
}; 

std::vector<Message> messages; // where we will hold all the messages for the wall 

void command(int client_socket, int queue_size); 
void sendContent(int client_socket);
void clear(int client_socket);
void post(int client_socket, int queue_size);

int main(int argc, char const* argv[]) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // set a default port and queue size
    int port = PORT; 
    int queue_size = 3; 

    if (argc == 1){
        printf("Wall server running on port %d.\n", port);
        queue_size = 20;
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
        printf("Too many args\n");
    }

    // create the socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }
    
    // then we set socket options 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE); 
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);  

    // next bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { 
        perror("bind failed");
        exit(EXIT_FAILURE); 
    }

    // begin listening on the port w/ specified queue size
    if (listen(server_fd, queue_size) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //printf("Wall server running on port %d with queue size %d.\n", port, queue_size);

    // main loop to handle connection requests
    while (1) { 
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        command(client_socket, queue_size);
        close(client_socket);  // close the client socket after handling
    }
    close(server_fd);  // close the server socket when done
    return 0;
}

void command(int client_socket, int queue_size) {
    char buffer[1024] = {0}; // start as empty
    while (1) {
        sendContent(client_socket); // displays current wall content

        // addign an extra newline before the "Enter command" prompt
        send(client_socket, "\n", 1, 0);
        send(client_socket, COMMAND_PROMPT, strlen(COMMAND_PROMPT), 0);

        ssize_t valread = read(client_socket, buffer, sizeof(buffer));
        buffer[valread - 1] = '\0'; // remove newline

        if (strcmp(buffer, "clear") == 0) {
            clear(client_socket);
        } else if (strcmp(buffer, "post") == 0) {
            post(client_socket, queue_size);
        } else if (strcmp(buffer, "quit") == 0) {
            send(client_socket, QUIT_MESSAGE, strlen(QUIT_MESSAGE), 0);
            break;
        } else if (strcmp(buffer, "kill") == 0) {
            send(client_socket, KILL_MESSAGE, strlen(KILL_MESSAGE), 0);
            exit(0); // terminate entire server
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

void post(int client_socket, int queue_size) {
    char name[50] = {0};
    char message[70] = {0};

    // ask for the user's name
    send(client_socket, NAME_PROMPT, strlen(NAME_PROMPT), 0);
    ssize_t name_len = read(client_socket, name, sizeof(name) - 1);
    if (name_len > 0 && name[name_len - 1] == '\n') {
        name[name_len - 1] = '\0'; // remove the extra newline character
    }

    // prompt for the message content
    std::string prompt = POST_PROMPT1 + std::to_string(MAX_MESSAGE_LENGTH - strlen(name) - 2) + POST_PROMPT2;
    send(client_socket, prompt.c_str(), prompt.length(), 0);
    ssize_t message_len = read(client_socket, message, sizeof(message) - 1);
    if (message_len > 0 && message[message_len - 1] == '\n') {
        message[message_len - 1] = '\0'; // remove the extra newline character
    }

    // check the combined length
    if (strlen(name) + strlen(message) + 2 > MAX_MESSAGE_LENGTH) {
        send(client_socket, ERROR_MESSAGE, strlen(ERROR_MESSAGE), 0);

        // clear the input buffer so it doesn't think that's the next command
        char flush_buffer[1024];
        read(client_socket, flush_buffer, sizeof(flush_buffer));

        return;
    }

    // add the message to the wall (vect.)
    messages.push_back({name, message});

    // check number of msgs
    if (messages.size() > queue_size) {
        messages.erase(messages.begin()); // remove the oldest message if the wall is full
    }

    // send the success msg 
    send(client_socket, SUCCESS_MESSAGE, strlen(SUCCESS_MESSAGE), 0);
}
