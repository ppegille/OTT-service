-- OTT Streaming Server Database Schema
-- Enhancement Phase 2: User Authentication & Video Metadata
-- Created: 2025-11-03

-- Users Table: Store user credentials and information
CREATE TABLE IF NOT EXISTS users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_login DATETIME
);

-- Videos Table: Store video metadata
CREATE TABLE IF NOT EXISTS videos (
    video_id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    filename TEXT UNIQUE NOT NULL,
    thumbnail_path TEXT,
    duration INTEGER DEFAULT 0,
    file_size INTEGER DEFAULT 0,
    hls_path TEXT,
    hls_status TEXT DEFAULT 'pending',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Watch History Table: Track user viewing progress (Phase 3)
CREATE TABLE IF NOT EXISTS watch_history (
    history_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    video_id INTEGER NOT NULL,
    last_position INTEGER DEFAULT 0,
    last_watched DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY(video_id) REFERENCES videos(video_id) ON DELETE CASCADE,
    UNIQUE(user_id, video_id)
);

-- Genres Table: Store available genres (Enhancement Phase 1)
CREATE TABLE IF NOT EXISTS genres (
    genre_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,
    description TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Video Genres Table: Many-to-many relationship between videos and genres
CREATE TABLE IF NOT EXISTS video_genres (
    video_id INTEGER NOT NULL,
    genre_id INTEGER NOT NULL,
    PRIMARY KEY (video_id, genre_id),
    FOREIGN KEY(video_id) REFERENCES videos(video_id) ON DELETE CASCADE,
    FOREIGN KEY(genre_id) REFERENCES genres(genre_id) ON DELETE CASCADE
);

-- Watchlist Table: User's favorite/saved videos (Enhancement Phase 1)
CREATE TABLE IF NOT EXISTS watchlist (
    watchlist_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    video_id INTEGER NOT NULL,
    added_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY(video_id) REFERENCES videos(video_id) ON DELETE CASCADE,
    UNIQUE(user_id, video_id)
);

-- Create indexes for faster queries
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_videos_filename ON videos(filename);
CREATE INDEX IF NOT EXISTS idx_watch_history_user ON watch_history(user_id);
CREATE INDEX IF NOT EXISTS idx_watch_history_video ON watch_history(video_id);
CREATE INDEX IF NOT EXISTS idx_video_genres_video ON video_genres(video_id);
CREATE INDEX IF NOT EXISTS idx_video_genres_genre ON video_genres(genre_id);
CREATE INDEX IF NOT EXISTS idx_watchlist_user ON watchlist(user_id);
CREATE INDEX IF NOT EXISTS idx_watchlist_video ON watchlist(video_id);
