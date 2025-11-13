/*
 * OTT Streaming Server - Database Module
 * Enhancement Phase 2: SQLite integration
 *
 * Manages users, videos, and watch history
 */

#include "../include/database.h"
#include "../include/config.h"
#include "../include/crypto.h"
#include "../include/json.h"
#include "../include/json_builder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global database connection
static sqlite3* db = NULL;

/**
 * Execute SQL commands from a file
 */
int execute_sql_file(sqlite3* database, const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Failed to open SQL file: %s\n", filepath);
        return -1;
    }

    // Read entire file
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* sql = malloc(fsize + 1);
    fread(sql, 1, fsize, f);
    fclose(f);
    sql[fsize] = '\0';

    // Execute SQL
    char* err_msg = NULL;
    int rc = sqlite3_exec(database, sql, NULL, NULL, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error in %s: %s\n", filepath, err_msg);
        sqlite3_free(err_msg);
        free(sql);
        return -1;
    }

    free(sql);
    return 0;
}

/**
 * Initialize database connection and create tables
 */
int init_database(const char* db_path) {
    int rc = sqlite3_open(db_path, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    printf("✓ Database opened: %s\n", db_path);

    // Create tables from schema
    if (execute_sql_file(db, SCHEMA_PATH) != 0) {
        fprintf(stderr, "Failed to create schema\n");
        return -1;
    }

    printf("✓ Database schema created\n");

    // Insert initial data
    if (execute_sql_file(db, SEED_PATH) != 0) {
        fprintf(stderr, "Failed to seed database\n");
        return -1;
    }

    printf("✓ Database seeded with test data\n");

    return 0;
}

/**
 * Close database connection
 */
void close_database(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
        printf("✓ Database connection closed\n");
    }
}

/**
 * Authenticate user with username and password
 */
int authenticate_user(const char* username, const char* password, int* user_id) {
    if (!db) return 0;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT user_id, password_hash FROM users WHERE username = ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // User found
        int uid = sqlite3_column_int(stmt, 0);
        const char* stored_hash = (const char*)sqlite3_column_text(stmt, 1);

        // Verify password
        if (verify_password(password, stored_hash)) {
            *user_id = uid;
            sqlite3_finalize(stmt);
            printf("  [Auth] User '%s' authenticated successfully (ID: %d)\n", username, uid);
            return 1;
        } else {
            printf("  [Auth] Invalid password for user '%s'\n", username);
        }
    } else {
        printf("  [Auth] User '%s' not found\n", username);
    }

    sqlite3_finalize(stmt);
    return 0;
}

/**
 * Create new user
 */
int create_user(const char* username, const char* password) {
    if (!db) return -1;

    // Hash password
    char password_hash[SHA256_HEX_LENGTH];
    sha256_hash(password, password_hash);

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to create user: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    printf("  [DB] Created user: %s\n", username);
    return 0;
}

/**
 * Update user's last login time
 */
int update_last_login(int user_id) {
    if (!db) return -1;

    sqlite3_stmt* stmt;
    const char* sql = "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE user_id = ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

/**
 * Get all videos as JSON array
 */
int get_videos_json(char* json_output, size_t max_len) {
    if (!db) return -1;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT video_id, title, filename, thumbnail_path, duration, file_size FROM videos ORDER BY created_at DESC";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }

    // Start JSON array
    strcpy(json_output, "{\"videos\":[");
    size_t offset = strlen(json_output);

    int first = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) {
            strcat(json_output + offset, ",");
            offset++;
        }
        first = 0;

        int video_id = sqlite3_column_int(stmt, 0);
        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        const char* filename = (const char*)sqlite3_column_text(stmt, 2);
        const char* thumbnail = sqlite3_column_text(stmt, 3) ?
            (const char*)sqlite3_column_text(stmt, 3) : "";
        int duration = sqlite3_column_int(stmt, 4);
        long file_size = sqlite3_column_int64(stmt, 5);

        char video_json[512];
        snprintf(video_json, sizeof(video_json),
            "{\"video_id\":%d,\"title\":\"%s\",\"filename\":\"%s\",\"thumbnail\":\"%s\",\"duration\":%d,\"file_size\":%ld}",
            video_id, title, filename, thumbnail, duration, file_size);

        if (offset + strlen(video_json) >= max_len - 10) {
            break;  // Prevent buffer overflow
        }

        strcat(json_output + offset, video_json);
        offset += strlen(video_json);
    }

    strcat(json_output + offset, "]}");
    sqlite3_finalize(stmt);

    return 0;
}

