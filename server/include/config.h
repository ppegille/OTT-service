/*
 * OTT Streaming Server - Configuration Constants
 *
 * Centralized configuration for all magic numbers and constants.
 * This file consolidates all hardcoded values for easy maintenance.
 *
 * Author: Network Programming Final Project
 * Date: 2025-11-13
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Server Configuration
// ============================================================================

#define SERVER_PORT 8080
#define SERVER_BACKLOG 10           // Listen queue size

// ============================================================================
// Buffer Size Constants
// ============================================================================

#define BUFFER_SIZE 65536           // 64KB - Main buffer for video streaming
#define MAX_PATH 512                // Maximum file path length
#define MAX_FILENAME_LEN 256        // Maximum filename length
#define MAX_TITLE_LEN 256           // Maximum video title length
#define MAX_HEADER_LEN 128          // HTTP header field length
#define MAX_ESCAPE_BUFFER 510       // Escaped string buffer (2x - 2)
#define MAX_COMMAND_LEN 1024        // System command buffer
#define MAX_JSON_BUFFER 8192        // Large JSON response buffer
#define MAX_JSON_SMALL_BUFFER 512   // Small JSON/string buffer
#define MAX_COOKIE_LEN 256          // Cookie header buffer

// ============================================================================
// Session Management Configuration
// ============================================================================

#define SESSION_RANDOM_BYTES 16             // 128 bits of entropy
#define SESSION_HEX_LENGTH 32               // 16 bytes * 2 hex chars
#define SESSION_ID_LENGTH 33                // 32 hex chars + null terminator
#define MAX_SESSIONS 100                    // Maximum concurrent sessions
#define SESSION_TIMEOUT 1800                // 30 minutes in seconds
#define SESSION_CLEANUP_INTERVAL 300        // Cleanup every 5 minutes

// ============================================================================
// User Authentication Configuration
// ============================================================================

#define USERNAME_MIN_LENGTH 2       // Minimum username length
#define USERNAME_MAX_LENGTH 63      // Maximum username length
#define PASSWORD_MIN_LENGTH 8       // Minimum password length
#define PASSWORD_MAX_LENGTH 255     // Maximum password length
#define USER_ID_LENGTH 64           // User ID/username buffer size

// ============================================================================
// Random Number Generation
// ============================================================================

#define MAX_RANDOM_BYTE_VALUE 256   // Random byte range (0-255)
#define HEX_CHARS_PER_BYTE 2        // 1 byte = 2 hex characters
#define HEX_SHIFT_HIGH 4            // Bit shift for high nibble

// ============================================================================
// FFmpeg & Thumbnail Configuration
// ============================================================================

#define THUMBNAIL_DEFAULT_OFFSET 5  // Default thumbnail capture time (seconds)
#define THUMBNAIL_WIDTH 320         // Thumbnail width in pixels
#define THUMBNAIL_HEIGHT -1         // Auto-calculate height (aspect ratio)
#define THUMBNAIL_DIR "thumbnails"  // Thumbnail storage directory
#define VIDEO_DIR "videos"          // Video files directory

// ============================================================================
// Database Configuration
// ============================================================================

#define DB_PATH "../server/database/ott_server.db"
#define DB_RETRY_COUNT 3            // Number of retries for DB operations
#define DB_BUSY_TIMEOUT 5000        // SQLite busy timeout (ms)

// ============================================================================
// HTTP Configuration
// ============================================================================

#define HTTP_METHOD_MAX_LEN 16      // Maximum HTTP method length
#define HTTP_VERSION_MAX_LEN 16     // Maximum HTTP version length
#define HTTP_RESPONSE_HEADER_SIZE 1024  // HTTP response header buffer

// ============================================================================
// Video Streaming Configuration
// ============================================================================

#define DEFAULT_STREAM_CHUNK_SIZE 65536     // 64KB chunks for streaming
#define MIN_STREAM_CHUNK_SIZE 4096          // Minimum 4KB chunk
#define MAX_STREAM_CHUNK_SIZE 1048576       // Maximum 1MB chunk

// ============================================================================
// JSON Configuration
// ============================================================================

#define JSON_ARRAY_INITIAL_CAPACITY 20      // Initial array size for JSON
#define JSON_OBJECT_MAX_FIELDS 50           // Maximum fields in JSON object
#define JSON_STRING_ESCAPE_OVERHEAD 2       // Buffer overhead for escaping
#define JSON_SAFETY_MARGIN 10               // Buffer overflow prevention margin

// ============================================================================
// URL Encoding/Decoding
// ============================================================================

#define URL_ENCODE_SPACE_CHAR '+'           // URL encoded space character
#define URL_DECODE_BUFFER_OVERHEAD 10       // Extra space for URL decoding

// ============================================================================
// System Limits
// ============================================================================

#define MAX_CONCURRENT_CHILDREN 100         // Max child processes
#define CHILD_PROCESS_TIMEOUT 300           // Child process timeout (seconds)

// ============================================================================
// Logging Configuration
// ============================================================================

#define LOG_BUFFER_SIZE 2048        // Log message buffer
#define LOG_TIMESTAMP_SIZE 32       // Timestamp string size

// ============================================================================
// HLS (HTTP Live Streaming) Configuration
// ============================================================================

#define HLS_SEGMENT_DURATION 10     // HLS segment length (seconds)
#define HLS_OUTPUT_DIR "hls"        // HLS output directory
#define HLS_MAX_BITRATE 5000        // Maximum bitrate (kbps)

// ============================================================================
// Performance Tuning
// ============================================================================

#define SOCKET_SEND_BUFFER_SIZE 262144      // 256KB socket send buffer
#define SOCKET_RECV_BUFFER_SIZE 65536       // 64KB socket receive buffer
#define FILE_READ_BUFFER_SIZE 65536         // 64KB file read buffer

// ============================================================================
// Error Messages Maximum Lengths
// ============================================================================

#define MAX_ERROR_MESSAGE_LEN 256   // Maximum error message length

// ============================================================================
// Status Codes (Enum for type safety)
// ============================================================================

typedef enum {
    STATUS_SUCCESS = 0,                // Operation succeeded
    STATUS_ERROR = -1,                 // General error
    STATUS_NOT_FOUND = -2,             // Resource not found
    STATUS_INVALID_PARAMETER = -3,     // Invalid function parameter
    STATUS_PERMISSION_DENIED = -4,     // Access denied
    STATUS_BUFFER_TOO_SMALL = -5,      // Buffer size insufficient
    STATUS_DATABASE_ERROR = -6,        // Database operation failed
    STATUS_NETWORK_ERROR = -7,         // Network operation failed
    STATUS_TIMEOUT = -8,               // Operation timed out
    STATUS_ALREADY_EXISTS = -9,        // Resource already exists
    STATUS_NOT_INITIALIZED = -10,      // Component not initialized
    STATUS_INVALID_STATE = -11,        // Invalid state for operation
    STATUS_OUT_OF_MEMORY = -12,        // Memory allocation failed
    STATUS_AUTHENTICATION_FAILED = -13, // Auth credentials invalid
    STATUS_SESSION_EXPIRED = -14        // Session has expired
} StatusCode;

// ============================================================================
// HTTP Response Codes
// ============================================================================

#define HTTP_200_OK "HTTP/1.1 200 OK\r\n"
#define HTTP_206_PARTIAL "HTTP/1.1 206 Partial Content\r\n"
#define HTTP_302_FOUND "HTTP/1.1 302 Found\r\n"
#define HTTP_400_BAD_REQUEST "HTTP/1.1 400 Bad Request\r\n"
#define HTTP_401_UNAUTHORIZED "HTTP/1.1 401 Unauthorized\r\n"
#define HTTP_403_FORBIDDEN "HTTP/1.1 403 Forbidden\r\n"
#define HTTP_404_NOT_FOUND "HTTP/1.1 404 Not Found\r\n"
#define HTTP_409_CONFLICT "HTTP/1.1 409 Conflict\r\n"
#define HTTP_416_RANGE_NOT_SATISFIABLE "HTTP/1.1 416 Range Not Satisfiable\r\n"
#define HTTP_500_INTERNAL_ERROR "HTTP/1.1 500 Internal Server Error\r\n"

#endif // CONFIG_H
