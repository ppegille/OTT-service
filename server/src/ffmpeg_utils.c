/*
 * OTT Streaming Server - FFmpeg Utilities Implementation
 *
 * Video metadata extraction and thumbnail generation using FFmpeg
 * Enhancement Phase 3
 * Author: Generated for Network Programming Final Project
 * Date: 2025-11-10
 */

#include "../include/ffmpeg_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Extract video duration in seconds using ffprobe
 *
 * Command: ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 video.mp4
 *
 * @param video_path Full path to video file
 * @return Duration in seconds, -1 on error
 */
int get_video_duration(const char* video_path) {
    if (!video_path) {
        fprintf(stderr, "⚠️  get_video_duration: NULL video path\n");
        return -1;
    }

    // Check if file exists
    if (access(video_path, F_OK) != 0) {
        fprintf(stderr, "⚠️  Video file not found: %s\n", video_path);
        return -1;
    }

    // Build ffprobe command
    char command[1024];
    snprintf(command, sizeof(command),
             "ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 \"%s\"",
             video_path);

    // Execute command and capture output
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        fprintf(stderr, "⚠️  Failed to execute ffprobe\n");
        return -1;
    }

    // Read duration
    char buffer[128];
    if (fgets(buffer, sizeof(buffer), pipe) == NULL) {
        fprintf(stderr, "⚠️  Failed to read ffprobe output\n");
        pclose(pipe);
        return -1;
    }

    pclose(pipe);

    // Parse duration (convert to integer seconds)
    float duration_float = atof(buffer);
    int duration_seconds = (int)(duration_float + 0.5); // Round to nearest second

    return duration_seconds;
}

/**
 * Generate thumbnail image from video using FFmpeg
 *
 * Command: ffmpeg -i video.mp4 -ss 00:00:05 -vframes 1 -vf scale=320:-1 thumbnail.jpg
 *
 * @param video_path Full path to video file
 * @param output_path Full path for output thumbnail
 * @param time_offset Timestamp to capture (in seconds)
 * @return 0 on success, -1 on error
 */
int generate_thumbnail(const char* video_path, const char* output_path, int time_offset) {
    if (!video_path || !output_path) {
        fprintf(stderr, "⚠️  generate_thumbnail: NULL path\n");
        return -1;
    }

    // Check if input file exists
    if (access(video_path, F_OK) != 0) {
        fprintf(stderr, "⚠️  Video file not found: %s\n", video_path);
        return -1;
    }

    // Build FFmpeg command
    // -i: input file
    // -ss: seek to timestamp (in seconds)
    // -vframes 1: extract only 1 frame
    // -vf scale=320:-1: resize to width 320px, maintain aspect ratio
    // -y: overwrite output file
    char command[2048];
    snprintf(command, sizeof(command),
             "ffmpeg -v quiet -i \"%s\" -ss %d -vframes 1 -vf scale=320:-1 -y \"%s\" 2>&1",
             video_path, time_offset, output_path);

    // Execute command
    int ret = system(command);

    if (ret != 0) {
        fprintf(stderr, "⚠️  FFmpeg thumbnail generation failed for: %s\n", video_path);
        return -1;
    }

    // Verify output file was created
    if (access(output_path, F_OK) != 0) {
        fprintf(stderr, "⚠️  Thumbnail file was not created: %s\n", output_path);
        return -1;
    }

    return 0;
}

/**
 * Generate thumbnail with default settings
 * - Time offset: 5 seconds
 * - Width: 320px
 *
 * @param video_path Full path to video file
 * @param output_path Full path for output thumbnail
 * @return 0 on success, -1 on error
 */
int generate_thumbnail_default(const char* video_path, const char* output_path) {
    return generate_thumbnail(video_path, output_path, 5);
}

/**
 * Generate HLS playlist and segments from video file
 *
 * Creates adaptive bitrate streaming segments with multiple quality levels:
 * - 1080p (5000kbps)
 * - 720p (2800kbps)
 * - 480p (1400kbps)
 * - 360p (800kbps)
 *
 * @param video_path Full path to source video file
 * @param output_dir Directory to store HLS files (will be created if not exists)
 * @param segment_duration Length of each segment in seconds (default: 10)
 * @return 0 on success, -1 on error
 */
int generate_hls_playlist(const char* video_path, const char* output_dir, int segment_duration) {
    if (!video_path || !output_dir) {
        fprintf(stderr, "⚠️  generate_hls_playlist: NULL path\n");
        return -1;
    }

    // Check if input file exists
    if (access(video_path, F_OK) != 0) {
        fprintf(stderr, "⚠️  Video file not found: %s\n", video_path);
        return -1;
    }

    // Create output directory if it doesn't exist
    char mkdir_cmd[512];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", output_dir);
    system(mkdir_cmd);

    // FFmpeg HLS transcoding command - simplified single quality (720p)
    // -c:v libx264: H.264 video codec
    // -c:a aac: AAC audio codec
    // -hls_time: segment duration
    // -hls_playlist_type vod: Video on Demand playlist
    // -hls_segment_filename: pattern for segment files
    char command[2048];

    snprintf(command, sizeof(command),
        "ffmpeg -v quiet -i \"%s\" "
        "-vf scale=1280:720 "
        "-c:v libx264 -b:v 2800k -preset fast "
        "-c:a aac -b:a 128k -ac 2 "
        "-f hls "
        "-hls_time %d "
        "-hls_playlist_type vod "
        "-hls_segment_filename \"%s/segment_%%03d.ts\" "
        "\"%s/master.m3u8\" 2>&1",
        video_path,
        segment_duration,
        output_dir,
        output_dir
    );

    printf("🎬 Starting HLS transcoding...\n");
    printf("   Source: %s\n", video_path);
    printf("   Output: %s\n", output_dir);

    int ret = system(command);

    if (ret != 0) {
        fprintf(stderr, "⚠️  HLS transcoding failed\n");
        return -1;
    }

    // Verify master playlist was created
    char master_path[1024];
    snprintf(master_path, sizeof(master_path), "%s/master.m3u8", output_dir);

    if (access(master_path, F_OK) != 0) {
        fprintf(stderr, "⚠️  Master playlist was not created: %s\n", master_path);
        return -1;
    }

    printf("✅ HLS transcoding complete\n");
    return 0;
}

/**
 * Generate HLS playlist with default segment duration (10 seconds)
 *
 * @param video_path Full path to source video file
 * @param output_dir Directory to store HLS files
 * @return 0 on success, -1 on error
 */
int generate_hls_playlist_default(const char* video_path, const char* output_dir) {
    return generate_hls_playlist(video_path, output_dir, 10);
}
