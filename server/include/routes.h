/*
 * OTT Streaming Server - Route Handler Module
 *
 * Table-driven routing system to replace monolithic main.c routing logic.
 * Reduces code duplication and makes adding new endpoints trivial.
 *
 * Author: Network Programming Final Project
 * Date: 2025-11-13
 */

#ifndef ROUTES_H
#define ROUTES_H

#include "server.h"

// ============================================================================
// Route Handler Function Type
// ============================================================================

/**
 * Route handler function signature
 * @param client_fd Socket file descriptor for the client
 * @param req Parsed HTTP request
 * @param session_id Session ID (may be empty string if no session)
 * @param buffer Full HTTP request buffer (for body extraction)
 */
typedef void (*RouteHandler)(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);

// ============================================================================
// Route Definition Structure
// ============================================================================

typedef struct {
    const char* method;        // HTTP method (GET, POST, etc.)
    const char* path;          // Exact path or prefix
    RouteHandler handler;      // Handler function
    int requires_auth;         // 1 if session required, 0 if public
    int is_prefix;             // 1 if path is prefix match, 0 if exact match
} Route;

// ============================================================================
// Route Dispatcher
// ============================================================================

/**
 * Dispatch request to appropriate handler
 * @param client_fd Socket file descriptor
 * @param req Parsed HTTP request
 * @param session_id Session ID (validated)
 * @param buffer Full HTTP request buffer
 * @return 1 if route was handled, 0 if no matching route found
 */
int dispatch_route(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);

// ============================================================================
// Route Handler Functions (implemented in routes.c)
// ============================================================================

// Public routes (no authentication required)
void handle_get_login(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_post_login(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_post_register(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_get_root(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);

// Protected API routes (authentication required)
void handle_get_api_videos(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_get_api_user(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_post_watch_progress(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_get_watch_history(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_post_logout(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_get_recommendations(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_get_search(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_get_genres(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_get_genre_videos(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_get_watchlist(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_post_watchlist_add(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_delete_watchlist_remove(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);

// Static file serving
void handle_static_file(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_video_stream(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_thumbnail(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);
void handle_hls_file(int client_fd, HTTPRequest* req, const char* session_id, const char* buffer);

#endif // ROUTES_H
