/*
 * OTT Streaming Server - Enhancement Phase 3
 *
 * HTTP server with video gallery and watch history tracking
 * Author: Generated for Network Programming Final Project
 * Date: 2025-11-03
 */

#include "../include/server.h"
#include "../include/database.h"
#include "../include/json.h"
#include "../include/video_scanner.h"
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

    // Initialize session store
    printf("Step 2: Initializing session store...\n");
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

    // Step 1: Create socket
    printf("Step 1: Creating socket...\n");
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

    // Step 2: Bind to port
    printf("Step 2: Binding to port %d...\n", PORT);
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

    // Step 3: Listen for connections
    printf("Step 3: Listening for connections...\n");
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

            // Handle POST /login (authentication endpoint)
            if (strcmp(req.method, "POST") == 0 && strcmp(req.path, "/login") == 0) {
                printf("  [DEBUG] POST /login received\n");
                // Find request body (after \r\n\r\n)
                const char* body = strstr(buffer, "\r\n\r\n");
                if (body) {
                    body += 4;  // Skip \r\n\r\n
                    printf("  [DEBUG] Body found, calling handle_login\n");
                    handle_login(client_fd, body);
                } else {
                    printf("  [DEBUG] No body found, redirecting to login\n");
                    redirect_to_login(client_fd);
                }
                close(client_fd);
                printf("  [Child %d] Connection closed\n\n", getpid());
                exit(0);
            }

            // Handle GET /login (serve login page - no session required)
            if (strcmp(req.method, "GET") == 0 && strcmp(req.path, "/login") == 0) {
                req.range = (Range){0, 0, 0};
                stream_file(client_fd, "../client/login.html", req.range);
                close(client_fd);
                printf("  [Child %d] Connection closed\n\n", getpid());
                exit(0);
            }

            // For all other requests, check session
            const char* cookie_header = find_header(buffer, "Cookie");
            printf("  [DEBUG] Cookie header: '%s'\n", cookie_header ? cookie_header : "(null)");

            char* session_id = parse_cookie(cookie_header);
            printf("  [DEBUG] Parsed session_id: '%s'\n", session_id ? session_id : "(null)");

            if (!validate_session(session_id)) {
                // No valid session - redirect to login
                printf("  [Child %d] No valid session, redirecting to login\n", getpid());
                redirect_to_login(client_fd);
                close(client_fd);
                printf("  [Child %d] Connection closed\n\n", getpid());
                exit(0);
            }

            // Valid session - refresh it and serve requested content
            refresh_session(session_id);
            printf("  [Child %d] Valid session: %s\n", getpid(), session_id);

            // Handle API endpoints (Phase 3)
            // GET /api/videos - Return list of videos with watch history
            if (strcmp(req.method, "GET") == 0 && strcmp(req.path, "/api/videos") == 0) {
                char json_output[8192];

                // Get user_id from session
                int user_id = get_user_id_from_session(session_id);

                if (user_id < 0) {
                    send_json_error(client_fd, 401, "Unauthorized: Invalid session");
                } else if (get_videos_with_history(user_id, json_output, sizeof(json_output)) == 0) {
                    send_json_response(client_fd, json_output);
                } else {
                    send_json_error(client_fd, 500, "Internal server error");
                }

                close(client_fd);
                printf("  [Child %d] Connection closed\n\n", getpid());
                exit(0);
            }

            // POST /api/watch-progress - Update watch position
            if (strcmp(req.method, "POST") == 0 && strcmp(req.path, "/api/watch-progress") == 0) {
                // Get user_id from session
                int user_id = get_user_id_from_session(session_id);

                if (user_id < 0) {
                    send_json_error(client_fd, 401, "Unauthorized: Invalid session");
                } else {
                    // Find request body
                    const char* body = strstr(buffer, "\r\n\r\n");
                    if (body) {
                        body += 4;  // Skip \r\n\r\n

                        // Parse JSON body
                        int video_id = parse_json_int(body, "video_id");
                        int position = parse_json_int(body, "position");

                        if (video_id > 0 && position >= 0) {
                            if (update_watch_position(user_id, video_id, position) == 0) {
                                send_json_response(client_fd, "{\"status\":\"success\"}");
                                printf("  [API] Watch position updated: user=%d, video=%d, pos=%d\n",
                                       user_id, video_id, position);
                            } else {
                                send_json_error(client_fd, 500, "Failed to update watch position");
                            }
                        } else {
                            send_json_error(client_fd, 400, "Invalid video_id or position");
                        }
                    } else {
                        send_json_error(client_fd, 400, "Missing request body");
                    }
                }

                close(client_fd);
                printf("  [Child %d] Connection closed\n\n", getpid());
                exit(0);
            }

            // GET /api/watch-history/:video_id - Get watch history for specific video
            if (strcmp(req.method, "GET") == 0 && strncmp(req.path, "/api/watch-history/", 19) == 0) {
                // Get user_id from session
                int user_id = get_user_id_from_session(session_id);

                if (user_id < 0) {
                    send_json_error(client_fd, 401, "Unauthorized: Invalid session");
                } else {
                    // Extract video_id from path
                    int video_id = atoi(req.path + 19);

                    if (video_id > 0) {
                        char json_output[512];
                        if (get_watch_history_json(user_id, video_id, json_output, sizeof(json_output)) == 0) {
                            send_json_response(client_fd, json_output);
                        } else {
                            send_json_error(client_fd, 500, "Failed to get watch history");
                        }
                    } else {
                        send_json_error(client_fd, 400, "Invalid video_id");
                    }
                }

                close(client_fd);
                printf("  [Child %d] Connection closed\n\n", getpid());
                exit(0);
            }

            // Determine filename
            char video_path[MAX_PATH];
            char css_path[MAX_PATH];
            char js_path[MAX_PATH];
            char client_path[MAX_PATH];

            const char* filename;
            if (strcmp(req.path, "/") == 0) {
                // Default page: Video Gallery (Phase 3)
                filename = "../client/gallery.html";
            } else if (strncmp(req.path, "/videos/", 8) == 0) {
                snprintf(video_path, MAX_PATH, "../%s", req.path + 1);
                filename = video_path;
            } else if (strncmp(req.path, "/css/", 5) == 0) {
                snprintf(css_path, MAX_PATH, "../client/%s", req.path + 1);
                filename = css_path;
            } else if (strncmp(req.path, "/js/", 4) == 0) {
                snprintf(js_path, MAX_PATH, "../client/%s", req.path + 1);
                filename = js_path;
            } else {
                // Try to serve from client directory
                snprintf(client_path, MAX_PATH, "../client%s", req.path);
                filename = client_path;
            }

            // Check for Range header
            const char* range_header = find_header(buffer, "Range");
            req.range = parse_range(range_header);

            // Stream the file
            stream_file(client_fd, filename, req.range);

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
