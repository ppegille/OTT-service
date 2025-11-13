/*
 * OTT Streaming Server - Session Management with Shared Memory
 *
 * POSIX Shared Memory + Semaphore for multi-process session sharing
 * Enhancement Phase 2: Database integration
 * Author: Generated for Network Programming Final Project
 * Date: 2025-11-03
 */

#include "../include/server.h"
#include "../include/database.h"
#include "../include/json.h"
#include "../include/validation.h"

// Shared memory structures
typedef struct {
    Session sessions[MAX_SESSIONS];
    int session_count;
} SharedSessionStore;

// Global variables
static int shm_id = -1;
static SharedSessionStore* session_store = NULL;
static sem_t* session_sem = NULL;

#define SEM_NAME "/ott_session_sem"

/**
 * Initialize shared memory session store
 * Called once by parent process at server startup
 */
void init_session_store() {
    // Create shared memory segment
    shm_id = shmget(IPC_PRIVATE,
                    sizeof(SharedSessionStore),
                    IPC_CREAT | 0666);

    if (shm_id < 0) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // Attach to shared memory
    session_store = (SharedSessionStore*)shmat(shm_id, NULL, 0);
    if (session_store == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // Initialize session store
    memset(session_store, 0, sizeof(SharedSessionStore));
    session_store->session_count = 0;

    // Create/open semaphore for synchronization
    // Remove existing semaphore first
    sem_unlink(SEM_NAME);

    session_sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (session_sem == SEM_FAILED) {
        perror("sem_open failed");
        shmdt(session_store);
        shmctl(shm_id, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));  // Initialize random seed

    printf("✓ Session store initialized (shared memory)\n");
    printf("  - Shared memory ID: %d\n", shm_id);
    printf("  - Max sessions: %d\n", MAX_SESSIONS);
    printf("  - Semaphore: %s\n", SEM_NAME);
}

/**
 * Cleanup shared memory and semaphore
 * Called at server shutdown
 */
void cleanup_session_store() {
    if (session_store != NULL) {
        shmdt(session_store);
        if (shm_id >= 0) {
            shmctl(shm_id, IPC_RMID, NULL);
        }
    }

    if (session_sem != NULL) {
        sem_close(session_sem);
        sem_unlink(SEM_NAME);
    }

    printf("✓ Session store cleaned up\n");
}

/**
 * Generate cryptographically secure session ID
 * Uses /dev/urandom for secure randomness (Unix/Linux/macOS)
 * Format: 32 hexadecimal characters (128 bits of entropy)
 */
void generate_session_id(char* session_id) {
    unsigned char random_bytes[SESSION_RANDOM_BYTES];  // 128 bits
    const char* hex_chars = "0123456789abcdef";

    // Open /dev/urandom for cryptographically secure random bytes
    FILE* urandom = fopen("/dev/urandom", "rb");
    if (!urandom) {
        // Fallback: use time + process ID for less secure but working alternative
        fprintf(stderr, "[WARNING] Cannot open /dev/urandom, using weak fallback\n");
        srand(time(NULL) ^ getpid());
        for (int i = 0; i < SESSION_RANDOM_BYTES; i++) {
            random_bytes[i] = (unsigned char)(rand() % MAX_RANDOM_BYTE_VALUE);
        }
    } else {
        // Read SESSION_RANDOM_BYTES random bytes from /dev/urandom
        size_t bytes_read = fread(random_bytes, 1, SESSION_RANDOM_BYTES, urandom);
        fclose(urandom);

        if (bytes_read != SESSION_RANDOM_BYTES) {
            fprintf(stderr, "[ERROR] Failed to read from /dev/urandom\n");
            // Fallback to weak randomness
            srand(time(NULL) ^ getpid());
            for (int i = 0; i < SESSION_RANDOM_BYTES; i++) {
                random_bytes[i] = (unsigned char)(rand() % MAX_RANDOM_BYTE_VALUE);
            }
        }
    }

    // Convert to hexadecimal string (SESSION_HEX_LENGTH chars + null terminator)
    for (int i = 0; i < SESSION_RANDOM_BYTES; i++) {
        session_id[i * HEX_CHARS_PER_BYTE] = hex_chars[(random_bytes[i] >> HEX_SHIFT_HIGH) & 0x0F];
        session_id[i * HEX_CHARS_PER_BYTE + 1] = hex_chars[random_bytes[i] & 0x0F];
    }
    session_id[SESSION_HEX_LENGTH] = '\0';
}

/**
 * Create new session for user
 * Thread-safe with semaphore
 * Returns: 1 on success, 0 on failure
 */
int create_session(int user_id, const char* username, char* session_id_out, size_t session_id_size) {
    if (!username || !session_id_out || session_id_size < SESSION_ID_LENGTH) {
        return 0;
    }

    // Lock semaphore
    sem_wait(session_sem);

    // Check if store is full
    if (session_store->session_count >= MAX_SESSIONS) {
        printf("⚠️  Session store full, cleaning up expired sessions...\n");

        // Cleanup without locking (already locked)
        time_t now = time(NULL);
        int cleaned = 0;
        for (int i = 0; i < MAX_SESSIONS; i++) {
            if (session_store->sessions[i].is_active) {
                if (now - session_store->sessions[i].last_accessed > SESSION_TIMEOUT) {
                    session_store->sessions[i].is_active = 0;
                    session_store->session_count--;
                    cleaned++;
                }
            }
        }

        if (cleaned > 0) {
            printf("✓ Cleaned %d expired sessions\n", cleaned);
        }

        if (session_store->session_count >= MAX_SESSIONS) {
            printf("❌ Cannot create session: store still full\n");
            sem_post(session_sem);  // Unlock
            return 0;
        }
    }

    // Find empty slot
    int slot = -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!session_store->sessions[i].is_active) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        printf("❌ Cannot create session: no available slot\n");
        sem_post(session_sem);  // Unlock
        return 0;
    }

    // Generate session ID
    generate_session_id(session_store->sessions[slot].session_id);

    // Set session data
    session_store->sessions[slot].user_id = user_id;
    strncpy(session_store->sessions[slot].username, username, USER_ID_LENGTH - 1);
    session_store->sessions[slot].username[USER_ID_LENGTH - 1] = '\0';
    session_store->sessions[slot].created_at = time(NULL);
    session_store->sessions[slot].last_accessed = time(NULL);
    session_store->sessions[slot].is_active = 1;

    session_store->session_count++;

    // Copy to caller's buffer before unlocking
    strncpy(session_id_out, session_store->sessions[slot].session_id, session_id_size - 1);
    session_id_out[session_id_size - 1] = '\0';

    printf("✓ Session created: %s for user '%s' (ID: %d, total: %d)\n",
           session_id_out, username, user_id, session_store->session_count);

    // Unlock semaphore
    sem_post(session_sem);

    return 1;
}

