/*
 * OTT Streaming Server - Logging System Implementation
 *
 * Thread-safe logging with formatted output and multiple destinations.
 *
 * Author: Network Programming Final Project
 * Date: 2025-11-13
 */

#include "../include/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

// ============================================================================
// ANSI Color Codes
// ============================================================================

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GRAY    "\033[90m"

// ============================================================================
// Global State
// ============================================================================

static struct {
    LogConfig config;
    FILE* log_file;
    pthread_mutex_t mutex;
    bool initialized;
} g_logger = {
    .initialized = false
};

// ============================================================================
// Initialization & Shutdown
// ============================================================================

int log_init(const LogConfig* config) {
    if (g_logger.initialized) {
        return 0; // Already initialized
    }

    if (!config) {
        return -1;
    }

    // Copy configuration
    g_logger.config = *config;

    // Initialize mutex
    if (pthread_mutex_init(&g_logger.mutex, NULL) != 0) {
        return -1;
    }

    // Open log file if needed
    if (config->log_to_file && config->log_file_path) {
        g_logger.log_file = fopen(config->log_file_path, "a");
        if (!g_logger.log_file) {
            pthread_mutex_destroy(&g_logger.mutex);
            return -1;
        }
        // Set line buffering for better real-time logging
        setvbuf(g_logger.log_file, NULL, _IOLBF, 0);
    }

    g_logger.initialized = true;
    return 0;
}

int log_init_default(void) {
    LogConfig config = {
        .min_level = LOG_LEVEL_INFO,
        .log_to_console = true,
        .log_to_file = false,
        .log_file_path = NULL,
        .include_timestamp = true,
        .include_pid = false,
        .include_tid = false,
        .color_output = true
    };

    return log_init(&config);
}

void log_shutdown(void) {
    if (!g_logger.initialized) {
        return;
    }

    pthread_mutex_lock(&g_logger.mutex);

    if (g_logger.log_file) {
        fflush(g_logger.log_file);
        fclose(g_logger.log_file);
        g_logger.log_file = NULL;
    }

    g_logger.initialized = false;

    pthread_mutex_unlock(&g_logger.mutex);
    pthread_mutex_destroy(&g_logger.mutex);
}

// ============================================================================
// Core Logging Function
// ============================================================================

