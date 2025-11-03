/*
 * OTT Streaming Server - JSON Utility Functions
 *
 * Simple JSON generation and parsing utilities for API responses
 * Enhancement Phase 3: Video Gallery & Watch History
 * Author: Generated for Network Programming Final Project
 * Date: 2025-11-03
 */

#ifndef JSON_H
#define JSON_H

#include <stddef.h>

/**
 * Parse integer value from JSON string
 * Example: parse_json_int("{\"video_id\":123}", "video_id") returns 123
 * Returns: parsed integer value, or -1 on error
 */
int parse_json_int(const char* json, const char* key);

/**
 * Parse string value from JSON string
 * Example: parse_json_string("{\"title\":\"Test\"}", "title", buffer, size)
 * Returns: 0 on success, -1 on error
 */
int parse_json_string(const char* json, const char* key, char* output, size_t max_len);

/**
 * Send JSON response with HTTP 200 OK header
 * Includes Content-Type: application/json header
 */
void send_json_response(int client_fd, const char* json_body);

/**
 * Send JSON error response with specified HTTP status code
 * Example: send_json_error(client_fd, 404, "Video not found")
 */
void send_json_error(int client_fd, int status_code, const char* error_message);

/**
 * Escape special characters in string for JSON
 * Handles: ", \, /, \b, \f, \n, \r, \t
 * Returns: 0 on success, -1 if output buffer too small
 */
int json_escape_string(const char* input, char* output, size_t max_len);

#endif // JSON_H