/**
 * Validate session by session_id
 * Thread-safe with semaphore
 * Returns: 1 if valid, 0 if invalid
 */
int validate_session(const char* session_id) {
    if (!session_id || strlen(session_id) == 0) {
        return 0;
    }

    // Lock semaphore
    sem_wait(session_sem);

    time_t now = time(NULL);
    int result = 0;

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (session_store->sessions[i].is_active &&
            strcmp(session_store->sessions[i].session_id, session_id) == 0) {

            // Check if session is expired
            if (now - session_store->sessions[i].last_accessed > SESSION_TIMEOUT) {
                printf("⏰ Session expired: %s (user: %s, ID: %d)\n",
                       session_id, session_store->sessions[i].username, session_store->sessions[i].user_id);
                session_store->sessions[i].is_active = 0;
                session_store->session_count--;
                result = 0;
            } else {
                result = 1;  // Valid session
            }
            break;
        }
    }

    // Unlock semaphore
    sem_post(session_sem);

    return result;
}

/**
 * Refresh session (update last_accessed time)
 * Thread-safe with semaphore
 */
void refresh_session(const char* session_id) {
    if (!session_id) return;

    // Lock semaphore
    sem_wait(session_sem);

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (session_store->sessions[i].is_active &&
            strcmp(session_store->sessions[i].session_id, session_id) == 0) {
            session_store->sessions[i].last_accessed = time(NULL);
            break;
        }
    }

    // Unlock semaphore
    sem_post(session_sem);
}

/**
 * Destroy session (logout)
 * Thread-safe with semaphore
 */
void destroy_session(const char* session_id) {
    if (!session_id) return;

    // Lock semaphore
    sem_wait(session_sem);

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (session_store->sessions[i].is_active &&
            strcmp(session_store->sessions[i].session_id, session_id) == 0) {
            printf("🚪 Session destroyed: %s (user: %s, ID: %d)\n",
                   session_id, session_store->sessions[i].username, session_store->sessions[i].user_id);
            session_store->sessions[i].is_active = 0;
            session_store->session_count--;
            break;
        }
    }

    // Unlock semaphore
    sem_post(session_sem);
}

/**
 * Clean up expired sessions
 * Thread-safe with semaphore
 */
void cleanup_expired_sessions() {
    // Lock semaphore
    sem_wait(session_sem);

    time_t now = time(NULL);
    int cleaned = 0;

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (session_store->sessions[i].is_active) {
            if (now - session_store->sessions[i].last_accessed > SESSION_TIMEOUT) {
                printf("🧹 Cleaning expired session: %s (user: %s, ID: %d)\n",
                       session_store->sessions[i].session_id,
                       session_store->sessions[i].username,
                       session_store->sessions[i].user_id);
                session_store->sessions[i].is_active = 0;
                session_store->session_count--;
                cleaned++;
            }
        }
    }

    if (cleaned > 0) {
        printf("✓ Cleaned %d expired sessions (remaining: %d)\n",
               cleaned, session_store->session_count);
    }

    // Unlock semaphore
    sem_post(session_sem);
}

