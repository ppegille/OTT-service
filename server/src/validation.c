/*
 * OTT Streaming Server - Input Validation Implementation
 *
 * Security-focused validation functions with detailed error reporting.
 *
 * Author: Network Programming Final Project
 * Date: 2025-11-13
 */

#include "../include/validation.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

// ============================================================================
// Username & Password Validation
// ============================================================================

bool validate_username(const char* username) {
    if (!username) return false;

    size_t len = strlen(username);
    if (len < USERNAME_MIN_LENGTH || len > USERNAME_MAX_LENGTH) {
        return false;
    }

    // Check for valid characters: alphanumeric and underscore only
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!isalnum(c) && c != '_') {
            return false;
        }
    }

    return true;
}

bool validate_password(const char* password) {
    if (!password) return false;

    size_t len = strlen(password);
    if (len < PASSWORD_MIN_LENGTH || len > PASSWORD_MAX_LENGTH) {
        return false;
    }

    // Check for at least one letter and one number
    bool has_letter = false;
    bool has_number = false;

    for (size_t i = 0; i < len; i++) {
        if (isalpha(password[i])) has_letter = true;
        if (isdigit(password[i])) has_number = true;
    }

    return has_letter && has_number;
}

StatusCode validate_username_detailed(const char* username, char* error_buf, size_t buf_size) {
    if (!username || !error_buf) {
        return STATUS_INVALID_PARAMETER;
    }

    size_t len = strlen(username);

    if (len < USERNAME_MIN_LENGTH) {
        snprintf(error_buf, buf_size, "Username must be at least %d characters", USERNAME_MIN_LENGTH);
        return STATUS_INVALID_PARAMETER;
    }

    if (len > USERNAME_MAX_LENGTH) {
        snprintf(error_buf, buf_size, "Username must be at most %d characters", USERNAME_MAX_LENGTH);
        return STATUS_INVALID_PARAMETER;
    }

    // Check for valid characters
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!isalnum(c) && c != '_') {
            snprintf(error_buf, buf_size, "Username can only contain letters, numbers, and underscores");
            return STATUS_INVALID_PARAMETER;
        }
    }

    return STATUS_SUCCESS;
}