/**
 * Get video by ID
 */
int get_video_by_id(int video_id, Video* video) {
    if (!db || !video) return -1;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT title, filename, thumbnail_path, duration, file_size FROM videos WHERE video_id = ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, video_id);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        video->video_id = video_id;
        strncpy(video->title, (const char*)sqlite3_column_text(stmt, 0), sizeof(video->title) - 1);
        strncpy(video->filename, (const char*)sqlite3_column_text(stmt, 1), sizeof(video->filename) - 1);
        if (sqlite3_column_text(stmt, 2)) {
            strncpy(video->thumbnail_path, (const char*)sqlite3_column_text(stmt, 2), sizeof(video->thumbnail_path) - 1);
        }
        video->duration = sqlite3_column_int(stmt, 3);
        video->file_size = sqlite3_column_int64(stmt, 4);

        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}

/**
 * Get video by filename
 */
int get_video_by_filename(const char* filename, Video* video) {
    if (!db || !video) return -1;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT video_id, title, thumbnail_path, duration, file_size FROM videos WHERE filename = ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, filename, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        video->video_id = sqlite3_column_int(stmt, 0);
        strncpy(video->title, (const char*)sqlite3_column_text(stmt, 1), sizeof(video->title) - 1);
        strncpy(video->filename, filename, sizeof(video->filename) - 1);
        if (sqlite3_column_text(stmt, 2)) {
            strncpy(video->thumbnail_path, (const char*)sqlite3_column_text(stmt, 2), sizeof(video->thumbnail_path) - 1);
        }
        video->duration = sqlite3_column_int(stmt, 3);
        video->file_size = sqlite3_column_int64(stmt, 4);

        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}

/**
 * Register new video
 */
int register_video(const char* filename, const char* title, long file_size) {
    if (!db) return -1;

    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR IGNORE INTO videos (filename, title, file_size) VALUES (?, ?, ?)";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, title, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, file_size);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        printf("  [DB] Registered video: %s\n", filename);
        return 0;
    }

    return -1;
}

/**
 * Update video metadata (duration and thumbnail_path)
 */
int update_video_metadata(const char* filename, int duration, const char* thumbnail_path) {
    if (!db) return -1;

    sqlite3_stmt* stmt;
    const char* sql = "UPDATE videos SET duration = ?, thumbnail_path = ? WHERE filename = ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "  [DB] Failed to prepare UPDATE statement\n");
        return -1;
    }

    sqlite3_bind_int(stmt, 1, duration);
    sqlite3_bind_text(stmt, 2, thumbnail_path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, filename, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        printf("  [DB] Updated metadata for %s: duration=%ds, thumbnail=%s\n",
               filename, duration, thumbnail_path);
        return 0;
    }

    return -1;
}

/**
 * Get watch position for user/video (Phase 3)
 */
int get_watch_position(int user_id, int video_id) {
    if (!db) return 0;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT last_position FROM watch_history WHERE user_id = ? AND video_id = ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, video_id);

    int position = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        position = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return position;
}

/**
 * Update watch position for user/video (Phase 3)
 */
int update_watch_position(int user_id, int video_id, int position) {
    if (!db) return -1;

    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO watch_history (user_id, video_id, last_position, last_watched) "
                      "VALUES (?, ?, ?, CURRENT_TIMESTAMP)";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, video_id);
    sqlite3_bind_int(stmt, 3, position);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

/**
 * Get video filename by video_id
 */
