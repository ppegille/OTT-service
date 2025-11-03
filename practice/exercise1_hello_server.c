#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    printf("=== Exercise 1: Basic HTTP Server ===\n\n");

    // Step 1: Create socket
    printf("Step 1: Creating socket...\n");
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    printf("✓ Socket created (fd=%d)\n\n", server_fd);

    // Allow immediate port reuse (helpful during development)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Step 2: Bind to port
    printf("Step 2: Binding to port %d...\n", PORT);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }
    printf("✓ Bound to port %d\n\n", PORT);

    // Step 3: Listen for connections
    printf("Step 3: Listening for connections...\n");
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }
    printf("✓ Listening (backlog=10)\n\n");

    printf("🚀 Server ready! Access http://localhost:%d/\n", PORT);
    printf("Press Ctrl+C to stop\n\n");

    // Step 4: Accept connections loop
    while (1) {
        printf("Waiting for connection...\n");

        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        char* client_ip = inet_ntoa(client_addr.sin_addr);
        printf("✓ Client connected: %s\n", client_ip);

        // Step 5: Read request
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            // Print first line of request
            char* newline = strchr(buffer, '\n');
            if (newline) *newline = '\0';
            printf("  Request: %s\n", buffer);
        }

        // Step 6: Send HTTP response
        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: 104\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body>"
            "<h1>Hello, World!</h1>"
            "<p>Exercise 1: Basic HTTP Server Working!</p>"
            "</body></html>";

        ssize_t bytes_sent = send(client_fd, response, strlen(response), 0);
        printf("  Sent %ld bytes\n", bytes_sent);

        // Step 7: Close connection
        close(client_fd);
        printf("  Connection closed\n\n");
    }

    close(server_fd);
    return 0;
}