StatusCode validate_password_detailed(const char* password, char* error_buf, size_t buf_size) {
    if (!password || !error_buf) {
        return STATUS_INVALID_PARAMETER;
    }

    size_t len = strlen(password);

    if (len < PASSWORD_MIN_LENGTH) {
        snprintf(error_buf, buf_size, "Password must be at least %d characters", PASSWORD_MIN_LENGTH);
        return STATUS_INVALID_PARAMETER;
    }

    if (len > PASSWORD_MAX_LENGTH) {
        snprintf(error_buf, buf_size, "Password must be at most %d characters", PASSWORD_MAX_LENGTH);
        return STATUS_INVALID_PARAMETER;
    }

    bool has_letter = false;
    bool has_number = false;

    for (size_t i = 0; i < len; i++) {
        if (isalpha(password[i])) has_letter = true;
        if (isdigit(password[i])) has_number = true;
    }

    if (!has_letter) {
        snprintf(error_buf, buf_size, "Password must contain at least one letter");
        return STATUS_INVALID_PARAMETER;
    }

    if (!has_number) {
        snprintf(error_buf, buf_size, "Password must contain at least one number");
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

// ============================================================================
// Path & File Validation
// ============================================================================

bool is_path_safe(const char* path) {
    if (!path) {
        return false;
    }

    // Check for directory traversal patterns: ../ or ..\ (backslash)
    if (strstr(path, "../") != NULL || strstr(path, "..\\") != NULL) {
        return false;
    }

    // Check for encoded directory traversal patterns
    if (strstr(path, "%2e%2e") != NULL || strstr(path, "%2E%2E") != NULL) {
        return false;
    }

    // Note: HTTP URL paths starting with / are normal (e.g., /login, /api/videos)
    // We only check for directory traversal, not URL format

    return true;
}

bool validate_file_extension(const char* filename, const char** allowed_extensions) {
    if (!filename || !allowed_extensions) {
        return false;
    }

    // Find file extension
    const char* ext = strrchr(filename, '.');
    if (!ext || ext == filename) {
        return false; // No extension or starts with dot
    }

    // Check against allowed list
    for (int i = 0; allowed_extensions[i] != NULL; i++) {
        if (strcasecmp(ext, allowed_extensions[i]) == 0) {
            return true;
        }
    }

    return false;
}

StatusCode sanitize_filename(const char* filename, char* safe_filename, size_t buf_size) {
    if (!filename || !safe_filename || buf_size == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    size_t len = strlen(filename);
    if (len == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    if (len >= buf_size) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Copy filename, replacing dangerous characters
    size_t j = 0;
    for (size_t i = 0; i < len && j < buf_size - 1; i++) {
        char c = filename[i];

        // Allow: alphanumeric, dash, underscore, dot
        if (isalnum(c) || c == '-' || c == '_' || c == '.') {
            safe_filename[j++] = c;
        } else {
            // Replace with underscore
            safe_filename[j++] = '_';
        }
    }

    safe_filename[j] = '\0';
    return STATUS_SUCCESS;
}

// ============================================================================
// SQL Injection Prevention
// ============================================================================

StatusCode escape_sql_string(const char* input, char* output, size_t buf_size) {
    if (!input || !output || buf_size == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    size_t input_len = strlen(input);
    size_t j = 0;

    for (size_t i = 0; i < input_len; i++) {
        if (j >= buf_size - 2) { // Need space for potential escape + null
            return STATUS_BUFFER_TOO_SMALL;
        }

        if (input[i] == '\'') {
            // Escape single quote by doubling it
            output[j++] = '\'';
            output[j++] = '\'';
        } else {
            output[j++] = input[i];
        }
    }

    output[j] = '\0';
    return STATUS_SUCCESS;
}

bool validate_sql_identifier(const char* identifier) {
    if (!identifier || identifier[0] == '\0') {
        return false;
    }

    // First character must be letter or underscore
    if (!isalpha(identifier[0]) && identifier[0] != '_') {
        return false;
    }

    // Rest must be alphanumeric or underscore
    for (size_t i = 1; identifier[i] != '\0'; i++) {
        if (!isalnum(identifier[i]) && identifier[i] != '_') {
            return false;
        }
    }

    return true;
}

// ============================================================================
// Integer & Numeric Validation
// ============================================================================

StatusCode parse_int_safe(const char* str, int* value, int min, int max) {
    if (!str || !value) {
        return STATUS_INVALID_PARAMETER;
    }

    // Check for empty string
    if (str[0] == '\0') {
        return STATUS_INVALID_PARAMETER;
    }

    char* endptr;
    long parsed = strtol(str, &endptr, 10);

    // Check if parsing failed
    if (endptr == str || *endptr != '\0') {
        return STATUS_INVALID_PARAMETER;
    }

    // Check bounds
    if (parsed < min || parsed > max) {
        return STATUS_INVALID_PARAMETER;
    }

    *value = (int)parsed;
    return STATUS_SUCCESS;
}

StatusCode parse_video_id(const char* str, int* video_id) {
    return parse_int_safe(str, video_id, 1, 999999);
}

StatusCode parse_position(const char* str, int* position) {
    return parse_int_safe(str, position, 0, 86400); // Max 24 hours
}

// ============================================================================
// String Validation Utilities
// ============================================================================

bool is_printable_ascii(const char* str) {
    if (!str) return false;

    for (size_t i = 0; str[i] != '\0'; i++) {
        unsigned char c = (unsigned char)str[i];
        if (c < 32 || c > 126) {
            return false;
        }
    }

    return true;
}

bool contains_null_byte(const char* str, size_t max_len) {
    if (!str) return true;

    size_t len = strnlen(str, max_len);
    return len < max_len && str[len] == '\0' && len != strlen(str);
}

bool validate_string_length(const char* str, size_t min_len, size_t max_len) {
    if (!str) return false;

    size_t len = strlen(str);
    return len >= min_len && len <= max_len;
}

// ============================================================================
// HTTP Header Validation
// ============================================================================

bool validate_content_type(const char* content_type, const char* expected) {
    if (!content_type || !expected) {
        return false;
    }

    // Case-insensitive comparison
    return strcasecmp(content_type, expected) == 0;
}

bool validate_http_method(const char* method) {
    if (!method) return false;

    const char* valid_methods[] = {
        "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH", NULL
    };

    for (int i = 0; valid_methods[i] != NULL; i++) {
        if (strcmp(method, valid_methods[i]) == 0) {
            return true;
        }
    }

    return false;
}

// ============================================================================
// Session ID Validation
// ============================================================================

bool validate_session_id(const char* session_id) {
    if (!session_id) return false;

    size_t len = strlen(session_id);
    if (len != SESSION_HEX_LENGTH) {
        return false;
    }

    // Check if all characters are hex digits
    for (size_t i = 0; i < len; i++) {
        if (!isxdigit(session_id[i])) {
            return false;
        }
    }

    return true;
}

// ============================================================================
// Email Validation (Basic)
// ============================================================================

bool validate_email_format(const char* email) {
    if (!email) return false;

    size_t len = strlen(email);
    if (len < 3 || len > 254) { // RFC 5321
        return false;
    }

    // Must contain exactly one @
    const char* at_pos = strchr(email, '@');
    if (!at_pos || strchr(at_pos + 1, '@')) {
        return false;
    }

    // Must have characters before and after @
    if (at_pos == email || *(at_pos + 1) == '\0') {
        return false;
    }

    // Must have at least one dot after @
    if (!strchr(at_pos + 1, '.')) {
        return false;
    }

    return true;
}
