/*
 * OTT Streaming Server - JSON Builder Implementation
 *
 * Provides reusable JSON generation with automatic escaping and
 * buffer management.
 *
 * Author: Network Programming Final Project
 * Date: 2025-11-13
 */

#include "../include/json_builder.h"
#include "../include/json.h"  // For json_escape_string
#include "../include/config.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// Initialization and Status
// ============================================================================

void json_builder_init(JSONBuilder* builder, char* buffer, size_t capacity) {
    builder->buffer = buffer;
    builder->capacity = capacity;
    builder->offset = 0;
    builder->element_count = 0;
    builder->error = 0;

    if (buffer && capacity > 0) {
        buffer[0] = '\0';
    }
}

int json_builder_has_error(const JSONBuilder* builder) {
    return builder->error;
}

size_t json_builder_remaining(const JSONBuilder* builder) {
    if (builder->offset >= builder->capacity) {
        return 0;
    }
    return builder->capacity - builder->offset;
}

// ============================================================================
// Low-Level Helpers
// ============================================================================

void json_builder_add_separator(JSONBuilder* builder) {
    if (builder->error || builder->element_count == 0) {
        return;
    }

    if (json_builder_remaining(builder) < 2) {
        builder->error = 1;
        return;
    }

    builder->offset += snprintf(
        builder->buffer + builder->offset,
        builder->capacity - builder->offset,
        ","
    );
}

void json_builder_add_raw(JSONBuilder* builder, const char* text) {
    if (builder->error || !text) {
        return;
    }

    size_t remaining = json_builder_remaining(builder);
    if (remaining < strlen(text) + 1) {
        builder->error = 1;
        return;
    }

    builder->offset += snprintf(
        builder->buffer + builder->offset,
        builder->capacity - builder->offset,
        "%s",
        text
    );
}

// ============================================================================
// Object and Array Construction
// ============================================================================

void json_builder_start_object(JSONBuilder* builder) {
    if (builder->error) return;

    json_builder_add_separator(builder);
    json_builder_add_raw(builder, "{");
    builder->element_count = 0;
}

void json_builder_end_object(JSONBuilder* builder) {
    if (builder->error) return;

    json_builder_add_raw(builder, "}");
    builder->element_count++;  // Increment parent's count
}

void json_builder_start_array(JSONBuilder* builder) {
    if (builder->error) return;

    json_builder_add_separator(builder);
    json_builder_add_raw(builder, "[");
    builder->element_count = 0;
}

void json_builder_end_array(JSONBuilder* builder) {
    if (builder->error) return;

    json_builder_add_raw(builder, "]");
    builder->element_count++;  // Increment parent's count
}

// ============================================================================
// Named Field Writers
// ============================================================================

void json_builder_add_string(JSONBuilder* builder, const char* key, const char* value) {
    if (builder->error || !key || !value) {
        return;
    }

    // Escape the value
    char escaped[MAX_ESCAPE_BUFFER];
    if (json_escape_string(value, escaped, sizeof(escaped)) != 0) {
        fprintf(stderr, "[JSON Builder] Failed to escape string: %s\n", value);
        builder->error = 1;
        return;
    }

    json_builder_add_separator(builder);

    size_t remaining = json_builder_remaining(builder);
    size_t needed = strlen(key) + strlen(escaped) + 10;  // "key":"value", + safety

    if (remaining < needed) {
        builder->error = 1;
        return;
    }

    builder->offset += snprintf(
        builder->buffer + builder->offset,
        builder->capacity - builder->offset,
        "\"%s\":\"%s\"",
        key,
        escaped
    );

    builder->element_count++;
}

void json_builder_add_int(JSONBuilder* builder, const char* key, int value) {
    if (builder->error || !key) {
        return;
    }

    json_builder_add_separator(builder);

    if (json_builder_remaining(builder) < 50) {
        builder->error = 1;
        return;
    }

    builder->offset += snprintf(
        builder->buffer + builder->offset,
        builder->capacity - builder->offset,
        "\"%s\":%d",
        key,
        value
    );

    builder->element_count++;
}

