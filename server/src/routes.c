/*
 * OTT Streaming Server - Route Handler Implementation
 *
 * Table-driven routing system with handler functions for all API endpoints.
 * Extracted from main.c to improve maintainability.
 *
 * Author: Network Programming Final Project
 * Date: 2025-11-13
 */

#include "../include/routes.h"
#include "../include/database.h"
#include "../include/json.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// ============================================================================
// Forward Declarations
// ============================================================================

void handle_get_hls_status(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_favicon(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Extract request body from HTTP buffer
 */
static const char* extract_body(const char* buffer) {
    const char* body = strstr(buffer, "\r\n\r\n");
    return body ? (body + 4) : NULL;
}

/**
 * Parse query parameter from URL
 */
static int parse_query_param(const char* path, const char* param_name, char* output, size_t max_len) {
    const char* query_start = strchr(path, '?');
    if (!query_start) {
        return 0;
    }

    query_start++; // Skip '?'

    char search[64];
    snprintf(search, sizeof(search), "%s=", param_name);

    const char* param = strstr(query_start, search);
    if (!param) {
        return 0;
    }

    param += strlen(search);

    // Extract value until '&' or end
    size_t i = 0;
    while (param[i] && param[i] != '&' && i < max_len - 1) {
        output[i] = param[i];
        i++;
    }
    output[i] = '\0';

    return (i > 0) ? 1 : 0;
}

// ============================================================================
// Route Table
// ============================================================================

static Route routes[] = {
    // Public routes (no auth required)
    {"POST", "/login", handle_post_login, 0, 0},
    {"POST", "/api/register", handle_post_register, 0, 0},
    {"GET", "/login", handle_get_login, 0, 0},
    {"GET", "/", handle_get_root, 0, 0},
    {"GET", "/favicon.ico", handle_favicon, 0, 0},  // Prevent 404 errors

    // Protected API routes (auth required)
    {"GET", "/api/videos", handle_get_api_videos, 1, 0},
    {"GET", "/api/user", handle_get_api_user, 1, 0},
    {"POST", "/api/watch-progress", handle_post_watch_progress, 1, 0},
    {"GET", "/api/watch-history/", handle_get_watch_history, 1, 1},  // prefix match
    {"POST", "/api/logout", handle_post_logout, 1, 0},
    {"GET", "/api/recommendations", handle_get_recommendations, 1, 0},
    {"GET", "/api/search", handle_get_search, 1, 0},
    {"GET", "/api/genres", handle_get_genres, 1, 0},
    {"GET", "/api/genres/", handle_get_genre_videos, 1, 1},  // prefix match
    {"GET", "/api/watchlist", handle_get_watchlist, 1, 0},
    {"POST", "/api/watchlist", handle_post_watchlist_add, 1, 0},
    {"DELETE", "/api/watchlist/", handle_delete_watchlist_remove, 1, 1},  // prefix match
    {"GET", "/api/hls/status/", handle_get_hls_status, 1, 1},  // prefix match

    // Static file serving
    {"GET", "/login.html", handle_static_file, 0, 0},  // Login page (no auth required)
    {"GET", "/player.html", handle_static_file, 1, 0},
    {"GET", "/gallery.html", handle_static_file, 1, 0},
    {"GET", "/css/", handle_static_file, 0, 1},  // CSS files (no auth required)
    {"GET", "/js/", handle_static_file, 0, 1},  // JavaScript files (no auth required)
    {"GET", "/videos/", handle_video_stream, 1, 1},  // prefix match
    {"GET", "/thumbnails/", handle_thumbnail, 1, 1},  // prefix match
    {"GET", "/hls/", handle_hls_file, 1, 1},  // prefix match

    // Terminator
    {NULL, NULL, NULL, 0, 0}
};

// ============================================================================
// Route Dispatcher
// ============================================================================

int dispatch_route(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    for (int i = 0; routes[i].handler != NULL; i++) {
        int match = 0;

        // Check method
        if (strcmp(routes[i].method, req->method) != 0) {
            continue;
        }

        // Check path (exact or prefix match)
        if (routes[i].is_prefix) {
            // Prefix match
            match = (strncmp(routes[i].path, req->path, strlen(routes[i].path)) == 0);
        } else {
            // Exact match
            match = (strcmp(routes[i].path, req->path) == 0);
        }

        if (!match) {
            continue;
        }

        // Found matching route - call handler
        routes[i].handler(client_fd, req, session_id, buffer);
        return 1;
    }

    // No matching route found
    return 0;
}

// ============================================================================
// Public Route Handlers
// ============================================================================

void handle_get_login(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)session_id;  // unused
    (void)buffer;  // unused

    req->range = (Range){0, 0, 0};
    stream_file(client_fd, "../client/login.html", req->range);
}

void handle_post_login(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused
    (void)session_id;  // unused

    const char* body = extract_body(buffer);
    if (body) {
        handle_login(client_fd, body);
    } else {
        redirect_to_login(client_fd);
    }
}

void handle_post_register(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused
    (void)session_id;  // unused

    const char* body = extract_body(buffer);
    if (!body) {
        send_json_error(client_fd, 400, "Missing request body");
    } else {
        handle_registration(client_fd, body);
    }
}

void handle_get_root(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)buffer;  // unused

    if (session_id && strlen(session_id) > 0 && validate_session(session_id)) {
        // Valid session - serve gallery
        refresh_session(session_id);
        printf("  [Route] Valid session, serving gallery\n");
        req->range = (Range){0, 0, 0};
        stream_file(client_fd, "../client/gallery.html", req->range);
    } else {
        // No valid session - serve login page
        printf("  [Route] No valid session, serving login\n");
        req->range = (Range){0, 0, 0};
        stream_file(client_fd, "../client/login.html", req->range);
    }
}