int get_video_filename(int video_id, char* filename, size_t max_len) {
    if (!db || !filename || max_len == 0) {
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql = "SELECT filename FROM videos WHERE video_id = ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, video_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* fname = (const char*)sqlite3_column_text(stmt, 0);
        snprintf(filename, max_len, "%s", fname ? fname : "");
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}

/**
 * Get all videos with watch history for specific user (Phase 3)
 * Returns JSON array of videos with watch progress
 */
int get_videos_with_history(int user_id, char* json_output, size_t max_len) {
    if (!db || !json_output || max_len == 0) {
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT v.video_id, v.title, v.filename, v.thumbnail_path, v.duration, v.file_size, "
        "       COALESCE(wh.last_position, 0) as last_position "
        "FROM videos v "
        "LEFT JOIN watch_history wh ON v.video_id = wh.video_id AND wh.user_id = ? "
        "ORDER BY v.video_id";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        snprintf(json_output, max_len, "{\"status\":\"error\",\"message\":\"Database error\"}");
        return -1;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    // Initialize JSON builder
    JSONBuilder builder;
    json_builder_init(&builder, json_output, max_len);

    // Build JSON response: {"videos":[...]}
    json_builder_start_object(&builder);
    json_builder_start_array_field(&builder, "videos");

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Check if we're running out of buffer space
        if (json_builder_remaining(&builder) < MAX_JSON_SMALL_BUFFER) {
            fprintf(stderr, "[WARNING] Buffer nearly full, stopping at %d videos\n", count);
            break;
        }

        int video_id = sqlite3_column_int(stmt, 0);
        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        const char* filename = (const char*)sqlite3_column_text(stmt, 2);
        const char* thumbnail = sqlite3_column_text(stmt, 3) ?
                                (const char*)sqlite3_column_text(stmt, 3) : "";
        int duration = sqlite3_column_int(stmt, 4);
        long file_size = sqlite3_column_int64(stmt, 5);
        int last_position = sqlite3_column_int(stmt, 6);

        // Add video object using builder (handles escaping automatically)
        json_builder_add_video_object(
            &builder,
            video_id,
            title,
            filename,
            thumbnail,
            duration,
            file_size,
            last_position > 0,
            last_position
        );

        count++;
    }

    // Close JSON array and object
    json_builder_end_array(&builder);
    json_builder_end_object(&builder);

    sqlite3_finalize(stmt);

    // Check for builder errors
    if (json_builder_has_error(&builder)) {
        fprintf(stderr, "[ERROR] JSON builder encountered an error\n");
        snprintf(json_output, max_len, "{\"status\":\"error\",\"message\":\"JSON generation failed\"}");
        return -1;
    }

    printf("  [API] Returned %d videos with history for user %d\n", count, user_id);
    return 0;
}

/**
 * Get watch history JSON for specific video (Phase 3)
 */
int get_watch_history_json(int user_id, int video_id, char* json_output, size_t max_len) {
    if (!db || !json_output || max_len == 0) {
        return -1;
    }

    int position = get_watch_position(user_id, video_id);

    snprintf(json_output, max_len,
        "{\"video_id\":%d,\"last_position\":%d,\"watched\":%s}",
        video_id,
        position,
        (position > 0) ? "true" : "false"
    );

    return 0;
}

/**
 * Get recommended videos for user based on watch history (Continue Watching)
 * Returns JSON array of videos that user has partially watched
 */
int get_recommended_videos(int user_id, char* json_output, size_t max_len) {
    if (!db || !json_output || max_len == 0) {
        return -1;
    }

    sqlite3_stmt* stmt;
    // Get videos user has started watching but not completed (position < 90% of duration)
    // Show most recently watched first
    // Using percentage-based threshold (90%) instead of fixed 30 seconds to support short videos
    const char* sql =
        "SELECT v.video_id, v.title, v.filename, v.thumbnail_path, v.duration, v.file_size, "
        "       wh.last_position, wh.last_watched "
        "FROM videos v "
        "INNER JOIN watch_history wh ON v.video_id = wh.video_id "
        "WHERE wh.user_id = ? AND wh.last_position > 0 AND wh.last_position < (v.duration * 0.9) "
        "ORDER BY wh.last_watched DESC "
        "LIMIT 5";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        snprintf(json_output, max_len, "{\"status\":\"error\",\"message\":\"Database error\"}");
        return -1;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    // Initialize JSON builder
    JSONBuilder builder;
    json_builder_init(&builder, json_output, max_len);

    // Build JSON response: {"recommendations":[...]}
    json_builder_start_object(&builder);
    json_builder_start_array_field(&builder, "recommendations");

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Check buffer space
        if (json_builder_remaining(&builder) < MAX_JSON_SMALL_BUFFER) {
            fprintf(stderr, "[WARNING] Buffer nearly full, stopping at %d recommendations\n", count);
            break;
        }

        int video_id = sqlite3_column_int(stmt, 0);
        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        const char* filename = (const char*)sqlite3_column_text(stmt, 2);
        const char* thumbnail = sqlite3_column_text(stmt, 3) ?
                                (const char*)sqlite3_column_text(stmt, 3) : "";
        int duration = sqlite3_column_int(stmt, 4);
        long file_size = sqlite3_column_int64(stmt, 5);
        int last_position = sqlite3_column_int(stmt, 6);
        const char* last_watched = (const char*)sqlite3_column_text(stmt, 7);

        // Calculate progress percentage
        int progress_percent = (duration > 0) ? (last_position * 100 / duration) : 0;

        // Build recommendation object with extra fields
        json_builder_start_object(&builder);
        json_builder_add_int(&builder, "video_id", video_id);
        json_builder_add_string(&builder, "title", title ? title : "Unknown");
        json_builder_add_string(&builder, "filename", filename ? filename : "");
        json_builder_add_string(&builder, "thumbnail", (thumbnail && strlen(thumbnail) > 0) ? thumbnail : "");
        json_builder_add_int(&builder, "duration", duration);
        json_builder_add_long(&builder, "file_size", file_size);
        json_builder_add_int(&builder, "last_position", last_position);
        json_builder_add_int(&builder, "progress_percent", progress_percent);
        json_builder_add_string(&builder, "last_watched", last_watched ? last_watched : "");
        json_builder_end_object(&builder);

        count++;
    }

    // Close JSON array and object
    json_builder_end_array(&builder);
    json_builder_end_object(&builder);

    sqlite3_finalize(stmt);

    // Check for errors
    if (json_builder_has_error(&builder)) {
        fprintf(stderr, "[ERROR] JSON builder encountered an error\n");
        snprintf(json_output, max_len, "{\"status\":\"error\",\"message\":\"JSON generation failed\"}");
        return -1;
    }

    printf("  [API] Returned %d recommended videos for user %d\n", count, user_id);
    return 0;
}

