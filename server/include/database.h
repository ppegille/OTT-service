#ifndef DATABASE_H
#define DATABASE_H

#include <stddef.h>
#include <sqlite3.h>

// Database configuration
#define DB_PATH "database/ott.db"
#define SCHEMA_PATH "database/schema.sql"
#define SEED_PATH "database/seed.sql"

// User structure for database operations
typedef struct {
    int user_id;
    char username[64];
    char password_hash[65];  // SHA-256 hex string (64 chars + null)
} User;

// Video structure for database operations
typedef struct {
    int video_id;
    char title[256];
    char filename[256];
    char thumbnail_path[256];
    int duration;
    long file_size;
} Video;

// Database initialization and cleanup
int init_database(const char* db_path);
void close_database(void);

// User authentication functions
int authenticate_user(const char* username, const char* password, int* user_id);
int create_user(const char* username, const char* password);
int update_last_login(int user_id);

// Video management functions
int get_videos_json(char* json_output, size_t max_len);
int get_videos_with_history(int user_id, char* json_output, size_t max_len);
int get_video_by_id(int video_id, Video* video);
int get_video_by_filename(const char* filename, Video* video);
int register_video(const char* filename, const char* title, long file_size);

// Watch history functions (Phase 3)
int get_watch_position(int user_id, int video_id);
int update_watch_position(int user_id, int video_id, int position);
int get_watch_history_json(int user_id, int video_id, char* json_output, size_t max_len);

// Utility functions
int execute_sql_file(sqlite3* db, const char* filepath);

#endif // DATABASE_H