void json_builder_add_long(JSONBuilder* builder, const char* key, long value) {
    if (builder->error || !key) {
        return;
    }

    json_builder_add_separator(builder);

    if (json_builder_remaining(builder) < 50) {
        builder->error = 1;
        return;
    }

    builder->offset += snprintf(
        builder->buffer + builder->offset,
        builder->capacity - builder->offset,
        "\"%s\":%ld",
        key,
        value
    );

    builder->element_count++;
}

void json_builder_add_bool(JSONBuilder* builder, const char* key, int value) {
    if (builder->error || !key) {
        return;
    }

    json_builder_add_separator(builder);

    if (json_builder_remaining(builder) < 50) {
        builder->error = 1;
        return;
    }

    builder->offset += snprintf(
        builder->buffer + builder->offset,
        builder->capacity - builder->offset,
        "\"%s\":%s",
        key,
        value ? "true" : "false"
    );

    builder->element_count++;
}

void json_builder_add_null(JSONBuilder* builder, const char* key) {
    if (builder->error || !key) {
        return;
    }

    json_builder_add_separator(builder);

    if (json_builder_remaining(builder) < 50) {
        builder->error = 1;
        return;
    }

    builder->offset += snprintf(
        builder->buffer + builder->offset,
        builder->capacity - builder->offset,
        "\"%s\":null",
        key
    );

    builder->element_count++;
}

// ============================================================================
// Array Element Writers
// ============================================================================

void json_builder_add_string_element(JSONBuilder* builder, const char* value) {
    if (builder->error || !value) {
        return;
    }

    // Escape the value
    char escaped[MAX_ESCAPE_BUFFER];
    if (json_escape_string(value, escaped, sizeof(escaped)) != 0) {
        fprintf(stderr, "[JSON Builder] Failed to escape string element: %s\n", value);
        builder->error = 1;
        return;
    }

    json_builder_add_separator(builder);

    size_t remaining = json_builder_remaining(builder);
    size_t needed = strlen(escaped) + 5;  // "value", + safety

    if (remaining < needed) {
        builder->error = 1;
        return;
    }

    builder->offset += snprintf(
        builder->buffer + builder->offset,
        builder->capacity - builder->offset,
        "\"%s\"",
        escaped
    );

    builder->element_count++;
}

void json_builder_add_int_element(JSONBuilder* builder, int value) {
    if (builder->error) {
        return;
    }

    json_builder_add_separator(builder);

    if (json_builder_remaining(builder) < 20) {
        builder->error = 1;
        return;
    }

    builder->offset += snprintf(
        builder->buffer + builder->offset,
        builder->capacity - builder->offset,
        "%d",
        value
    );

    builder->element_count++;
}

// ============================================================================
// Composite Builders
// ============================================================================

void json_builder_start_array_field(JSONBuilder* builder, const char* key) {
    if (builder->error || !key) {
        return;
    }

    json_builder_add_separator(builder);

    if (json_builder_remaining(builder) < strlen(key) + 10) {
        builder->error = 1;
        return;
    }

    builder->offset += snprintf(
        builder->buffer + builder->offset,
        builder->capacity - builder->offset,
        "\"%s\":[",
        key
    );

    builder->element_count++;
    builder->element_count = 0;  // Reset for array elements
}

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
) {
    if (builder->error) {
        return;
    }

    // Check if we have enough space (rough estimate)
    if (json_builder_remaining(builder) < MAX_JSON_SMALL_BUFFER) {
        builder->error = 1;
        return;
    }

    json_builder_start_object(builder);

    json_builder_add_int(builder, "video_id", video_id);
    json_builder_add_string(builder, "title", title ? title : "Unknown");
    json_builder_add_string(builder, "filename", filename ? filename : "");
    json_builder_add_string(builder, "thumbnail", (thumbnail && strlen(thumbnail) > 0) ? thumbnail : "");
    json_builder_add_int(builder, "duration", duration);
    json_builder_add_long(builder, "file_size", file_size);
    json_builder_add_bool(builder, "watched", watched);
    json_builder_add_int(builder, "last_position", last_position);

    json_builder_end_object(builder);
}