// ============================================================================
// Protected API Route Handlers
// ============================================================================

void handle_get_api_videos(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused
    (void)buffer;  // unused

    char json_output[8192];
    int user_id = get_user_id_from_session(session_id);

    if (user_id < 0) {
        send_json_error(client_fd, 401, "Unauthorized: Invalid session");
    } else if (get_videos_with_history(user_id, json_output, sizeof(json_output)) == 0) {
        send_json_response(client_fd, json_output);
    } else {
        send_json_error(client_fd, 500, "Internal server error");
    }
}

void handle_get_api_user(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused
    (void)buffer;  // unused

    char username[USER_ID_LENGTH];

    if (get_username_from_session(session_id, username, sizeof(username)) == 0) {
        char json_output[256];
        snprintf(json_output, sizeof(json_output), "{\"username\":\"%s\"}", username);
        send_json_response(client_fd, json_output);
    } else {
        send_json_error(client_fd, 401, "Unauthorized: Invalid session");
    }
}

void handle_post_watch_progress(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused

    int user_id = get_user_id_from_session(session_id);

    if (user_id < 0) {
        send_json_error(client_fd, 401, "Unauthorized: Invalid session");
        return;
    }

    const char* body = extract_body(buffer);
    if (!body) {
        send_json_error(client_fd, 400, "Missing request body");
        return;
    }

    // Parse JSON body
    int video_id = parse_json_int(body, "video_id");
    int position = parse_json_int(body, "position");

    if (video_id > 0 && position >= 0) {
        if (update_watch_position(user_id, video_id, position) == 0) {
            send_json_response(client_fd, "{\"status\":\"success\"}");
            printf("  [API] Watch position updated: user=%d, video=%d, pos=%d\n",
                   user_id, video_id, position);
        } else {
            send_json_error(client_fd, 500, "Failed to update watch position");
        }
    } else {
        send_json_error(client_fd, 400, "Invalid video_id or position");
    }
}

