/*
 * OTT Streaming Server - Session Management
 *
 * Cookie-based session management system
 * Author: Generated for Network Programming Final Project
 * Date: 2025-11-03
 */

#include "../include/server.h"

// Global session store
static Session session_store[MAX_SESSIONS];
static int session_count = 0;

/**
 * Initialize session store
 * Called at server startup
 */
void init_session_store() {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        session_store[i].is_active = 0;
        memset(session_store[i].session_id, 0, SESSION_ID_LENGTH);
        memset(session_store[i].user_id, 0, USER_ID_LENGTH);
    }
    session_count = 0;
    srand(time(NULL));  // Initialize random seed for session ID generation
    printf("✓ Session store initialized (max: %d sessions)\n", MAX_SESSIONS);
}

/**
 * Generate UUID-like session ID
 * Format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
 */
void generate_session_id(char* session_id) {
    const char* hex_chars = "0123456789abcdef";

    for (int i = 0; i < SESSION_ID_LENGTH - 1; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            session_id[i] = '-';
        } else if (i == 14) {
            session_id[i] = '4';  // UUID version 4
        } else if (i == 19) {
            session_id[i] = hex_chars[(rand() % 4) + 8];  // y = 8, 9, a, or b
        } else {
            session_id[i] = hex_chars[rand() % 16];
        }
    }
    session_id[SESSION_ID_LENGTH - 1] = '\0';
}

/**
 * Create new session for user
 * Returns: pointer to session_id on success, NULL on failure
 */
char* create_session(const char* user_id) {
    if (session_count >= MAX_SESSIONS) {
        printf("⚠️  Session store full, cleaning up expired sessions...\n");
        cleanup_expired_sessions();

        if (session_count >= MAX_SESSIONS) {
            printf("❌ Cannot create session: store still full\n");
            return NULL;
        }
    }

    // Find empty slot
    int slot = -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!session_store[i].is_active) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        printf("❌ Cannot create session: no available slot\n");
        return NULL;
    }

    // Generate session ID
    generate_session_id(session_store[slot].session_id);

    // Set session data
    strncpy(session_store[slot].user_id, user_id, USER_ID_LENGTH - 1);
    session_store[slot].user_id[USER_ID_LENGTH - 1] = '\0';
    session_store[slot].created_at = time(NULL);
    session_store[slot].last_accessed = time(NULL);
    session_store[slot].is_active = 1;

    session_count++;

    printf("✓ Session created: %s for user '%s' (total: %d)\n",
           session_store[slot].session_id, user_id, session_count);

    return session_store[slot].session_id;
}

/**
 * Validate session by session_id
 * Returns: 1 if valid, 0 if invalid
 */
int validate_session(const char* session_id) {
    if (!session_id || strlen(session_id) == 0) {
        return 0;
    }

    time_t now = time(NULL);

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (session_store[i].is_active &&
            strcmp(session_store[i].session_id, session_id) == 0) {

            // Check if session is expired
            if (now - session_store[i].last_accessed > SESSION_TIMEOUT) {
                printf("⏰ Session expired: %s (user: %s)\n",
                       session_id, session_store[i].user_id);
                session_store[i].is_active = 0;
                session_count--;
                return 0;
            }

            return 1;  // Valid session
        }
    }

    return 0;  // Session not found
}

/**
 * Refresh session (update last_accessed time)
 */
void refresh_session(const char* session_id) {
    if (!session_id) return;

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (session_store[i].is_active &&
            strcmp(session_store[i].session_id, session_id) == 0) {
            session_store[i].last_accessed = time(NULL);
            return;
        }
    }
}

/**
 * Destroy session (logout)
 */
void destroy_session(const char* session_id) {
    if (!session_id) return;

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (session_store[i].is_active &&
            strcmp(session_store[i].session_id, session_id) == 0) {
            printf("🚪 Session destroyed: %s (user: %s)\n",
                   session_id, session_store[i].user_id);
            session_store[i].is_active = 0;
            session_count--;
            return;
        }
    }
}

/**
 * Clean up expired sessions
 */
void cleanup_expired_sessions() {
    time_t now = time(NULL);
    int cleaned = 0;

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (session_store[i].is_active) {
            if (now - session_store[i].last_accessed > SESSION_TIMEOUT) {
                printf("🧹 Cleaning expired session: %s (user: %s)\n",
                       session_store[i].session_id, session_store[i].user_id);
                session_store[i].is_active = 0;
                session_count--;
                cleaned++;
            }
        }
    }

    if (cleaned > 0) {
        printf("✓ Cleaned %d expired sessions (remaining: %d)\n", cleaned, session_count);
    }
}

/**
 * Parse Cookie header and extract session_id
 * Cookie: session_id=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 * Returns: pointer to session_id (static buffer), or NULL if not found
 */
char* parse_cookie(const char* cookie_header) {
    static char session_id[SESSION_ID_LENGTH];

    if (!cookie_header) {
        return NULL;
    }

    // Find "session_id=" in cookie header
    const char* start = strstr(cookie_header, "session_id=");
    if (!start) {
        return NULL;
    }

    start += strlen("session_id=");

    // Extract session ID until semicolon or end of string
    int i = 0;
    while (start[i] != ';' && start[i] != '\0' && start[i] != '\r' && start[i] != '\n' && i < SESSION_ID_LENGTH - 1) {
        session_id[i] = start[i];
        i++;
    }
    session_id[i] = '\0';

    return session_id;
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
 * Returns: pointer to value (static buffer), or NULL if not found
 */
char* parse_post_body(const char* body, const char* param_name) {
    static char value[USER_ID_LENGTH];

    if (!body || !param_name) {
        return NULL;
    }

    // Find parameter in body
    char search_str[128];
    snprintf(search_str, sizeof(search_str), "%s=", param_name);

    const char* start = strstr(body, search_str);
    if (!start) {
        return NULL;
    }

    start += strlen(search_str);

    // Extract value until & or end of string
    int i = 0;
    while (start[i] != '&' && start[i] != '\0' && start[i] != '\r' && start[i] != '\n' && i < USER_ID_LENGTH - 1) {
        value[i] = start[i];
        i++;
    }
    value[i] = '\0';

    // URL decode if needed (for simplicity, just handle spaces)
    for (int j = 0; j < i; j++) {
        if (value[j] == '+') {
            value[j] = ' ';
        }
    }

    return value;
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
 * Handle POST /login request
 * Body format: user_id=USERNAME
 */
void handle_login(int client_fd, const char* request_body) {
    // Debug: print request body
    printf("  [DEBUG] POST body: '%s'\n", request_body ? request_body : "(null)");

    // Parse user_id from POST body
    char* user_id = parse_post_body(request_body, "user_id");

    printf("  [DEBUG] Parsed user_id: '%s'\n", user_id ? user_id : "(null)");

    if (!user_id || strlen(user_id) == 0) {
        // Invalid user_id, redirect back to login
        printf("❌ Login failed: empty user_id\n");
        redirect_to_login(client_fd);
        return;
    }

    // Create session
    char* session_id = create_session(user_id);

    if (!session_id) {
        // Session creation failed
        printf("❌ Login failed: cannot create session\n");
        redirect_to_login(client_fd);
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

    printf("✓ Login successful: %s → session %s\n", user_id, session_id);
}