/**
 * Parse Cookie header and extract session_id
 * Cookie: session_id=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 * Thread-safe: uses caller-provided buffer
 * Returns: 1 if found, 0 if not found
 */
int parse_cookie(const char* cookie_header, char* session_id, size_t session_id_size) {
    if (!cookie_header || !session_id || session_id_size == 0) {
        return 0;
    }

    // Find "session_id=" in cookie header
    const char* start = strstr(cookie_header, "session_id=");
    if (!start) {
        session_id[0] = '\0';
        return 0;
    }

    start += strlen("session_id=");

    // Extract session ID until semicolon or end of string
    size_t i = 0;
    while (start[i] != ';' && start[i] != '\0' && start[i] != '\r' && start[i] != '\n' && i < session_id_size - 1) {
        session_id[i] = start[i];
        i++;
    }
    session_id[i] = '\0';

    return (i > 0) ? 1 : 0;
}

/**
 * Generate Set-Cookie header
 * Set-Cookie: session_id=VALUE; Path=/; Max-Age=1800; HttpOnly
 */
void generate_set_cookie_header(char* buffer, const char* session_id) {
    snprintf(buffer, BUFFER_SIZE,
             "Set-Cookie: session_id=%s; Path=/; Max-Age=%d; HttpOnly\r\n",
             session_id, SESSION_TIMEOUT);
}

/**
 * Parse POST request body and extract parameter value
 * Body format: param1=value1&param2=value2
 * Thread-safe: uses caller-provided buffer
 * Returns: 1 if found, 0 if not found
 */
int parse_post_body(const char* body, const char* param_name, char* value, size_t value_size) {
    if (!body || !param_name || !value || value_size == 0) {
        return 0;
    }

    // Find parameter in body
    char search_str[128];
    snprintf(search_str, sizeof(search_str), "%s=", param_name);

    const char* start = strstr(body, search_str);
    if (!start) {
        value[0] = '\0';
        return 0;
    }

    start += strlen(search_str);

    // Extract value until & or end of string
    size_t i = 0;
    while (start[i] != '&' && start[i] != '\0' && start[i] != '\r' && start[i] != '\n' && i < value_size - 1) {
        value[i] = start[i];
        i++;
    }
    value[i] = '\0';

    // URL decode if needed (for simplicity, just handle spaces)
    for (size_t j = 0; j < i; j++) {
        if (value[j] == '+') {
            value[j] = ' ';
        }
    }

    return (i > 0) ? 1 : 0;
}

/**
 * Send 302 redirect to /login
 */
void redirect_to_login(int client_fd) {
    char response[BUFFER_SIZE];

    snprintf(response, sizeof(response),
             "%s"
             "Location: /login\r\n"
             "Content-Length: 0\r\n"
             "\r\n",
             HTTP_302_FOUND);

    write(client_fd, response, strlen(response));
}

/**
 * Send login error response
 */
void send_login_error(int client_fd, const char* error_message) {
    char response[BUFFER_SIZE];
    char html[2048];

    snprintf(html, sizeof(html),
             "<!DOCTYPE html>"
             "<html><head><meta charset='UTF-8'><title>Login Error</title>"
             "<style>body{font-family:Arial;display:flex;justify-content:center;align-items:center;min-height:100vh;background:#f5f5f5}"
             ".error{background:white;padding:40px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);text-align:center}"
             "h1{color:#e74c3c;margin-bottom:20px}p{color:#666;margin-bottom:30px}"
             "a{display:inline-block;padding:12px 30px;background:#3498db;color:white;text-decoration:none;border-radius:5px}"
             "a:hover{background:#2980b9}</style></head>"
             "<body><div class='error'><h1>❌ Login Failed</h1><p>%s</p>"
             "<a href='/login'>Try Again</a></div></body></html>",
             error_message);

    snprintf(response, sizeof(response),
             "%s"
             "Content-Type: text/html; charset=UTF-8\r\n"
             "Content-Length: %zu\r\n"
             "\r\n"
             "%s",
             HTTP_200_OK, strlen(html), html);

    write(client_fd, response, strlen(response));
}

/**
 * Handle POST /login request
 * Body format: username=USERNAME&password=PASSWORD
 */
