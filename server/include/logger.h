/*
 * OTT Streaming Server - Logging System
 *
 * Thread-safe structured logging with multiple levels and outputs.
 * Supports both console and file logging with automatic rotation.
 *
 * Author: Network Programming Final Project
 * Date: 2025-11-13
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdbool.h>

// ============================================================================
// Log Levels
// ============================================================================

typedef enum {
    LOG_LEVEL_DEBUG = 0,    // Detailed debug information
    LOG_LEVEL_INFO = 1,     // General information
    LOG_LEVEL_WARN = 2,     // Warning messages
    LOG_LEVEL_ERROR = 3,    // Error messages
    LOG_LEVEL_FATAL = 4     // Fatal errors (program exit)
} LogLevel;

// ============================================================================
// Log Configuration
// ============================================================================

typedef struct {
    LogLevel min_level;          // Minimum level to log
    bool log_to_console;         // Enable console output
    bool log_to_file;            // Enable file output
    const char* log_file_path;   // Path to log file
    bool include_timestamp;      // Include timestamps
    bool include_pid;            // Include process ID
    bool include_tid;            // Include thread ID
    bool color_output;           // Enable colored console output
} LogConfig;

// ============================================================================
// Initialization & Shutdown
// ============================================================================

/**
 * Initialize logging system with configuration
 *
 * @param config Log configuration
 * @return 0 on success, -1 on failure
 */
int log_init(const LogConfig* config);

/**
 * Initialize with default configuration
 * Defaults: INFO level, console only, timestamps enabled
 *
 * @return 0 on success, -1 on failure
 */
int log_init_default(void);

/**
 * Shutdown logging system and flush buffers
 */
void log_shutdown(void);

// ============================================================================
// Logging Functions
// ============================================================================

/**
 * Log a message at specified level
 *
 * @param level Log level
 * @param file Source file (__FILE__)
 * @param line Line number (__LINE__)
 * @param func Function name (__func__)
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void log_message(LogLevel level, const char* file, int line, const char* func,
                 const char* format, ...) __attribute__((format(printf, 5, 6)));

// ============================================================================
// Convenience Macros
// ============================================================================

#ifdef DEBUG
    #define LOG_DEBUG(fmt, ...) \
        log_message(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(fmt, ...) ((void)0)
#endif

#define LOG_INFO(fmt, ...) \
    log_message(LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    log_message(LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    log_message(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_FATAL(fmt, ...) \
    log_message(LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

// ============================================================================
// Configuration Functions
// ============================================================================

/**
 * Set minimum log level at runtime
 *
 * @param level New minimum log level
 */
void log_set_level(LogLevel level);

/**
 * Get current minimum log level
 *
 * @return Current log level
 */
LogLevel log_get_level(void);

/**
 * Enable or disable console logging
 *
 * @param enabled true to enable, false to disable
 */
void log_set_console_output(bool enabled);

/**
 * Enable or disable file logging
 *
 * @param enabled true to enable, false to disable
 */
void log_set_file_output(bool enabled);

/**
 * Flush log buffers to file
 */
void log_flush(void);

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Get log level name as string
 *
 * @param level Log level
 * @return Level name ("DEBUG", "INFO", etc.)
 */
const char* log_level_to_string(LogLevel level);

/**
 * Get log level from string
 *
 * @param level_str Level name string
 * @return Log level or LOG_LEVEL_INFO if invalid
 */
LogLevel log_level_from_string(const char* level_str);

/**
 * Get ANSI color code for log level
 *
 * @param level Log level
 * @return ANSI color escape sequence
 */
const char* log_level_color(LogLevel level);

#endif // LOGGER_H
