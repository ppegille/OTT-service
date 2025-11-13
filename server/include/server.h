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

// Import centralized configuration
#include "config.h"

// Backward compatibility aliases (deprecated, use config.h constants)
#define PORT SERVER_PORT
#define DB_PATH "../server/database/ott_server.db"

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
// is_path_safe() moved to validation.h
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
void handle_registration(int client_fd, const char* request_body);
void send_login_error(int client_fd, const char* error_message);
int get_user_id_from_session(const char* session_id);
int get_username_from_session(const char* session_id, char* username_out, size_t username_size);

#endif // SERVER_H
