/*
 * OTT Streaming Server - JSON Utility Implementation
 *
 * Simple JSON generation and parsing for API responses
 * Enhancement Phase 3: Video Gallery & Watch History
 * Author: Generated for Network Programming Final Project
 * Date: 2025-11-03
 */

#include "../include/json.h"
#include "../include/server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

/**
 * Parse integer value from JSON string
 * Simple parser for key-value pairs like "key":123
 * Returns: integer value or -1 on error
 */
int parse_json_int(const char* json, const char* key) {
    if (!json || !key) {
        return -1;
    }

    // Create search pattern: "key":
    char search_pattern[256];
    snprintf(search_pattern, sizeof(search_pattern), "\"%s\":", key);

    const char* pos = strstr(json, search_pattern);
    if (!pos) {
        return -1;
    }

    // Move past the key to the value
    pos += strlen(search_pattern);

    // Skip whitespace
    while (*pos && isspace(*pos)) {
        pos++;
    }

    // Parse integer
    return atoi(pos);
}

/**
 * Parse string value from JSON string
 * Simple parser for key-value pairs like "key":"value"
 * Returns: 0 on success, -1 on error
 */
int parse_json_string(const char* json, const char* key, char* output, size_t max_len) {
    if (!json || !key || !output || max_len == 0) {
        return -1;
    }

    // Create search pattern: "key":"
    char search_pattern[256];
    snprintf(search_pattern, sizeof(search_pattern), "\"%s\":\"", key);

    const char* start = strstr(json, search_pattern);
    if (!start) {
        return -1;
    }

    // Move past the key to the value start
    start += strlen(search_pattern);

    // Find closing quote
    const char* end = strchr(start, '"');
    if (!end) {
        return -1;
    }

    // Calculate length and copy
    size_t len = end - start;
    if (len >= max_len) {
        len = max_len - 1;
    }

    strncpy(output, start, len);
    output[len] = '\0';

    return 0;
}

/**
 * Escape special characters for JSON string
 * Handles: " \ / \b \f \n \r \t
 */
int json_escape_string(const char* input, char* output, size_t max_len) {
    if (!input || !output || max_len < 2) {
        return -1;
    }

    size_t out_pos = 0;
    const char* in_pos = input;

    while (*in_pos && out_pos < max_len - 2) {
        switch (*in_pos) {
            case '"':
                if (out_pos + 2 >= max_len) return -1;
                output[out_pos++] = '\\';
                output[out_pos++] = '"';
                break;
            case '\\':
                if (out_pos + 2 >= max_len) return -1;
                output[out_pos++] = '\\';
                output[out_pos++] = '\\';
                break;
            case '/':
                if (out_pos + 2 >= max_len) return -1;
                output[out_pos++] = '\\';
                output[out_pos++] = '/';
                break;
            case '\b':
                if (out_pos + 2 >= max_len) return -1;
                output[out_pos++] = '\\';
                output[out_pos++] = 'b';
                break;
            case '\f':
                if (out_pos + 2 >= max_len) return -1;
                output[out_pos++] = '\\';
                output[out_pos++] = 'f';
                break;
            case '\n':
                if (out_pos + 2 >= max_len) return -1;
                output[out_pos++] = '\\';
                output[out_pos++] = 'n';
                break;
            case '\r':
                if (out_pos + 2 >= max_len) return -1;
                output[out_pos++] = '\\';
                output[out_pos++] = 'r';
                break;
            case '\t':
                if (out_pos + 2 >= max_len) return -1;
                output[out_pos++] = '\\';
                output[out_pos++] = 't';
                break;
            default:
                output[out_pos++] = *in_pos;
                break;
        }
        in_pos++;
    }

    output[out_pos] = '\0';
    return 0;
}

/**
 * Send JSON response with HTTP 200 OK
 */
void send_json_response(int client_fd, const char* json_body) {
    char response[BUFFER_SIZE * 2];

    snprintf(response, sizeof(response),
             "%s"
             "Content-Type: application/json; charset=UTF-8\r\n"
             "Content-Length: %zu\r\n"
             "Cache-Control: no-cache\r\n"
             "\r\n"
             "%s",
             HTTP_200_OK,
             strlen(json_body),
             json_body);

    write(client_fd, response, strlen(response));
}

/**
 * Send JSON error response with status code
 */
void send_json_error(int client_fd, int status_code, const char* error_message) {
    char json_body[512];
    char response[BUFFER_SIZE];
    char escaped_msg[256];

    // Escape error message for JSON
    if (json_escape_string(error_message, escaped_msg, sizeof(escaped_msg)) != 0) {
        strncpy(escaped_msg, "Internal error", sizeof(escaped_msg) - 1);
    }

    // Create JSON error body
    snprintf(json_body, sizeof(json_body),
             "{\"status\":\"error\",\"code\":%d,\"message\":\"%s\"}",
             status_code, escaped_msg);

    // Determine HTTP status line
    const char* status_line;
    switch (status_code) {
        case 400:
            status_line = "HTTP/1.1 400 Bad Request\r\n";
            break;
        case 401:
            status_line = "HTTP/1.1 401 Unauthorized\r\n";
            break;
        case 404:
            status_line = "HTTP/1.1 404 Not Found\r\n";
            break;
        case 500:
            status_line = "HTTP/1.1 500 Internal Server Error\r\n";
            break;
        default:
            status_line = "HTTP/1.1 500 Internal Server Error\r\n";
            break;
    }

    // Build response
    snprintf(response, sizeof(response),
             "%s"
             "Content-Type: application/json; charset=UTF-8\r\n"
             "Content-Length: %zu\r\n"
             "\r\n"
             "%s",
             status_line,
             strlen(json_body),
             json_body);

    write(client_fd, response, strlen(response));
}
