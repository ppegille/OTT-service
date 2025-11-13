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

-- Default Genres (Enhancement Phase 1)
INSERT OR IGNORE INTO genres (name, description) VALUES
    ('액션', '액션과 모험이 가득한 영상'),
    ('코미디', '유머와 재미가 넘치는 영상'),
    ('드라마', '감동적인 스토리의 영상'),
    ('다큐멘터리', '실제 이야기와 정보를 담은 영상'),
    ('공포', '스릴과 공포를 선사하는 영상'),
    ('로맨스', '사랑과 감성의 영상'),
    ('SF', '과학과 미래를 탐구하는 영상'),
    ('애니메이션', '애니메이션 작품'),
    ('스릴러', '긴장감 넘치는 영상'),
    ('가족', '가족이 함께 즐길 수 있는 영상');
