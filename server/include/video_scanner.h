/*
 * OTT Streaming Server - Video Scanner Header
 *
 * Automatic video directory scanning and registration
 */

#ifndef VIDEO_SCANNER_H
#define VIDEO_SCANNER_H

/**
 * Scan videos directory and register new videos to database
 * Returns: number of videos registered
 */
int scan_and_register_videos(const char* video_dir);

/**
 * Update video metadata (duration, thumbnail) using FFmpeg
 * Placeholder for Phase 3 FFmpeg integration
 */
void update_video_metadata(const char* video_dir);

/**
 * Get file size in bytes
 */
long get_video_file_size(const char* filepath);

/**
 * Generate user-friendly title from filename
 */
void generate_title_from_filename(const char* filename, char* title, size_t max_len);

#endif // VIDEO_SCANNER_H
