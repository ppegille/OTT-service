#ifndef DATABASE_H
#define DATABASE_H

#include <stddef.h>
#include <sqlite3.h>
#include "config.h"

// Database configuration (DB_PATH is in config.h)
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
    char hls_path[256];
    char hls_status[20];
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
int update_video_metadata(const char* filename, int duration, const char* thumbnail_path);

// Watch history functions (Phase 3)
int get_watch_position(int user_id, int video_id);
int update_watch_position(int user_id, int video_id, int position);
int get_video_filename(int video_id, char* filename, size_t max_len);
int get_watch_history_json(int user_id, int video_id, char* json_output, size_t max_len);
int get_recommended_videos(int user_id, char* json_output, size_t max_len);

// Search functions
int search_videos(const char* query, char* json_output, size_t max_len);

// Genre functions (Enhancement Phase 1)
int get_genres_json(char* json_output, size_t max_len);
int get_videos_by_genre(int genre_id, char* json_output, size_t max_len);
int assign_genre_to_video(int video_id, int genre_id);

// Watchlist functions (Enhancement Phase 1)
int get_watchlist(int user_id, char* json_output, size_t max_len);
int add_to_watchlist(int user_id, int video_id);
int remove_from_watchlist(int user_id, int video_id);
int is_in_watchlist(int user_id, int video_id);

// HLS functions (Enhancement Phase 2)
int update_hls_path(int video_id, const char* hls_path, const char* status);
int get_hls_path(int video_id, char* hls_path, size_t max_len);

// Utility functions
int execute_sql_file(sqlite3* db, const char* filepath);

#endif // DATABASE_H