void handle_login(int client_fd, const char* request_body) {
    // Debug: print request body
    printf("  [DEBUG] POST body: '%s'\n", request_body ? request_body : "(null)");

    // Parse username and password from POST body (thread-safe)
    char username[USER_ID_LENGTH];
    char password[USER_ID_LENGTH];

    if (!parse_post_body(request_body, "username", username, sizeof(username))) {
        username[0] = '\0';
    }

    if (!parse_post_body(request_body, "password", password, sizeof(password))) {
        password[0] = '\0';
    }

    printf("  [DEBUG] Parsed username: '%s'\n", username);
    printf("  [DEBUG] Parsed password: '%s'\n", password);

    if (strlen(username) == 0) {
        printf("❌ Login failed: empty username\n");
        send_login_error(client_fd, "Username is required");
        return;
    }

    if (strlen(password) == 0) {
        printf("❌ Login failed: empty password\n");
        send_login_error(client_fd, "Password is required");
        return;
    }

    // Authenticate with database
    int user_id;
    if (!authenticate_user(username, password, &user_id)) {
        printf("❌ Authentication failed for user '%s'\n", username);
        send_login_error(client_fd, "Invalid username or password");
        return;
    }

    // Update last login time
    update_last_login(user_id);

    // Create session (thread-safe)
    char session_id[SESSION_ID_LENGTH];
    if (!create_session(user_id, username, session_id, sizeof(session_id))) {
        printf("❌ Login failed: cannot create session\n");
        send_login_error(client_fd, "Server error: Cannot create session");
        return;
    }

    // Generate Set-Cookie header
    char set_cookie[BUFFER_SIZE];
    generate_set_cookie_header(set_cookie, session_id);

    // Send 302 redirect to / with Set-Cookie
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "%s"
             "Location: /\r\n"
             "%s"
             "Content-Length: 0\r\n"
             "\r\n",
             HTTP_302_FOUND,
             set_cookie);

    write(client_fd, response, strlen(response));

    printf("✓ Login successful: %s (ID: %d) → session %s\n", username, user_id, session_id);
}

/**
 * Get user_id from session_id
 * Returns: user_id (>0) on success, -1 on failure
 */
int get_user_id_from_session(const char* session_id) {
    if (!session_id || strlen(session_id) == 0) {
        return -1;
    }

    // Lock semaphore
    sem_wait(session_sem);

    int user_id = -1;

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (session_store->sessions[i].is_active &&
            strcmp(session_store->sessions[i].session_id, session_id) == 0) {
            user_id = session_store->sessions[i].user_id;
            break;
        }
    }

    // Unlock semaphore
    sem_post(session_sem);

    return user_id;
}

/**
 * Get username from session
 * Returns 0 on success, -1 on failure
 */
int get_username_from_session(const char* session_id, char* username_out, size_t username_size) {
    if (!session_id || strlen(session_id) == 0 || !username_out || username_size == 0) {
        return -1;
    }

    // Lock semaphore
    sem_wait(session_sem);

    int found = 0;

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (session_store->sessions[i].is_active &&
            strcmp(session_store->sessions[i].session_id, session_id) == 0) {
            strncpy(username_out, session_store->sessions[i].username, username_size - 1);
            username_out[username_size - 1] = '\0';
            found = 1;
            break;
        }
    }

    // Unlock semaphore
    sem_post(session_sem);

    return found ? 0 : -1;
}

// ============================================================================
// User Registration Functions
// ============================================================================
// Note: parse_json_string, validate_username, validate_password are now
//       imported from json.h and validation.h modules

/**
 * Handle user registration request
 */
void handle_registration(int client_fd, const char* request_body) {
    char username[USER_ID_LENGTH];
    char password[USER_ID_LENGTH];

    printf("  [Registration] Processing registration request\n");

    // Parse JSON body
    if (parse_json_string(request_body, "username", username, sizeof(username)) != 0) {
        send_json_error(client_fd, 400, "Invalid JSON: missing username");
        return;
    }

    if (parse_json_string(request_body, "password", password, sizeof(password)) != 0) {
        send_json_error(client_fd, 400, "Invalid JSON: missing password");
        return;
    }

    printf("  [Registration] Username: %s\n", username);

    // Validate username (validation.h returns bool: true = valid, false = invalid)
    if (!validate_username(username)) {
        send_json_error(client_fd, 400, "Invalid username: must be 2-63 chars, alphanumeric + underscore only");
        return;
    }

    // Validate password (validation.h returns bool: true = valid, false = invalid)
    if (!validate_password(password)) {
        send_json_error(client_fd, 400, "Invalid password: must be 8+ chars with letters and numbers");
        return;
    }

    // Create user in database
    int result = create_user(username, password);

    if (result == 0) {
        printf("  [Registration] ✅ User created successfully: %s\n", username);
        send_json_response(client_fd, "{\"status\":\"success\",\"message\":\"Registration successful\"}");
    } else {
        // Check if it's a duplicate username error
        fprintf(stderr, "  [Registration] ❌ Failed to create user: %s\n", username);
        send_json_error(client_fd, 409, "Username already exists");
    }
}
