/*
 * OTT Streaming Server - Video Scanner
 *
 * Automatically scan videos directory and register to database
 * Enhancement Phase 3 - Updated with FFmpeg integration
 * Author: Generated for Network Programming Final Project
 * Date: 2025-11-10
 */

#include "../include/server.h"
#include "../include/database.h"
#include "../include/ffmpeg_utils.h"
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
 * Update video metadata (duration, thumbnail) for all videos in directory using FFmpeg
 * Batch operation that scans directory and updates all video files
 */
void update_all_video_metadata(const char* video_dir) {
    DIR* dir;
    struct dirent* entry;
    int updated_count = 0;

    printf("🎬 Updating video metadata with FFmpeg...\n");

    dir = opendir(video_dir);
    if (!dir) {
        fprintf(stderr, "⚠️  Cannot open video directory: %s\n", video_dir);
        return;
    }

    // Create thumbnails directory if it doesn't exist
    #ifdef _WIN32
        system("if not exist thumbnails mkdir thumbnails");
    #else
        system("mkdir -p thumbnails");
    #endif

    // Process each video file
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Check if file is a video
        if (ends_with(entry->d_name, ".mp4") ||
            ends_with(entry->d_name, ".mkv") ||
            ends_with(entry->d_name, ".avi") ||
            ends_with(entry->d_name, ".mov")) {

            // Build full video path
            char video_path[512];
            snprintf(video_path, sizeof(video_path), "%s/%s", video_dir, entry->d_name);

            // Extract duration using FFmpeg
            printf("  📊 Extracting duration for %s...\n", entry->d_name);
            int duration = get_video_duration(video_path);
            if (duration < 0) {
                fprintf(stderr, "  ⚠️  Failed to extract duration for %s\n", entry->d_name);
                duration = 0;
            } else {
                printf("  ✓ Duration: %d seconds (%d:%02d)\n",
                       duration, duration/60, duration%60);
            }

            // Generate thumbnail filename
            char thumbnail_filename[256];
            char* ext = strrchr(entry->d_name, '.');
            if (ext) {
                size_t name_len = ext - entry->d_name;
                strncpy(thumbnail_filename, entry->d_name, name_len);
                thumbnail_filename[name_len] = '\0';
                strcat(thumbnail_filename, ".jpg");
            } else {
                snprintf(thumbnail_filename, sizeof(thumbnail_filename), "%s.jpg", entry->d_name);
            }

            // Build thumbnail path
            char thumbnail_path[512];
            snprintf(thumbnail_path, sizeof(thumbnail_path), "thumbnails/%s", thumbnail_filename);

            // Generate thumbnail using FFmpeg
            printf("  🖼️  Generating thumbnail for %s...\n", entry->d_name);
            if (generate_thumbnail_default(video_path, thumbnail_path) == 0) {
                printf("  ✓ Thumbnail created: %s\n", thumbnail_path);
            } else {
                fprintf(stderr, "  ⚠️  Failed to generate thumbnail for %s\n", entry->d_name);
                // Use placeholder path if thumbnail generation fails
                strcpy(thumbnail_path, "thumbnails/placeholder.jpg");
            }

            // Update database
            if (update_video_metadata(entry->d_name, duration, thumbnail_path) == 0) {
                updated_count++;
            }
        }
    }

    closedir(dir);

    printf("✅ Metadata update complete: %d videos updated\n", updated_count);
}
