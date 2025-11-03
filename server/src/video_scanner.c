/*
 * OTT Streaming Server - Video Scanner
 *
 * Automatically scan videos directory and register to database
 * Enhancement Phase 3
 * Author: Generated for Network Programming Final Project
 * Date: 2025-11-03
 */

#include "../include/server.h"
#include "../include/database.h"
#include <dirent.h>
#include <sys/stat.h>

/**
 * Get file size in bytes
 */
long get_video_file_size(const char* filepath) {
    struct stat st;
    if (stat(filepath, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

/**
 * Check if string ends with suffix
 */
int ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) {
        return 0;
    }

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) {
        return 0;
    }

    return strcasecmp(str + str_len - suffix_len, suffix) == 0;
}

/**
 * Generate title from filename
 * Example: "test_video.mp4" -> "Test Video"
 */
void generate_title_from_filename(const char* filename, char* title, size_t max_len) {
    // Copy filename without extension
    strncpy(title, filename, max_len - 1);
    title[max_len - 1] = '\0';

    // Remove extension
    char* ext = strrchr(title, '.');
    if (ext) {
        *ext = '\0';
    }

    // Replace underscores with spaces
    for (char* p = title; *p; p++) {
        if (*p == '_' || *p == '-') {
            *p = ' ';
        }
    }

    // Capitalize first letter
    if (title[0] >= 'a' && title[0] <= 'z') {
        title[0] = title[0] - 'a' + 'A';
    }
}

/**
 * Scan videos directory and register new videos to database
 * Returns: number of videos registered
 */
int scan_and_register_videos(const char* video_dir) {
    DIR* dir;
    struct dirent* entry;
    int registered_count = 0;
    int total_count = 0;

    printf("📹 Scanning video directory: %s\n", video_dir);

    dir = opendir(video_dir);
    if (!dir) {
        fprintf(stderr, "⚠️  Cannot open video directory: %s\n", video_dir);
        return 0;
    }

    // Scan all files in directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Check if file is a video (by extension)
        if (ends_with(entry->d_name, ".mp4") ||
            ends_with(entry->d_name, ".mkv") ||
            ends_with(entry->d_name, ".avi") ||
            ends_with(entry->d_name, ".mov")) {

            total_count++;

            // Build full path
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", video_dir, entry->d_name);

            // Get file size
            long file_size = get_video_file_size(filepath);

            // Generate title from filename
            char title[256];
            generate_title_from_filename(entry->d_name, title, sizeof(title));

            // Register to database (INSERT OR IGNORE - won't duplicate)
            if (register_video(entry->d_name, title, file_size) == 0) {
                registered_count++;
                printf("  ✓ Registered: %s (%ld bytes)\n", entry->d_name, file_size);
            }
        }
    }

    closedir(dir);

    printf("📊 Video scan complete: %d files found, %d newly registered\n",
           total_count, registered_count);

    return registered_count;
}

/**
 * Update video metadata (duration, thumbnail) - Placeholder for FFmpeg integration
 */
void update_video_metadata(const char* video_dir) {
    // TODO: Phase 3 - FFmpeg integration
    // - Extract video duration
    // - Generate thumbnail
    // - Update database

    printf("ℹ️  Video metadata update (FFmpeg) - Not implemented yet\n");
}