void log_message(LogLevel level, const char* file, int line, const char* func,
                 const char* format, ...) {
    if (!g_logger.initialized) {
        // Fallback to stderr if not initialized
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
        va_end(args);
        return;
    }

    // Check log level filter
    if (level < g_logger.config.min_level) {
        return;
    }

    pthread_mutex_lock(&g_logger.mutex);

    // Build log message
    char message_buffer[2048];
    char timestamp_buffer[32];
    char prefix_buffer[256];

    // Get timestamp
    if (g_logger.config.include_timestamp) {
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        strftime(timestamp_buffer, sizeof(timestamp_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    }

    // Build prefix
    prefix_buffer[0] = '\0';
    if (g_logger.config.include_timestamp) {
        strcat(prefix_buffer, timestamp_buffer);
        strcat(prefix_buffer, " ");
    }

    // Add PID if requested
    if (g_logger.config.include_pid) {
        char pid_buf[32];
        snprintf(pid_buf, sizeof(pid_buf), "[%d] ", getpid());
        strcat(prefix_buffer, pid_buf);
    }

    // Add thread ID if requested
    if (g_logger.config.include_tid) {
        char tid_buf[32];
        snprintf(tid_buf, sizeof(tid_buf), "[%lu] ", (unsigned long)pthread_self());
        strcat(prefix_buffer, tid_buf);
    }

    // Add level
    char level_buf[32];
    snprintf(level_buf, sizeof(level_buf), "[%s] ", log_level_to_string(level));
    strcat(prefix_buffer, level_buf);

    // Format user message
    va_list args;
    va_start(args, format);
    vsnprintf(message_buffer, sizeof(message_buffer), format, args);
    va_end(args);

    // Output to console
    if (g_logger.config.log_to_console) {
        FILE* out = (level >= LOG_LEVEL_ERROR) ? stderr : stdout;

        if (g_logger.config.color_output) {
            fprintf(out, "%s%s%s%s\n",
                    log_level_color(level),
                    prefix_buffer,
                    message_buffer,
                    COLOR_RESET);
        } else {
            fprintf(out, "%s%s\n", prefix_buffer, message_buffer);
        }

        fflush(out);
    }

    // Output to file
    if (g_logger.config.log_to_file && g_logger.log_file) {
        // Include source location in file logs
        fprintf(g_logger.log_file, "%s%s (%s:%d:%s)\n",
                prefix_buffer,
                message_buffer,
                file, line, func);
        fflush(g_logger.log_file);
    }

    pthread_mutex_unlock(&g_logger.mutex);

    // Exit on fatal errors
    if (level == LOG_LEVEL_FATAL) {
        log_shutdown();
        exit(EXIT_FAILURE);
    }
}

// ============================================================================
// Configuration Functions
// ============================================================================

void log_set_level(LogLevel level) {
    if (!g_logger.initialized) return;

    pthread_mutex_lock(&g_logger.mutex);
    g_logger.config.min_level = level;
    pthread_mutex_unlock(&g_logger.mutex);
}

LogLevel log_get_level(void) {
    if (!g_logger.initialized) {
        return LOG_LEVEL_INFO;
    }

    pthread_mutex_lock(&g_logger.mutex);
    LogLevel level = g_logger.config.min_level;
    pthread_mutex_unlock(&g_logger.mutex);

    return level;
}

void log_set_console_output(bool enabled) {
    if (!g_logger.initialized) return;

    pthread_mutex_lock(&g_logger.mutex);
    g_logger.config.log_to_console = enabled;
    pthread_mutex_unlock(&g_logger.mutex);
}

void log_set_file_output(bool enabled) {
    if (!g_logger.initialized) return;

    pthread_mutex_lock(&g_logger.mutex);
    g_logger.config.log_to_file = enabled;
    pthread_mutex_unlock(&g_logger.mutex);
}

void log_flush(void) {
    if (!g_logger.initialized) return;

    pthread_mutex_lock(&g_logger.mutex);

    if (g_logger.log_file) {
        fflush(g_logger.log_file);
    }

    fflush(stdout);
    fflush(stderr);

    pthread_mutex_unlock(&g_logger.mutex);
}

// ============================================================================
// Utility Functions
// ============================================================================

const char* log_level_to_string(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO ";
        case LOG_LEVEL_WARN:  return "WARN ";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_FATAL: return "FATAL";
        default:              return "UNKNOWN";
    }
}

LogLevel log_level_from_string(const char* level_str) {
    if (!level_str) return LOG_LEVEL_INFO;

    if (strcasecmp(level_str, "DEBUG") == 0) return LOG_LEVEL_DEBUG;
    if (strcasecmp(level_str, "INFO") == 0)  return LOG_LEVEL_INFO;
    if (strcasecmp(level_str, "WARN") == 0)  return LOG_LEVEL_WARN;
    if (strcasecmp(level_str, "ERROR") == 0) return LOG_LEVEL_ERROR;
    if (strcasecmp(level_str, "FATAL") == 0) return LOG_LEVEL_FATAL;

    return LOG_LEVEL_INFO; // Default
}

const char* log_level_color(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return COLOR_GRAY;
        case LOG_LEVEL_INFO:  return COLOR_CYAN;
        case LOG_LEVEL_WARN:  return COLOR_YELLOW;
        case LOG_LEVEL_ERROR: return COLOR_RED;
        case LOG_LEVEL_FATAL: return COLOR_MAGENTA;
        default:              return COLOR_RESET;
    }
}
