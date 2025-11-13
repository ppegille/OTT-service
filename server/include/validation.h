/*
 * OTT Streaming Server - Input Validation Module
 *
 * Centralized validation functions for security and data integrity.
 * Prevents common vulnerabilities: SQL injection, path traversal, XSS.
 *
 * Author: Network Programming Final Project
 * Date: 2025-11-13
 */

#ifndef VALIDATION_H
#define VALIDATION_H

#include "config.h"
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// Username & Password Validation
// ============================================================================

/**
 * Validate username format
 * Rules: 2-63 characters, alphanumeric and underscore only
 *
 * @param username Username to validate
 * @return true if valid, false otherwise
 */
bool validate_username(const char* username);

/**
 * Validate password strength
 * Rules: 8-255 characters, must contain letters and numbers
 *
 * @param password Password to validate
 * @return true if valid, false otherwise
 */
bool validate_password(const char* password);

/**
 * Get detailed validation error message
 *
 * @param username Username to check
 * @param error_buf Buffer to store error message
 * @param buf_size Size of error buffer
 * @return STATUS_SUCCESS or error code
 */
StatusCode validate_username_detailed(const char* username, char* error_buf, size_t buf_size);

/**
 * Get detailed password validation error message
 *
 * @param password Password to check
 * @param error_buf Buffer to store error message
 * @param buf_size Size of error buffer
 * @return STATUS_SUCCESS or error code
 */
StatusCode validate_password_detailed(const char* password, char* error_buf, size_t buf_size);

// ============================================================================
// Path & File Validation
// ============================================================================

/**
 * Check if path is safe (no directory traversal attacks)
 * Prevents: ../, ..\, absolute paths, null bytes
 *
 * @param path Path to validate
 * @return true if safe, false if potential security risk
 */
bool is_path_safe(const char* path);

/**
 * Validate file extension against allowed list
 *
 * @param filename Filename to check
 * @param allowed_extensions Array of allowed extensions (e.g., {".mp4", ".mkv", NULL})
 * @return true if extension is allowed, false otherwise
 */
bool validate_file_extension(const char* filename, const char** allowed_extensions);

/**
 * Sanitize filename by removing dangerous characters
 *
 * @param filename Original filename
 * @param safe_filename Buffer for sanitized filename
 * @param buf_size Size of output buffer
 * @return STATUS_SUCCESS or error code
 */
StatusCode sanitize_filename(const char* filename, char* safe_filename, size_t buf_size);

// ============================================================================
// SQL Injection Prevention
// ============================================================================

/**
 * Escape single quotes for SQL safety
 * Converts: ' -> ''
 *
 * @param input Input string
 * @param output Buffer for escaped string
 * @param buf_size Size of output buffer
 * @return STATUS_SUCCESS or STATUS_BUFFER_TOO_SMALL
 */
StatusCode escape_sql_string(const char* input, char* output, size_t buf_size);

/**
 * Validate SQL identifier (table/column name)
 * Allows: alphanumeric, underscore only
 *
 * @param identifier SQL identifier to validate
 * @return true if safe, false if potentially malicious
 */
bool validate_sql_identifier(const char* identifier);

// ============================================================================
// Integer & Numeric Validation
// ============================================================================

/**
 * Parse integer with bounds checking
 *
 * @param str String to parse
 * @param value Output value
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @return STATUS_SUCCESS or error code
 */
StatusCode parse_int_safe(const char* str, int* value, int min, int max);

/**
 * Parse video ID with validation
 *
 * @param str String to parse
 * @param video_id Output video ID
 * @return STATUS_SUCCESS or error code
 */
StatusCode parse_video_id(const char* str, int* video_id);

/**
 * Parse playback position with validation
 *
 * @param str String to parse
 * @param position Output position in seconds
 * @return STATUS_SUCCESS or error code
 */
StatusCode parse_position(const char* str, int* position);

// ============================================================================
// String Validation Utilities
// ============================================================================

/**
 * Check if string contains only printable ASCII characters
 *
 * @param str String to check
 * @return true if all characters are printable, false otherwise
 */
bool is_printable_ascii(const char* str);

/**
 * Check if string contains null bytes (security risk)
 *
 * @param str String to check
 * @param max_len Maximum length to scan
 * @return true if null byte found, false otherwise
 */
bool contains_null_byte(const char* str, size_t max_len);

/**
 * Validate string length
 *
 * @param str String to validate
 * @param min_len Minimum length (inclusive)
 * @param max_len Maximum length (inclusive)
 * @return true if length is valid, false otherwise
 */
bool validate_string_length(const char* str, size_t min_len, size_t max_len);

// ============================================================================
// HTTP Header Validation
// ============================================================================

/**
 * Validate Content-Type header
 *
 * @param content_type Content-Type header value
 * @param expected Expected content type (e.g., "application/json")
 * @return true if matches, false otherwise
 */
bool validate_content_type(const char* content_type, const char* expected);

/**
 * Validate HTTP method
 *
 * @param method HTTP method string
 * @return true if valid method (GET, POST, DELETE, etc.), false otherwise
 */
bool validate_http_method(const char* method);

// ============================================================================
// Session ID Validation
// ============================================================================

/**
 * Validate session ID format
 * Must be: 32 hex characters exactly
 *
 * @param session_id Session ID to validate
 * @return true if valid format, false otherwise
 */
bool validate_session_id(const char* session_id);

// ============================================================================
// Email Validation (Future Enhancement)
// ============================================================================

/**
 * Basic email format validation
 *
 * @param email Email address to validate
 * @return true if basic format is valid, false otherwise
 */
bool validate_email_format(const char* email);

#endif // VALIDATION_H
