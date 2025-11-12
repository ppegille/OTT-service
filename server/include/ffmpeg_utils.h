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

#endif // FFMPEG_UTILS_H