void handle_get_watch_history(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)buffer;  // unused

    int user_id = get_user_id_from_session(session_id);

    if (user_id < 0) {
        send_json_error(client_fd, 401, "Unauthorized: Invalid session");
        return;
    }

    // Extract video_id from path: /api/watch-history/{id}
    int video_id = atoi(req->path + 19);

    if (video_id > 0) {
        char json_output[512];
        if (get_watch_history_json(user_id, video_id, json_output, sizeof(json_output)) == 0) {
            send_json_response(client_fd, json_output);
        } else {
            send_json_error(client_fd, 500, "Failed to get watch history");
        }
    } else {
        send_json_error(client_fd, 400, "Invalid video_id");
    }
}

void handle_post_logout(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused
    (void)buffer;  // unused

    destroy_session(session_id);
    printf("  [API] User logged out: session=%s\n", session_id);

    // Send response with Set-Cookie to clear the session cookie
    const char* response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/json\r\n"
                          "Set-Cookie: session_id=; HttpOnly; Max-Age=0; Path=/\r\n"
                          "Content-Length: 21\r\n"
                          "\r\n"
                          "{\"status\":\"success\"}";
    send(client_fd, response, strlen(response), 0);
}

void handle_get_recommendations(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused
    (void)buffer;  // unused

    char json_output[8192];
    int user_id = get_user_id_from_session(session_id);

    if (user_id < 0) {
        send_json_error(client_fd, 401, "Unauthorized: Invalid session");
    } else if (get_recommended_videos(user_id, json_output, sizeof(json_output)) == 0) {
        send_json_response(client_fd, json_output);
    } else {
        send_json_error(client_fd, 500, "Internal server error");
    }
}

void handle_get_search(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)session_id;  // unused for search
    (void)buffer;  // unused

    char json_output[8192];
    char query[256] = "";

    if (!parse_query_param(req->path, "q", query, sizeof(query)) || strlen(query) == 0) {
        send_json_error(client_fd, 400, "Missing search query parameter");
    } else if (search_videos(query, json_output, sizeof(json_output)) == 0) {
        send_json_response(client_fd, json_output);
    } else {
        send_json_error(client_fd, 500, "Internal server error");
    }
}

void handle_get_genres(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused
    (void)session_id;  // unused
    (void)buffer;  // unused

    char json_output[4096];

    if (get_genres_json(json_output, sizeof(json_output)) == 0) {
        send_json_response(client_fd, json_output);
    } else {
        send_json_error(client_fd, 500, "Internal server error");
    }
}

void handle_get_genre_videos(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)session_id;  // unused
    (void)buffer;  // unused

    char json_output[8192];

    // Parse genre_id from path: /api/genres/{id}/videos
    int genre_id = 0;
    sscanf(req->path + 12, "%d", &genre_id);

    if (genre_id <= 0) {
        send_json_error(client_fd, 400, "Invalid genre ID");
    } else if (get_videos_by_genre(genre_id, json_output, sizeof(json_output)) == 0) {
        send_json_response(client_fd, json_output);
    } else {
        send_json_error(client_fd, 500, "Internal server error");
    }
}

void handle_get_watchlist(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused
    (void)buffer;  // unused

    char json_output[8192];
    int user_id = get_user_id_from_session(session_id);

    if (user_id < 0) {
        send_json_error(client_fd, 401, "Unauthorized");
    } else if (get_watchlist(user_id, json_output, sizeof(json_output)) == 0) {
        send_json_response(client_fd, json_output);
    } else {
        send_json_error(client_fd, 500, "Internal server error");
    }
}

void handle_post_watchlist_add(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused

    int user_id = get_user_id_from_session(session_id);

    if (user_id < 0) {
        send_json_error(client_fd, 401, "Unauthorized");
        return;
    }

    const char* body = extract_body(buffer);
    if (!body) {
        send_json_error(client_fd, 400, "Missing request body");
        return;
    }

    int video_id = parse_json_int(body, "video_id");

    if (video_id > 0) {
        if (add_to_watchlist(user_id, video_id) == 0) {
            send_json_response(client_fd, "{\"status\":\"success\"}");
        } else {
            send_json_error(client_fd, 500, "Failed to add to watchlist");
        }
    } else {
        send_json_error(client_fd, 400, "Invalid video_id");
    }
}

