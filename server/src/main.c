/*
 * OTT Streaming Server - Enhancement Phase 3 (Refactored)
 *
 * HTTP server with video gallery and watch history tracking
 * Refactored with table-driven routing system
 *
 * Author: Generated for Network Programming Final Project
 * Date: 2025-11-03
 * Refactored: 2025-11-13
 */

#include "../include/server.h"
#include "../include/database.h"
#include "../include/json.h"
#include "../include/routes.h"
#include "../include/video_scanner.h"
#include "../include/ffmpeg_utils.h"
#include "../include/validation.h"
#include <signal.h>
#include <sys/wait.h>

// Signal handler for child process cleanup (zombie prevention)
void sigchld_handler(int sig) {
    (void)sig;  // Unused parameter
    // Reap all dead child processes
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// Signal handler for graceful shutdown
void sigint_handler(int sig) {
    (void)sig;
    printf("\n\n🛑 Shutting down server...\n");
    cleanup_session_store();
    close_database();
    printf("✓ Server stopped\n");
    exit(0);
}

// Signal handler for server cleanup on abnormal termination
void cleanup_handler(void) {
    cleanup_session_store();
    close_database();
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    printf("=== OTT Streaming Server - Enhancement Phase 3 ===\n");
    printf("    (Video Gallery & Watch History Tracking)\n\n");

    // Initialize database
    printf("Step 0: Initializing database...\n");
    if (init_database(DB_PATH) != 0) {
        fprintf(stderr, "Failed to initialize database\n");
        exit(EXIT_FAILURE);
    }
    printf("\n");

    // Scan and register videos automatically
    printf("Step 1: Scanning video directory...\n");
    scan_and_register_videos("../videos");
    printf("\n");

    // Extract video metadata (duration, thumbnails) using FFmpeg
    printf("Step 2: Extracting video metadata...\n");
    update_all_video_metadata("../videos");
    printf("\n");

    // Initialize session store
    printf("Step 3: Initializing session store...\n");
    init_session_store();
    printf("\n");

    // Set up signal handler for child process cleanup
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Set up signal handler for graceful shutdown (Ctrl+C)
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    atexit(cleanup_handler);

    // Step 4: Create socket
    printf("Step 4: Creating socket...\n");
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("✓ Socket created successfully\n\n");

    // Allow port reuse (important for development)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Step 5: Bind to port
    printf("Step 5: Binding to port %d...\n", PORT);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("✓ Successfully bound to port %d\n\n", PORT);

    // Step 6: Listen for connections
    printf("Step 6: Listening for connections...\n");
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("✓ Server is listening\n\n");

    printf("🚀 OTT Streaming Server is running!\n");
    printf("   Access the player at: http://localhost:%d/\n", PORT);
    printf("   Press Ctrl+C to stop the server\n\n");

    // Main server loop
    while (1) {
        printf("Waiting for connection...\n");

        // Accept client connection
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        char* client_ip = inet_ntoa(client_addr.sin_addr);
        printf("✓ Client connected: %s (PID: %d)\n", client_ip, getpid());

        // Fork a child process to handle this client
        pid_t pid = fork();

        if (pid < 0) {
            // Fork failed
            perror("Fork failed");
            close(client_fd);
            continue;
        }

        if (pid == 0) {
            // Child process: handle the client request
            close(server_fd);  // Child doesn't need the listening socket

            // Read HTTP request
            ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                close(client_fd);
                exit(0);  // Exit child process
            }

            buffer[bytes_read] = '\0';

            // Parse HTTP request
            HTTPRequest req = parse_http_request(buffer);
            printf("  [Child %d] %s %s\n", getpid(), req.method, req.path);

            // Security: Validate path to prevent directory traversal attacks
            if (!is_path_safe(req.path)) {
                send_403(client_fd, "Directory traversal attempt detected");
                close(client_fd);
                printf("  [Child %d] Connection closed (security violation)\n\n", getpid());
                exit(0);
            }

            // Parse session from Cookie header
            char cookie_header[MAX_COOKIE_LEN];
            char session_id[SESSION_ID_LENGTH];

            if (!find_header(buffer, "Cookie", cookie_header, sizeof(cookie_header))) {
                cookie_header[0] = '\0';
            }

            if (!parse_cookie(cookie_header, session_id, sizeof(session_id))) {
                session_id[0] = '\0';
            }

            // Validate and refresh session (if present)
            if (session_id[0] != '\0' && validate_session(session_id)) {
                refresh_session(session_id);
                printf("  [Child %d] Valid session: %s\n", getpid(), session_id);
            } else {
                session_id[0] = '\0';  // Clear invalid session
            }

            // Dispatch request to appropriate route handler
            if (dispatch_route(client_fd, &req, session_id, buffer)) {
                // Route was handled successfully
                printf("  [Child %d] Route handled successfully\n", getpid());
            } else {
                // No matching route found - send 404
                printf("  [Child %d] No matching route for %s %s\n", getpid(), req.method, req.path);
                send_404(client_fd);
            }

            // Close connection and exit child process
            close(client_fd);
            printf("  [Child %d] Connection closed\n\n", getpid());
            exit(0);  // Child process exits here
        }

        // Parent process: close client socket and continue accepting
        close(client_fd);  // Parent doesn't need the client socket
    }

    close(server_fd);
    return 0;
}
