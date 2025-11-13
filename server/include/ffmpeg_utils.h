/*
 * OTT Streaming Server - FFmpeg Utilities Header
 *
 * Video metadata extraction and thumbnail generation using FFmpeg
 * Enhancement Phase 3
 * Author: Generated for Network Programming Final Project
 * Date: 2025-11-10
 */

#ifndef FFMPEG_UTILS_H
#define FFMPEG_UTILS_H

/**
 * Extract video duration in seconds using FFmpeg
 *
 * @param video_path Full path to video file
 * @return Duration in seconds, -1 on error
 */
int get_video_duration(const char* video_path);

/**
 * Generate thumbnail image from video using FFmpeg
 *
 * @param video_path Full path to video file
 * @param output_path Full path for output thumbnail (e.g., thumbnails/video1.jpg)
 * @param time_offset Timestamp to capture (in seconds, default: 5s)
 * @return 0 on success, -1 on error
 */
int generate_thumbnail(const char* video_path, const char* output_path, int time_offset);

/**
 * Generate thumbnail with default settings (5 seconds, 320px width)
 *
 * @param video_path Full path to video file
 * @param output_path Full path for output thumbnail
 * @return 0 on success, -1 on error
 */
int generate_thumbnail_default(const char* video_path, const char* output_path);

/**
 * Generate HLS playlist and segments from video file
 *
 * Creates HLS streaming segments at 720p quality (2800kbps)
 * Optimized for fast transcoding and broad compatibility
 *
 * @param video_path Full path to source video file
 * @param output_dir Directory to store HLS files (will be created if not exists)
 * @param segment_duration Length of each segment in seconds
 * @return 0 on success, -1 on error
 */
int generate_hls_playlist(const char* video_path, const char* output_dir, int segment_duration);

/**
 * Generate HLS playlist with default segment duration (10 seconds)
 *
 * @param video_path Full path to source video file
 * @param output_dir Directory to store HLS files
 * @return 0 on success, -1 on error
 */
int generate_hls_playlist_default(const char* video_path, const char* output_dir);

#endif // FFMPEG_UTILS_H