void handle_delete_watchlist_remove(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)buffer;  // unused

    int user_id = get_user_id_from_session(session_id);

    if (user_id < 0) {
        send_json_error(client_fd, 401, "Unauthorized");
        return;
    }

    // Extract video_id from path: /api/watchlist/{id}
    int video_id = atoi(req->path + 15);

    if (video_id > 0) {
        if (remove_from_watchlist(user_id, video_id) == 0) {
            send_json_response(client_fd, "{\"status\":\"success\"}");
        } else {
            send_json_error(client_fd, 500, "Failed to remove from watchlist");
        }
    } else {
        send_json_error(client_fd, 400, "Invalid video_id");
    }
}

// ============================================================================
// Static File Handlers
// ============================================================================

void handle_static_file(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)session_id;  // unused
    (void)buffer;  // unused

    // Serve static HTML files
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "../client%s", req->path);

    req->range = (Range){0, 0, 0};
    stream_file(client_fd, filepath, req->range);
}

void handle_video_stream(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)session_id;  // unused

    // Parse Range header for video streaming
    char range_header[256];
    if (find_header(buffer, "Range", range_header, sizeof(range_header))) {
        req->range = parse_range(range_header);
    } else {
        req->range = (Range){0, 0, 0};
    }

    // Serve video files (stored in project_root/videos/)
    // /videos/test_video.mp4 → ../videos/test_video.mp4
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "../%s", req->path + 1);  // ../ to go to project root
    stream_file(client_fd, filepath, req->range);
}

void handle_thumbnail(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)session_id;  // unused
    (void)buffer;  // unused

    // Serve thumbnail files (stored in server/thumbnails/)
    // /thumbnails/test_video.jpg → thumbnails/test_video.jpg
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s", req->path + 1);  // Skip leading '/'

    req->range = (Range){0, 0, 0};
    stream_file(client_fd, filepath, req->range);
}

void handle_hls_file(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)session_id;  // unused
    (void)buffer;  // unused

    // Serve HLS files (stored in server/hls/)
    // /hls/video_name/index.m3u8 → hls/video_name/index.m3u8
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s", req->path + 1);  // Skip leading '/'

    req->range = (Range){0, 0, 0};
    stream_file(client_fd, filepath, req->range);
}

void handle_get_hls_status(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)session_id;  // unused
    (void)buffer;  // unused

    // Extract video_id from path: /api/hls/status/{id}
    int video_id = atoi(req->path + 16);  // "/api/hls/status/" = 16 chars

    if (video_id <= 0) {
        send_json_error(client_fd, 400, "Invalid video_id");
        return;
    }

    // Get video filename from database
    char filename[256];
    if (get_video_filename(video_id, filename, sizeof(filename)) != 0) {
        send_json_error(client_fd, 404, "Video not found");
        return;
    }

    // Remove extension from filename
    char* dot = strrchr(filename, '.');
    if (dot) *dot = '\0';

    // Check if HLS directory and index.m3u8 exist
    char hls_path[MAX_PATH];
    snprintf(hls_path, sizeof(hls_path), "hls/%s/index.m3u8", filename);

    FILE* f = fopen(hls_path, "r");
    bool available = (f != NULL);
    if (f) fclose(f);

    // Send JSON response
    char json_output[256];
    snprintf(json_output, sizeof(json_output),
        "{\"available\":%s,\"path\":\"/hls/%s/index.m3u8\"}",
        available ? "true" : "false",
        filename
    );

    send_json_response(client_fd, json_output);
}

void handle_favicon(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer) {
    (void)req;  // unused
    (void)session_id;  // unused
    (void)buffer;  // unused

    // Send empty 204 No Content response for favicon
    const char* response = "HTTP/1.1 204 No Content\r\n\r\n";
    write(client_fd, response, strlen(response));
}