// Search videos by title
int search_videos(const char* query, char* json_output, size_t max_len) {
    if (!db || !query || !json_output || max_len == 0) {
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT video_id, title, filename, thumbnail_path, duration, file_size "
        "FROM videos "
        "WHERE title LIKE ? "
        "ORDER BY title ASC";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare search statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Create search pattern: %query%
    char search_pattern[256];
    snprintf(search_pattern, sizeof(search_pattern), "%%%s%%", query);
    sqlite3_bind_text(stmt, 1, search_pattern, -1, SQLITE_TRANSIENT);

    // Initialize JSON builder
    JSONBuilder builder;
    json_builder_init(&builder, json_output, max_len);

    // Build JSON response: {"results":[...], "count": N}
    json_builder_start_object(&builder);
    json_builder_start_array_field(&builder, "results");

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Check buffer space
        if (json_builder_remaining(&builder) < MAX_JSON_SMALL_BUFFER) {
            fprintf(stderr, "[WARNING] Buffer nearly full, stopping search at %d results\n", count);
            break;
        }

        int video_id = sqlite3_column_int(stmt, 0);
        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        const char* filename = (const char*)sqlite3_column_text(stmt, 2);
        const char* thumbnail = (const char*)sqlite3_column_text(stmt, 3);
        int duration = sqlite3_column_int(stmt, 4);
        long file_size = sqlite3_column_int64(stmt, 5);

        // Add video object (with proper escaping)
        json_builder_add_video_object(
            &builder,
            video_id,
            title,
            filename,
            thumbnail,
            duration,
            file_size,
            0,  // watched flag not applicable for search
            0   // last_position not applicable for search
        );

        count++;
    }

    // Close array and add count
    json_builder_end_array(&builder);
    json_builder_add_int(&builder, "count", count);
    json_builder_end_object(&builder);

    sqlite3_finalize(stmt);

    // Check for errors
    if (json_builder_has_error(&builder)) {
        fprintf(stderr, "[ERROR] JSON builder encountered an error\n");
        snprintf(json_output, max_len, "{\"status\":\"error\",\"message\":\"JSON generation failed\"}");
        return -1;
    }

    printf("  [API] Search for '%s' returned %d results\n", query, count);
    return 0;
}

