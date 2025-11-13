/*
 * OTT Streaming Server - JSON Builder Module
 *
 * Reusable JSON generation utilities to reduce code duplication
 * and provide safer JSON construction with automatic escaping.
 *
 * Author: Network Programming Final Project
 * Date: 2025-11-13
 */

#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <stddef.h>

// JSON Builder state machine
typedef struct {
    char* buffer;               // Output buffer
    size_t capacity;            // Buffer capacity
    size_t offset;              // Current write position
    int element_count;          // Elements in current array/object
    int error;                  // Error flag
} JSONBuilder;

// ============================================================================
// Initialization and Finalization
// ============================================================================

/**
 * Initialize JSON builder with buffer
 * @param builder Builder instance to initialize
 * @param buffer Output buffer
 * @param capacity Buffer size
 */
void json_builder_init(JSONBuilder* builder, char* buffer, size_t capacity);

/**
 * Check if builder has encountered an error
 * @return 1 if error occurred, 0 otherwise
 */
int json_builder_has_error(const JSONBuilder* builder);

/**
 * Get remaining buffer space
 * @return Bytes remaining in buffer
 */
size_t json_builder_remaining(const JSONBuilder* builder);

// ============================================================================
// Object and Array Construction
// ============================================================================

/**
 * Start a new JSON object: {
 */
void json_builder_start_object(JSONBuilder* builder);

/**
 * End current JSON object: }
 */
void json_builder_end_object(JSONBuilder* builder);

/**
 * Start a new JSON array: [
 */
void json_builder_start_array(JSONBuilder* builder);

/**
 * End current JSON array: ]
 */
void json_builder_end_array(JSONBuilder* builder);

// ============================================================================
// Named Field Writers (for objects)
// ============================================================================

/**
 * Add a string field: "key":"value"
 * Automatically escapes the value for JSON safety
 */
void json_builder_add_string(JSONBuilder* builder, const char* key, const char* value);

/**
 * Add an integer field: "key":123
 */
void json_builder_add_int(JSONBuilder* builder, const char* key, int value);

/**
 * Add a long integer field: "key":1234567890
 */
void json_builder_add_long(JSONBuilder* builder, const char* key, long value);

/**
 * Add a boolean field: "key":true or "key":false
 */
void json_builder_add_bool(JSONBuilder* builder, const char* key, int value);

/**
 * Add a null field: "key":null
 */
void json_builder_add_null(JSONBuilder* builder, const char* key);

// ============================================================================
// Array Element Writers (for arrays)
// ============================================================================

/**
 * Add a string element: "value"
 * Automatically escapes the value for JSON safety
 */
void json_builder_add_string_element(JSONBuilder* builder, const char* value);

/**
 * Add an integer element: 123
 */
void json_builder_add_int_element(JSONBuilder* builder, int value);

// ============================================================================
// Composite Builders for Common Patterns
// ============================================================================

/**
 * Add a complete video object with all standard fields
 * Automatically handles JSON escaping for string fields
 *
 * @param builder Builder instance
 * @param video_id Video ID
 * @param title Video title (will be escaped)
 * @param filename Video filename (will be escaped)
 * @param thumbnail Thumbnail path (will be escaped, can be NULL or empty)
 * @param duration Duration in seconds
 * @param file_size File size in bytes
 * @param watched Whether video has been watched
 * @param last_position Last playback position in seconds
 */
void json_builder_add_video_object(
    JSONBuilder* builder,
    int video_id,
    const char* title,
    const char* filename,
    const char* thumbnail,
    int duration,
    long file_size,
    int watched,
    int last_position
);

/**
 * Start a named array field: "key":[
 */
void json_builder_start_array_field(JSONBuilder* builder, const char* key);

// ============================================================================
// Low-Level Helpers
// ============================================================================

/**
 * Add a separator comma if needed (internal helper)
 */
void json_builder_add_separator(JSONBuilder* builder);

/**
 * Add raw text without escaping (use with caution)
 */
void json_builder_add_raw(JSONBuilder* builder, const char* text);

#endif // JSON_BUILDER_H
