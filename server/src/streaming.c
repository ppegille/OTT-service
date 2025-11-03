/*
 * Video Streaming Module with Range Request Support
 *
 * Implements HTTP Range Requests (RFC 7233) for video streaming
 */

#include "../include/server.h"

/**
 * Parse Range header
 * Example: "bytes=0-1023" or "bytes=1000-" or "bytes=-500"
 */
Range parse_range(const char* range_header) {
    Range range = {0, -1, 0};

    if (!range_header) {
        return range;
    }

    // Check for "bytes=" prefix
    if (strncmp(range_header, "bytes=", 6) != 0) {
        return range;
    }

    range.has_range = 1;
    const char* range_str = range_header + 6;  // Skip "bytes="

    // Parse "start-end" format
    char* dash = strchr(range_str, '-');
    if (dash) {
        if (dash == range_str) {
            // Format: "-500" (last 500 bytes)
            range.start = -1;
            range.end = atol(dash + 1);
        } else {
            range.start = atol(range_str);

            // Check if end is specified
            if (*(dash + 1) != '\0') {
                range.end = atol(dash + 1);
            }
            // else: range.end = -1 means "to end of file"
        }
    }

    return range;
}

/**
 * Get file size
 */
long get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

/**
 * Stream file with Range Request support
 *
 * This is the core function that implements video streaming with seeking
 */
void stream_file(int client_fd, const char* filename, Range range) {
    // Open file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("  ✗ File not found: %s\n", filename);
        send_404(client_fd);
        return;
    }

    // Get file size
    long file_size = get_file_size(filename);
    if (file_size < 0) {
        printf("  ✗ Cannot get file size\n");
        fclose(file);
        send_404(client_fd);
        return;
    }

    // Determine range to send
    long start = 0, end = file_size - 1;

    if (range.has_range) {
        // Handle "last N bytes" format
        if (range.start == -1) {
            start = file_size - range.end;
            if (start < 0) start = 0;
            end = file_size - 1;
        } else {
            start = range.start;
            end = (range.end == -1) ? file_size - 1 : range.end;

            // Validate range
            if (start < 0 || start >= file_size) {
                printf("  ✗ Invalid range: start=%ld, file_size=%ld\n", start, file_size);

                // Send 416 Range Not Satisfiable
                char response[256];
                snprintf(response, sizeof(response),
                    "HTTP/1.1 416 Range Not Satisfiable\r\n"
                    "Content-Range: bytes */%ld\r\n"
                    "Connection: close\r\n"
                    "\r\n",
                    file_size);
                send(client_fd, response, strlen(response), 0);

                fclose(file);
                return;
            }

            // Adjust end if beyond file size
            if (end >= file_size) {
                end = file_size - 1;
            }

            // Check if end < start
            if (end < start) {
                printf("  ✗ Invalid range: end < start\n");

                char response[256];
                snprintf(response, sizeof(response),
                    "HTTP/1.1 416 Range Not Satisfiable\r\n"
                    "Content-Range: bytes */%ld\r\n"
                    "Connection: close\r\n"
                    "\r\n",
                    file_size);
                send(client_fd, response, strlen(response), 0);

                fclose(file);
                return;
            }
        }
    }

    long content_length = end - start + 1;
    const char* mime_type = get_mime_type(filename);

    // Build HTTP response header
    char header[512];
    if (range.has_range) {
        snprintf(header, sizeof(header),
            "HTTP/1.1 206 Partial Content\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "Content-Range: bytes %ld-%ld/%ld\r\n"
            "Accept-Ranges: bytes\r\n"
            "Connection: close\r\n"
            "\r\n",
            mime_type, content_length, start, end, file_size);
        printf("  → 206 Partial Content: bytes %ld-%ld/%ld (%ld bytes)\n",
               start, end, file_size, content_length);
    } else {
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "Accept-Ranges: bytes\r\n"
            "Connection: close\r\n"
            "\r\n",
            mime_type, content_length);
        printf("  → 200 OK: %ld bytes\n", content_length);
    }

    // Send response header
    if (send(client_fd, header, strlen(header), 0) < 0) {
        perror("Send header failed");
        fclose(file);
        return;
    }

    // Seek to start position
    if (fseek(file, start, SEEK_SET) != 0) {
        perror("fseek failed");
        fclose(file);
        return;
    }

    // Send file content in chunks
    char buffer[BUFFER_SIZE];
    long bytes_sent = 0;

    while (bytes_sent < content_length) {
        size_t bytes_to_send = (content_length - bytes_sent < BUFFER_SIZE)
                               ? (content_length - bytes_sent)
                               : BUFFER_SIZE;

        size_t bytes_read = fread(buffer, 1, bytes_to_send, file);
        if (bytes_read == 0) {
            if (feof(file)) {
                printf("  End of file reached\n");
                break;
            }
            if (ferror(file)) {
                perror("File read error");
                break;
            }
        }

        ssize_t sent = send(client_fd, buffer, bytes_read, 0);
        if (sent <= 0) {
            if (sent < 0) perror("Send failed");
            break;
        }

        bytes_sent += sent;
    }

    printf("  ✓ Sent %ld bytes\n", bytes_sent);

    fclose(file);
}
