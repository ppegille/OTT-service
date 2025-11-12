#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <fcntl.h>

// Configuration
#define PORT 8080
#define BUFFER_SIZE 65536  // 64KB for video streaming
#define MAX_PATH 512

// Buffer size constants (extracted from magic numbers)
#define MAX_FILENAME_LEN 256
#define MAX_TITLE_LEN 256
#define MAX_HEADER_LEN 128
#define MAX_ESCAPE_BUFFER 510
#define MAX_COMMAND_LEN 1024
#define MAX_JSON_BUFFER 8192
#define MAX_COOKIE_LEN 256

// Session configuration
#define SESSION_ID_LENGTH 33  // 32 hex chars + null terminator (128-bit secure random)
#define USER_ID_LENGTH 64
#define MAX_SESSIONS 100
#define SESSION_TIMEOUT 1800  // 30 minutes in seconds

// FFmpeg configuration
#define THUMBNAIL_DEFAULT_OFFSET 5    // Default thumbnail capture time (seconds)
#define THUMBNAIL_WIDTH 320            // Thumbnail width in pixels
#define THUMBNAIL_DIR "thumbnails"     // Thumbnail storage directory

// Status codes
#define STATUS_SUCCESS 0
#define STATUS_ERROR -1
#define STATUS_NOT_FOUND -2

// HTTP Response codes
#define HTTP_200_OK "HTTP/1.1 200 OK\r\n"
#define HTTP_206_PARTIAL "HTTP/1.1 206 Partial Content\r\n"
#define HTTP_302_FOUND "HTTP/1.1 302 Found\r\n"
#define HTTP_404_NOT_FOUND "HTTP/1.1 404 Not Found\r\n"
#define HTTP_416_RANGE_NOT_SATISFIABLE "HTTP/1.1 416 Range Not Satisfiable\r\n"

// Range request structure
typedef struct {
    long start;
    long end;
    int has_range;
} Range;

// HTTP request structure
typedef struct {
    char method[16];
    char path[MAX_PATH];
    char version[16];
    Range range;
} HTTPRequest;

// Session structure
typedef struct {
    char session_id[SESSION_ID_LENGTH];
    int user_id;           // Changed to INT for DB integration
    char username[USER_ID_LENGTH];
    time_t created_at;
    time_t last_accessed;
    int is_active;
} Session;

// Function declarations

// http.c
HTTPRequest parse_http_request(const char* request);
int find_header(const char* request, const char* header_name, char* value, size_t value_size);
const char* get_mime_type(const char* filename);
int is_path_safe(const char* path);
void send_404(int client_fd);
void send_403(int client_fd, const char* reason);

// streaming.c
Range parse_range(const char* range_header);
void stream_file(int client_fd, const char* filename, Range range);
long get_file_size(const char* filename);

// session.c
void init_session_store();
void cleanup_session_store();
void generate_session_id(char* session_id);
int create_session(int user_id, const char* username, char* session_id_out, size_t session_id_size);
int validate_session(const char* session_id);
void refresh_session(const char* session_id);
void destroy_session(const char* session_id);
void cleanup_expired_sessions();
int parse_cookie(const char* cookie_header, char* session_id, size_t session_id_size);
void generate_set_cookie_header(char* buffer, const char* session_id);
int parse_post_body(const char* body, const char* param_name, char* value, size_t value_size);
void redirect_to_login(int client_fd);
void handle_login(int client_fd, const char* request_body);
void send_login_error(int client_fd, const char* error_message);
int get_user_id_from_session(const char* session_id);
int get_username_from_session(const char* session_id, char* username_out, size_t username_size);

#endif // SERVER_H