// Get all genres as JSON
int get_genres_json(char* json_output, size_t max_len) {
    if (!db || !json_output || max_len == 0) {
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql = "SELECT genre_id, name, description FROM genres ORDER BY name ASC";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare genres query: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    int offset = 0;
    offset += snprintf(json_output, max_len, "{\"genres\":[");

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (count > 0) {
            offset += snprintf(json_output + offset, max_len - offset, ",");
        }

        int genre_id = sqlite3_column_int(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        const char* description = (const char*)sqlite3_column_text(stmt, 2);

        offset += snprintf(json_output + offset, max_len - offset,
            "{\"genre_id\":%d,\"name\":\"%s\",\"description\":\"%s\"}",
            genre_id,
            name ? name : "",
            description ? description : ""
        );

        count++;
    }

    offset += snprintf(json_output + offset, max_len - offset, "]}");
    sqlite3_finalize(stmt);

    printf("  [API] Returned %d genres\n", count);
    return 0;
}

// Get videos by genre ID
int get_videos_by_genre(int genre_id, char* json_output, size_t max_len) {
    if (!db || !json_output || max_len == 0) {
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT v.video_id, v.title, v.filename, v.thumbnail_path, v.duration, v.file_size "
        "FROM videos v "
        "INNER JOIN video_genres vg ON v.video_id = vg.video_id "
        "WHERE vg.genre_id = ? "
        "ORDER BY v.title ASC";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare videos by genre query: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, genre_id);

    int offset = 0;
    offset += snprintf(json_output, max_len, "{\"videos\":[");

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (count > 0) {
            offset += snprintf(json_output + offset, max_len - offset, ",");
        }

        int video_id = sqlite3_column_int(stmt, 0);
        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        const char* filename = (const char*)sqlite3_column_text(stmt, 2);
        const char* thumbnail = (const char*)sqlite3_column_text(stmt, 3);
        int duration = sqlite3_column_int(stmt, 4);
        long long file_size = sqlite3_column_int64(stmt, 5);

        offset += snprintf(json_output + offset, max_len - offset,
            "{\"video_id\":%d,"
            "\"title\":\"%s\","
            "\"filename\":\"%s\","
            "\"thumbnail\":\"%s\","
            "\"duration\":%d,"
            "\"file_size\":%lld}",
            video_id,
            title ? title : "",
            filename ? filename : "",
            thumbnail ? thumbnail : "",
            duration,
            file_size
        );

        count++;
    }

    offset += snprintf(json_output + offset, max_len - offset, "],\"count\":%d}", count);
    sqlite3_finalize(stmt);

    printf("  [API] Genre %d returned %d videos\n", genre_id, count);
    return 0;
}

