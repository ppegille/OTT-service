/*
 * OTT Streaming Server - Database Module
 * Enhancement Phase 2: SQLite integration
 *
 * Manages users, videos, and watch history
 */

#include "../include/database.h"
#include "../include/crypto.h"
#include "../include/json.h"
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

    // Start building JSON
    size_t offset = 0;
    offset += snprintf(json_output + offset, max_len - offset, "{\"videos\":[");

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && offset < max_len - 512) {
        if (count > 0) {
            offset += snprintf(json_output + offset, max_len - offset, ",");
        }

        int video_id = sqlite3_column_int(stmt, 0);
        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        const char* filename = (const char*)sqlite3_column_text(stmt, 2);
        const char* thumbnail = sqlite3_column_text(stmt, 3) ?
                                (const char*)sqlite3_column_text(stmt, 3) : "";
        int duration = sqlite3_column_int(stmt, 4);
        long file_size = sqlite3_column_int64(stmt, 5);
        int last_position = sqlite3_column_int(stmt, 6);

        // Escape strings for JSON using json_escape_string()
        char escaped_title[512];
        char escaped_filename[512];
        char escaped_thumbnail[512];

        if (json_escape_string(title, escaped_title, sizeof(escaped_title)) != 0) {
            fprintf(stderr, "[ERROR] Failed to escape title: %s\n", title);
            strncpy(escaped_title, "Error", sizeof(escaped_title) - 1);
        }

        if (json_escape_string(filename, escaped_filename, sizeof(escaped_filename)) != 0) {
            fprintf(stderr, "[ERROR] Failed to escape filename: %s\n", filename);
            strncpy(escaped_filename, "Error", sizeof(escaped_filename) - 1);
        }

        if (json_escape_string(thumbnail, escaped_thumbnail, sizeof(escaped_thumbnail)) != 0) {
            fprintf(stderr, "[ERROR] Failed to escape thumbnail: %s\n", thumbnail);
            strncpy(escaped_thumbnail, "Error", sizeof(escaped_thumbnail) - 1);
        }

        // Add video JSON object
        offset += snprintf(json_output + offset, max_len - offset,
            "{\"video_id\":%d,\"title\":\"%s\",\"filename\":\"%s\","
            "\"thumbnail\":\"%s\",\"duration\":%d,\"file_size\":%ld,"
            "\"watched\":%s,\"last_position\":%d}",
            video_id,
            escaped_title,
            escaped_filename,
            escaped_thumbnail,
            duration,
            file_size,
            (last_position > 0) ? "true" : "false",
            last_position
        );

        count++;
    }

    // Close JSON array and object
    offset += snprintf(json_output + offset, max_len - offset, "]}");

    sqlite3_finalize(stmt);

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
