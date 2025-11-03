-- OTT Streaming Server Initial Data
-- Test users and videos for development
-- Created: 2025-11-03

-- Test Users
-- Password for all users: "password123"
-- SHA-256 Hash: ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f

INSERT OR IGNORE INTO users (username, password_hash) VALUES
    ('alice', 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f'),
    ('bob', 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f'),
    ('testuser', 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f');

-- Test Video
INSERT OR IGNORE INTO videos (title, filename, file_size) VALUES
    ('Test Video', 'test_video.mp4', 193600);