// Assign genre to video
int assign_genre_to_video(int video_id, int genre_id) {
    if (!db) {
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR IGNORE INTO video_genres (video_id, genre_id) VALUES (?, ?)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare assign genre statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, video_id);
    sqlite3_bind_int(stmt, 2, genre_id);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        fprintf(stderr, "Failed to assign genre: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    printf("  [DB] Assigned genre %d to video %d\n", genre_id, video_id);
    return 0;
}

// Get user's watchlist
int get_watchlist(int user_id, char* json_output, size_t max_len) {
    if (!db || !json_output || max_len == 0) {
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT v.video_id, v.title, v.filename, v.thumbnail_path, v.duration, v.file_size, w.added_at "
        "FROM videos v "
        "INNER JOIN watchlist w ON v.video_id = w.video_id "
        "WHERE w.user_id = ? "
        "ORDER BY w.added_at DESC";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare watchlist query: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    int offset = 0;
    offset += snprintf(json_output, max_len, "{\"watchlist\":[");

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (count > 0) {
            offset += snprintf(json_output + offset, max_len - offset, ",");
        }

        int video_id = sqlite3_column_int(stmt, 0);
        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        const char* filename = (const char*)sqlite3_column_text(stmt, 2);
        const char* thumbnail = (const char*)sqlite3_column_text(stmt, 3);
        int duration = sqlite3_column_int(stmt, 4);
        long long file_size = sqlite3_column_int64(stmt, 5);
        const char* added_at = (const char*)sqlite3_column_text(stmt, 6);

        offset += snprintf(json_output + offset, max_len - offset,
            "{\"video_id\":%d,"
            "\"title\":\"%s\","
            "\"filename\":\"%s\","
            "\"thumbnail\":\"%s\","
            "\"duration\":%d,"
            "\"file_size\":%lld,"
            "\"added_at\":\"%s\"}",
            video_id,
            title ? title : "",
            filename ? filename : "",
            thumbnail ? thumbnail : "",
            duration,
            file_size,
            added_at ? added_at : ""
        );

        count++;
    }

    offset += snprintf(json_output + offset, max_len - offset, "],\"count\":%d}", count);
    sqlite3_finalize(stmt);

    printf("  [API] User %d has %d videos in watchlist\n", user_id, count);
    return 0;
}

// Add video to watchlist
int add_to_watchlist(int user_id, int video_id) {
    if (!db) {
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR IGNORE INTO watchlist (user_id, video_id) VALUES (?, ?)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare add to watchlist statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, video_id);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        fprintf(stderr, "Failed to add to watchlist: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    printf("  [DB] User %d added video %d to watchlist\n", user_id, video_id);
    return 0;
}

// Remove video from watchlist
int remove_from_watchlist(int user_id, int video_id) {
    if (!db) {
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql = "DELETE FROM watchlist WHERE user_id = ? AND video_id = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare remove from watchlist statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, video_id);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        fprintf(stderr, "Failed to remove from watchlist: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    printf("  [DB] User %d removed video %d from watchlist\n", user_id, video_id);
    return 0;
}

// Check if video is in watchlist
int is_in_watchlist(int user_id, int video_id) {
    if (!db) {
        return 0;
    }

    sqlite3_stmt* stmt;
    const char* sql = "SELECT COUNT(*) FROM watchlist WHERE user_id = ? AND video_id = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare watchlist check statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, video_id);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count > 0 ? 1 : 0;
}

// ============================================================================
// HLS Functions (Enhancement Phase 2)
// ============================================================================

/**
 * Update HLS path and status for a video
 *
 * @param video_id Video identifier
 * @param hls_path Path to master.m3u8 file (relative to server root)
 * @param status HLS transcoding status: "pending", "processing", "ready", "failed"
 * @return 0 on success, -1 on error
 */
int update_hls_path(int video_id, const char* hls_path, const char* status) {
    if (!db) {
        fprintf(stderr, "Database not initialized\n");
        return -1;
    }

    if (!hls_path || !status) {
        fprintf(stderr, "Invalid parameters for update_hls_path\n");
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql = "UPDATE videos SET hls_path = ?, hls_status = ? WHERE video_id = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare HLS update statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, hls_path, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, status, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, video_id);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        fprintf(stderr, "Failed to update HLS path: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    printf("  [DB] Updated HLS path for video %d: %s (status: %s)\n", video_id, hls_path, status);
    return 0;
}

/**
 * Get HLS playlist path for a video
 *
 * @param video_id Video identifier
 * @param hls_path Buffer to store HLS path
 * @param max_len Maximum buffer length
 * @return 0 on success, -1 on error or not found
 */
int get_hls_path(int video_id, char* hls_path, size_t max_len) {
    if (!db) {
        fprintf(stderr, "Database not initialized\n");
        return -1;
    }

    if (!hls_path || max_len == 0) {
        fprintf(stderr, "Invalid parameters for get_hls_path\n");
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql = "SELECT hls_path, hls_status FROM videos WHERE video_id = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare HLS path query: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, video_id);

    int result = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* path = (const char*)sqlite3_column_text(stmt, 0);
        const char* status = (const char*)sqlite3_column_text(stmt, 1);

        if (path && status && strcmp(status, "ready") == 0) {
            strncpy(hls_path, path, max_len - 1);
            hls_path[max_len - 1] = '\0';
            result = 0;
        } else {
            fprintf(stderr, "  [DB] HLS not ready for video %d (status: %s)\n",
                    video_id, status ? status : "unknown");
        }
    }

    sqlite3_finalize(stmt);
    return result;
}
