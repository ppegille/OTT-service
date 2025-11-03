#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 4096

long get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

void send_404(int client_fd) {
    const char* response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 47\r\n"
        "\r\n"
        "<html><body><h1>404 Not Found</h1></body></html>";

    send(client_fd, response, strlen(response), 0);
}

void send_file(int client_fd, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("  ✗ File not found: %s\n", filename);
        send_404(client_fd);
        return;
    }

    long file_size = get_file_size(filename);
    printf("  File size: %ld bytes\n", file_size);

    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n",
        file_size);

    send(client_fd, header, strlen(header), 0);
    printf("  ✓ Sent header (%ld bytes)\n", strlen(header));

    char buffer[BUFFER_SIZE];
    size_t total_sent = 0;
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        ssize_t sent = send(client_fd, buffer, bytes_read, 0);
        if (sent > 0) {
            total_sent += sent;
        }
    }

    printf("  ✓ Sent content (%zu bytes)\n", total_sent);
    fclose(file);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    printf("=== Exercise 2: File Server ===\n\n");

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("🚀 Server ready on http://localhost:%d/\n", PORT);
    printf("Try: http://localhost:%d/test.txt\n\n", PORT);

    while (1) {
        printf("Waiting for connection...\n");

        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            continue;
        }

        printf("✓ Client connected\n");

        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            char method[16], path[256], version[16];
            sscanf(buffer, "%15s %255s %15s", method, path, version);

            printf("  %s %s\n", method, path);

            const char* filename = (path[0] == '/') ? path + 1 : path;

            if (strlen(filename) == 0) {
                filename = "test.txt";
            }

            send_file(client_fd, filename);
        }

        close(client_fd);
        printf("  Connection closed\n\n");
    }

    close(server_fd);
    return 0;
}
