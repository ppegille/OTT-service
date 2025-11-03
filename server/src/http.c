/*
 * HTTP Request/Response Handler
 *
 * Handles HTTP request parsing and response generation
 */

#include "../include/server.h"
#include <ctype.h>

/**
 * Convert hex char to int (0-15)
 */
static int hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

/**
 * URL decode (in-place)
 * Converts %XX to actual characters
 * Example: %20 -> space, %EB%A8%B8 -> 머
 */
static void url_decode(char* dst, const char* src) {
    char a, b;
    while (*src) {
        if (*src == '%' &&
            (a = src[1]) && (b = src[2]) &&
            isxdigit(a) && isxdigit(b)) {
            *dst++ = (hex_to_int(a) << 4) | hex_to_int(b);
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

/**
 * Parse HTTP request line
 * Example: "GET /video.mp4 HTTP/1.1"
 * Example: "GET /player.html?video_id=1 HTTP/1.1"
 * Example: "GET /videos/%EB%A8%B8%EB%8B%88.mp4 HTTP/1.1"
 */
HTTPRequest parse_http_request(const char* request) {
    HTTPRequest req = {0};
    char raw_path[MAX_PATH];

    // Parse request line
    sscanf(request, "%15s %511s %15s", req.method, raw_path, req.version);

    // Remove query string from path (e.g., /page.html?param=value → /page.html)
    char* query_start = strchr(raw_path, '?');
    if (query_start) {
        *query_start = '\0';  // Terminate path at '?'
    }

    // URL decode the path (e.g., %20 -> space, %EB%A8%B8 -> 머)
    url_decode(req.path, raw_path);

    return req;
}

/**
 * Find header value in HTTP request
 * Returns static buffer (not thread-safe in current MVP)
 */
const char* find_header(const char* request, const char* header_name) {
    static char value[256];
    char search[128];

    // Search for header (case-insensitive)
    snprintf(search, sizeof(search), "\n%s: ", header_name);
    const char* header_start = strcasestr(request, search);

    if (!header_start) {
        // Try without newline (first header)
        snprintf(search, sizeof(search), "%s: ", header_name);
        header_start = strcasestr(request, search);
        if (!header_start) return NULL;
    } else {
        header_start++;  // Skip the newline
    }

    header_start += strlen(header_name) + 2;  // Skip "Name: "
    const char* header_end = strstr(header_start, "\r\n");
    if (!header_end) return NULL;

    size_t len = header_end - header_start;
    if (len >= sizeof(value)) len = sizeof(value) - 1;

    strncpy(value, header_start, len);
    value[len] = '\0';

    return value;
}

/**
 * Get MIME type from filename extension
 */
const char* get_mime_type(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";

    if (strcmp(ext, ".mp4") == 0) return "video/mp4";
    if (strcmp(ext, ".html") == 0) return "text/html; charset=utf-8";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";

    return "application/octet-stream";
}

/**
 * Send 404 Not Found response
 */
void send_404(int client_fd) {
    const char* response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: 47\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>404 Not Found</h1></body></html>";

    send(client_fd, response, strlen(response), 0);
    printf("  → 404 Not Found\n");
}
