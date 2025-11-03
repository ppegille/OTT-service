# OTT Video Streaming Server - System Architecture

## 1. System Overview

This document describes the complete architecture for an OTT (Over-The-Top) video streaming platform that supports multiple concurrent users, video playback with resume functionality, and efficient HTTP-based streaming.

### Key Technologies
- **Backend**: C-based HTTP server with multi-threading
- **Frontend**: HTML5, CSS3, JavaScript
- **Video Processing**: FFmpeg
- **Database**: SQLite3
- **Protocol**: HTTP with Range Request support (RFC 7233)

---

## 2. System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                         CLIENT TIER (Web Browser)                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐              │
│  │ Login Page   │  │ Video List   │  │ Video Player │              │
│  │ (index.html) │─▶│ (list.html)  │─▶│ (player.html)│              │
│  └──────────────┘  └──────────────┘  └──────────────┘              │
│         │                 │                  │                       │
│         └─────────────────┴──────────────────┘                       │
│                           │                                          │
│                     JavaScript API Client                            │
│                    (fetch API, XMLHttpRequest)                       │
│                                                                       │
└───────────────────────────────┬─────────────────────────────────────┘
                                │
                         HTTP/HTTPS
                                │
┌───────────────────────────────▼─────────────────────────────────────┐
│                      APPLICATION TIER (C Server)                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                       │
│  ┌────────────────────────────────────────────────────────────┐     │
│  │                   HTTP Server (main.c)                      │     │
│  │                    TCP Socket (Port 8080)                   │     │
│  │              Multi-threaded Request Handler                 │     │
│  └───────┬────────────────────────────────────────────────────┘     │
│          │                                                            │
│  ┌───────▼──────────┐  ┌──────────────┐  ┌──────────────────┐      │
│  │ Request Router   │  │ Session Mgr  │  │ Thread Pool      │      │
│  │ (router.c)       │  │ (session.c)  │  │ (thread_pool.c)  │      │
│  └───┬──────────────┘  └──────┬───────┘  └──────────────────┘      │
│      │                         │                                     │
│  ┌───▼────────┐  ┌────────▼───┐  ┌──────────────┐  ┌────────────┐ │
│  │   Auth     │  │  Streaming │  │    Video     │  │  Database  │ │
│  │  Module    │  │   Module   │  │  Management  │  │   Module   │ │
│  │ (auth.c)   │  │(streaming.c)│ │  (video.c)   │  │   (db.c)   │ │
│  └────────────┘  └─────────────┘ └──────────────┘  └─────┬──────┘ │
│        │               │                │                  │         │
│        └───────────────┴────────────────┴──────────────────┘         │
│                                │                                     │
└────────────────────────────────┼─────────────────────────────────────┘
                                 │
┌────────────────────────────────▼─────────────────────────────────────┐
│                         DATA TIER                                     │
├──────────────────────────────────────────────────────────────────────┤
│                                                                       │
│  ┌──────────────────┐  ┌─────────────────┐  ┌──────────────────┐   │
│  │  SQLite Database │  │  Video Files    │  │   Thumbnails     │   │
│  │   (ott.db)       │  │  (videos/*.mp4) │  │(thumbnails/*.jpg)│   │
│  │                  │  │                 │  │                  │   │
│  │  - users         │  │  - sample.mp4   │  │  - sample.jpg    │   │
│  │  - videos        │  │  - movie1.mp4   │  │  - movie1.jpg    │   │
│  │  - watch_history │  │  - movie2.mp4   │  │  - movie2.jpg    │   │
│  │  - sessions      │  │                 │  │                  │   │
│  └──────────────────┘  └─────────────────┘  └──────────────────┘   │
│                                                                       │
└───────────────────────────────────────────────────────────────────────┘
```

---

## 3. Component Descriptions

### 3.1 Client Tier Components

#### Login Page (index.html)
- User authentication interface
- Form validation
- Session cookie management
- Redirect to video list on success

#### Video List Page (list.html)
- Display grid of available videos
- Show thumbnails and metadata
- Play/Resume button logic
- Search and filter functionality

#### Video Player Page (player.html)
- HTML5 video player with controls
- Resume dialog for continuing playback
- Custom seek functionality
- Progress tracking and reporting

### 3.2 Application Tier Components

#### HTTP Server (main.c)
- TCP socket listener on port 8080
- Multi-threaded connection handler
- Thread pool management (max 50 threads)
- Request parsing and routing

#### Request Router (router.c)
- URL path parsing
- Route to appropriate handler
- Static file serving (HTML/CSS/JS)
- API endpoint routing

#### Session Manager (session.c)
- Session ID generation (UUID)
- Session storage and validation
- Timeout management (30 min)
- Cookie-based session tracking

#### Authentication Module (auth.c)
- User credential verification
- Password hashing (SHA-256)
- Login/logout handling
- Session creation/destruction

#### Streaming Module (streaming.c)
- HTTP Range Request handling (RFC 7233)
- Partial content delivery (206 response)
- Byte range parsing
- Efficient file I/O with buffering
- Content-Type detection

#### Video Management Module (video.c)
- Video metadata extraction
- Thumbnail generation via FFmpeg
- Video indexing and cataloging
- File system monitoring

#### Database Module (db.c)
- SQLite connection pooling
- Prepared statement management
- Transaction handling
- Query execution and result parsing

### 3.3 Data Tier Components

#### SQLite Database (ott.db)
- User account information
- Video metadata and catalog
- Watch history and positions
- Active session data

#### File Storage
- Video files in MP4 format
- Generated thumbnail images
- Static web assets (HTML/CSS/JS)

---

## 4. Database Schema Design

```sql
-- Users table
CREATE TABLE users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash CHAR(64) NOT NULL,  -- SHA-256 hash
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_login DATETIME,
    INDEX idx_username (username)
);

-- Videos table
CREATE TABLE videos (
    video_id INTEGER PRIMARY KEY AUTOINCREMENT,
    filename VARCHAR(255) UNIQUE NOT NULL,
    title VARCHAR(255) NOT NULL,
    description TEXT,
    duration INTEGER NOT NULL,  -- in seconds
    file_size BIGINT NOT NULL,  -- in bytes
    file_path VARCHAR(512) NOT NULL,
    thumbnail_path VARCHAR(512),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_filename (filename),
    INDEX idx_title (title)
);

-- Watch history table
CREATE TABLE watch_history (
    history_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    video_id INTEGER NOT NULL,
    last_position INTEGER DEFAULT 0,  -- in seconds
    last_watched DATETIME DEFAULT CURRENT_TIMESTAMP,
    watch_count INTEGER DEFAULT 1,
    completed BOOLEAN DEFAULT 0,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (video_id) REFERENCES videos(video_id) ON DELETE CASCADE,
    UNIQUE (user_id, video_id),
    INDEX idx_user_video (user_id, video_id),
    INDEX idx_last_watched (last_watched)
);

-- Sessions table
CREATE TABLE sessions (
    session_id CHAR(36) PRIMARY KEY,  -- UUID
    user_id INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    expires_at DATETIME NOT NULL,
    ip_address VARCHAR(45),  -- IPv6 compatible
    user_agent TEXT,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX idx_expires (expires_at),
    INDEX idx_user_id (user_id)
);

-- Sample data
INSERT INTO users (username, password_hash) VALUES
('admin', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918'),  -- 'admin'
('user1', '0a041b9462caa4a31bac3567e0b6e6fd9100787db2ab433d96f6d178cabfce90');  -- 'user1'
```

### Entity Relationship Diagram

```
┌─────────────┐         ┌──────────────┐         ┌─────────────┐
│    users    │         │ watch_history│         │   videos    │
├─────────────┤         ├──────────────┤         ├─────────────┤
│ user_id (PK)│◄────────┤ history_id   │────────▶│ video_id(PK)│
│ username    │         │ user_id (FK) │         │ filename    │
│ password_   │         │ video_id (FK)│         │ title       │
│   hash      │         │ last_position│         │ description │
│ created_at  │         │ last_watched │         │ duration    │
│ last_login  │         │ watch_count  │         │ file_size   │
└─────┬───────┘         │ completed    │         │ file_path   │
      │                 └──────────────┘         │ thumbnail_  │
      │                                          │   path      │
      │                                          │ created_at  │
      │                 ┌──────────────┐         └─────────────┘
      │                 │   sessions   │
      │                 ├──────────────┤
      └────────────────▶│ session_id   │
                        │ user_id (FK) │
                        │ created_at   │
                        │ expires_at   │
                        │ ip_address   │
                        │ user_agent   │
                        └──────────────┘
```

---

## 5. API Endpoint Design

### 5.1 Authentication Endpoints

#### POST /api/auth/login
Login and create session.

**Request:**
```json
{
  "username": "user1",
  "password": "user1"
}
```

**Response (Success - 200 OK):**
```json
{
  "status": "success",
  "session_id": "550e8400-e29b-41d4-a716-446655440000",
  "user": {
    "user_id": 1,
    "username": "user1"
  }
}
```
**Headers:**
```
Set-Cookie: session_id=550e8400-e29b-41d4-a716-446655440000; HttpOnly; Path=/; Max-Age=1800
```

**Response (Failure - 401 Unauthorized):**
```json
{
  "status": "error",
  "message": "Invalid username or password"
}
```

#### POST /api/auth/logout
Logout and destroy session.

**Request Headers:**
```
Cookie: session_id=550e8400-e29b-41d4-a716-446655440000
```

**Response (200 OK):**
```json
{
  "status": "success",
  "message": "Logged out successfully"
}
```

#### GET /api/auth/validate
Validate current session.

**Request Headers:**
```
Cookie: session_id=550e8400-e29b-41d4-a716-446655440000
```

**Response (200 OK):**
```json
{
  "status": "valid",
  "user_id": 1,
  "username": "user1"
}
```

### 5.2 Video Endpoints

#### GET /api/videos
Get list of all available videos.

**Request Headers:**
```
Cookie: session_id=550e8400-e29b-41d4-a716-446655440000
```

**Response (200 OK):**
```json
{
  "status": "success",
  "videos": [
    {
      "video_id": 1,
      "title": "Sample Movie 1",
      "description": "A sample video",
      "duration": 3600,
      "thumbnail_url": "/thumbnails/sample1.jpg",
      "last_position": 1200,
      "watch_count": 3,
      "completed": false
    },
    {
      "video_id": 2,
      "title": "Sample Movie 2",
      "description": "Another video",
      "duration": 5400,
      "thumbnail_url": "/thumbnails/sample2.jpg",
      "last_position": 0,
      "watch_count": 0,
      "completed": false
    }
  ]
}
```

#### GET /api/videos/:video_id
Get metadata for specific video.

**Response (200 OK):**
```json
{
  "status": "success",
  "video": {
    "video_id": 1,
    "title": "Sample Movie 1",
    "description": "A sample video",
    "duration": 3600,
    "file_size": 104857600,
    "thumbnail_url": "/thumbnails/sample1.jpg",
    "last_position": 1200,
    "watch_count": 3,
    "completed": false
  }
}
```

### 5.3 Streaming Endpoints

#### GET /api/stream/:video_id
Stream video content with Range Request support.

**Request Headers:**
```
Cookie: session_id=550e8400-e29b-41d4-a716-446655440000
Range: bytes=0-1048575
```

**Response (206 Partial Content):**
```
HTTP/1.1 206 Partial Content
Content-Type: video/mp4
Content-Length: 1048576
Content-Range: bytes 0-1048575/104857600
Accept-Ranges: bytes

[binary video data]
```

**Response (200 OK - Full Content):**
```
HTTP/1.1 200 OK
Content-Type: video/mp4
Content-Length: 104857600
Accept-Ranges: bytes

[binary video data]
```

#### GET /thumbnails/:filename
Serve thumbnail images.

**Response (200 OK):**
```
HTTP/1.1 200 OK
Content-Type: image/jpeg
Content-Length: 15234

[binary image data]
```

### 5.4 Watch History Endpoints

#### POST /api/history/update
Update watch position for a video.

**Request:**
```json
{
  "video_id": 1,
  "position": 1800,
  "completed": false
}
```

**Response (200 OK):**
```json
{
  "status": "success",
  "message": "Watch position updated"
}
```

#### GET /api/history/:video_id
Get watch history for specific video.

**Response (200 OK):**
```json
{
  "status": "success",
  "history": {
    "video_id": 1,
    "last_position": 1800,
    "last_watched": "2025-10-27T14:30:00Z",
    "watch_count": 5,
    "completed": false
  }
}
```

#### GET /api/history
Get all watch history for current user.

**Response (200 OK):**
```json
{
  "status": "success",
  "history": [
    {
      "video_id": 1,
      "title": "Sample Movie 1",
      "last_position": 1800,
      "last_watched": "2025-10-27T14:30:00Z",
      "watch_count": 5,
      "completed": false
    }
  ]
}
```

### 5.5 Static File Endpoints

#### GET /
Serve login page (index.html)

#### GET /list.html
Serve video list page

#### GET /player.html
Serve video player page

#### GET /css/:filename
Serve CSS files

#### GET /js/:filename
Serve JavaScript files

---

## 6. Client-Server Communication Flow

### 6.1 User Login Flow

```
┌────────┐                    ┌────────────┐                 ┌──────────┐
│ Client │                    │   Server   │                 │ Database │
└───┬────┘                    └─────┬──────┘                 └────┬─────┘
    │                               │                             │
    │ 1. GET /                      │                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │ 2. index.html                 │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
    │ 3. POST /api/auth/login       │                             │
    │   {username, password}        │                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │                               │ 4. Verify credentials       │
    │                               │────────────────────────────▶│
    │                               │                             │
    │                               │ 5. User record              │
    │                               │◀────────────────────────────│
    │                               │                             │
    │                               │ 6. Create session           │
    │                               │────────────────────────────▶│
    │                               │                             │
    │                               │ 7. Session ID               │
    │                               │◀────────────────────────────│
    │                               │                             │
    │ 8. 200 OK + Set-Cookie        │                             │
    │   session_id=XXX              │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
    │ 9. Redirect to /list.html     │                             │
    │                               │                             │
```

### 6.2 Video List Display Flow

```
┌────────┐                    ┌────────────┐                 ┌──────────┐
│ Client │                    │   Server   │                 │ Database │
└───┬────┘                    └─────┬──────┘                 └────┬─────┘
    │                               │                             │
    │ 1. GET /list.html             │                             │
    │   Cookie: session_id=XXX      │                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │                               │ 2. Validate session         │
    │                               │────────────────────────────▶│
    │                               │                             │
    │                               │ 3. Session valid            │
    │                               │◀────────────────────────────│
    │                               │                             │
    │ 4. list.html                  │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
    │ 5. GET /api/videos            │                             │
    │   Cookie: session_id=XXX      │                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │                               │ 6. Query videos + history   │
    │                               │────────────────────────────▶│
    │                               │                             │
    │                               │ 7. Video list with history  │
    │                               │◀────────────────────────────│
    │                               │                             │
    │ 8. JSON response              │                             │
    │   [videos with positions]     │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
    │ 9. GET /thumbnails/sample1.jpg│                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │ 10. Image data                │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
```

### 6.3 Video Streaming Flow with Resume

```
┌────────┐                    ┌────────────┐                 ┌──────────┐
│ Client │                    │   Server   │                 │ Database │
└───┬────┘                    └─────┬──────┘                 └────┬─────┘
    │                               │                             │
    │ 1. GET /player.html?id=1      │                             │
    │   Cookie: session_id=XXX      │                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │ 2. player.html                │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
    │ 3. GET /api/history/1         │                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │                               │ 4. Query watch history      │
    │                               │────────────────────────────▶│
    │                               │                             │
    │                               │ 5. {last_position: 1200}    │
    │                               │◀────────────────────────────│
    │                               │                             │
    │ 6. JSON {last_position: 1200} │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
    │ [User chooses Resume]          │                             │
    │                               │                             │
    │ 7. GET /api/stream/1          │                             │
    │   Range: bytes=50000000-      │                             │
    │   (calculated from position)  │                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │                               │ 8. Open video file          │
    │                               │    Seek to byte offset      │
    │                               │                             │
    │ 9. 206 Partial Content        │                             │
    │    Content-Range: bytes       │                             │
    │    50000000-104857599/        │                             │
    │    104857600                  │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
    │ 10. [User watches video]       │                             │
    │                               │                             │
    │ 11. POST /api/history/update  │                             │
    │     {video_id: 1,             │                             │
    │      position: 1500}          │                             │
    │     (every 30 seconds)        │                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │                               │ 12. Update watch position   │
    │                               │────────────────────────────▶│
    │                               │                             │
    │ 13. 200 OK                    │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
```

### 6.4 Custom Start Position Flow

```
┌────────┐                    ┌────────────┐                 ┌──────────┐
│ Client │                    │   Server   │                 │ Database │
└───┬────┘                    └─────┬──────┘                 └────┬─────┘
    │                               │                             │
    │ [User enters time: 10:30]      │                             │
    │                               │                             │
    │ 1. GET /api/videos/1          │                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │                               │ 2. Query video metadata     │
    │                               │────────────────────────────▶│
    │                               │                             │
    │                               │ 3. Video info (duration,    │
    │                               │    file_size)               │
    │                               │◀────────────────────────────│
    │                               │                             │
    │ 4. JSON {duration: 3600, ...} │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
    │ [Calculate byte offset:        │                             │
    │  position_sec = 630            │                             │
    │  byte_offset = (file_size /    │                             │
    │    duration) * position_sec]   │                             │
    │                               │                             │
    │ 5. GET /api/stream/1          │                             │
    │   Range: bytes=18350000-      │                             │
    │──────────────────────────────▶│                             │
    │                               │                             │
    │ 6. 206 Partial Content        │                             │
    │    [Video data from 10:30]    │                             │
    │◀──────────────────────────────│                             │
    │                               │                             │
```

### 6.5 Concurrent User Handling Flow

```
┌─────────┐  ┌─────────┐  ┌─────────┐        ┌──────────────┐
│ User A  │  │ User B  │  │ User C  │        │    Server    │
└────┬────┘  └────┬────┘  └────┬────┘        └──────┬───────┘
     │            │            │                     │
     │ Stream     │            │                     │
     │ Request    │            │                     │
     │───────────────────────────────────────────────▶
     │            │            │              ┌──────▼──────┐
     │            │            │              │  Main Thread │
     │            │            │              │  (Listener)  │
     │            │            │              └──────┬───────┘
     │            │            │                     │
     │            │            │              ┌──────▼──────────┐
     │            │            │              │  Thread Pool    │
     │            │            │              │  (Max 50 threads)│
     │            │            │              └──────┬──────────┘
     │            │            │                     │
     │            │ Stream     │              ┌──────▼──────┐
     │            │ Request    │              │  Thread 1   │
     │            │──────────────────────────▶│  (User A)   │
     │            │            │              └─────────────┘
     │            │            │                     │
     │            │            │ Stream       ┌──────▼──────┐
     │            │            │ Request      │  Thread 2   │
     │            │            │─────────────▶│  (User B)   │
     │            │            │              └─────────────┘
     │            │            │                     │
     │◀───────────────────────────────────────┬─────▼──────┐
     │  Video    │            │              │  Thread 3   │
     │  Data     │            │              │  (User C)   │
     │            │            │              └─────┬───────┘
     │            │            │                    │
     │            │◀───────────────────────────────┘
     │            │  Video    │
     │            │  Data     │
     │            │            │
     │            │            │◀─────────────────┘
     │            │            │  Video
     │            │            │  Data
```

---

## 7. Threading Model

### Thread Pool Architecture

```
┌──────────────────────────────────────────────────────────┐
│                      Main Thread                         │
│                   (TCP Listener)                         │
└────────────────────────┬─────────────────────────────────┘
                         │
                         │ accept() connections
                         │
         ┌───────────────▼──────────────────┐
         │      Connection Queue            │
         │      (Thread-safe FIFO)          │
         └───────────────┬──────────────────┘
                         │
         ┌───────────────▼──────────────────┐
         │         Thread Pool              │
         │      (Pre-spawned threads)       │
         │                                  │
         │  ┌──────┐  ┌──────┐  ┌──────┐   │
         │  │Thread│  │Thread│  │Thread│   │
         │  │  1   │  │  2   │  │ ...  │   │
         │  └───┬──┘  └───┬──┘  └───┬──┘   │
         └──────┼─────────┼─────────┼───────┘
                │         │         │
                │         │         │
         ┌──────▼─────────▼─────────▼───────┐
         │    Request Handler Function      │
         │                                  │
         │  1. Parse HTTP request           │
         │  2. Validate session             │
         │  3. Route to handler             │
         │  4. Process request              │
         │  5. Send response                │
         │  6. Close connection             │
         └──────────────────────────────────┘
```

### Thread Synchronization

- **Mutex locks** for shared resources:
  - Database connection pool
  - Session storage
  - File descriptor pool

- **Read-write locks** for:
  - Video metadata cache
  - Session validation cache

---

## 8. Security Considerations

### 8.1 Authentication Security
- Password hashing with SHA-256
- Session tokens using UUID v4
- HttpOnly cookies to prevent XSS
- Session expiration (30 minutes)
- IP address validation (optional)

### 8.2 Input Validation
- SQL injection prevention via prepared statements
- Path traversal prevention in file requests
- Video ID validation (integer only)
- Position validation (0 <= position <= duration)

### 8.3 Access Control
- Session validation for all API endpoints
- Video access restricted to authenticated users
- File system access limited to designated directories

---

## 9. Performance Optimizations

### 9.1 Caching Strategy
- Video metadata cached in memory (LRU cache)
- Thumbnail pre-generation on video upload
- Session cache with periodic cleanup
- Static file caching with ETag headers

### 9.2 Streaming Optimizations
- Buffered file I/O (64KB buffer)
- Range request support for seeking
- Concurrent connection limit per user (3 streams)
- Bandwidth throttling (optional)

### 9.3 Database Optimizations
- Prepared statement caching
- Index on frequently queried columns
- Connection pooling (5-10 connections)
- Batch updates for watch position

---

## 10. File Structure

```
ott-streaming-server/
├── server/
│   ├── src/
│   │   ├── main.c               # Entry point, HTTP server
│   │   ├── router.c             # Request routing
│   │   ├── auth.c               # Authentication module
│   │   ├── session.c            # Session management
│   │   ├── streaming.c          # Video streaming with Range requests
│   │   ├── video.c              # Video management
│   │   ├── db.c                 # Database operations
│   │   ├── thread_pool.c        # Thread pool implementation
│   │   ├── utils.c              # Utility functions
│   │   └── http.c               # HTTP parsing and response
│   ├── include/
│   │   ├── server.h             # Main server structures
│   │   ├── auth.h
│   │   ├── streaming.h
│   │   ├── db.h
│   │   └── utils.h
│   └── Makefile                 # Build configuration
├── client/
│   ├── index.html               # Login page
│   ├── list.html                # Video list page
│   ├── player.html              # Video player page
│   ├── css/
│   │   ├── style.css            # Main stylesheet
│   │   └── player.css           # Player-specific styles
│   └── js/
│       ├── auth.js              # Login functionality
│       ├── list.js              # Video list handling
│       └── player.js            # Video player controls
├── videos/                      # Video storage directory
│   ├── sample1.mp4
│   └── sample2.mp4
├── thumbnails/                  # Thumbnail storage
│   ├── sample1.jpg
│   └── sample2.jpg
├── database/
│   ├── schema.sql               # Database schema
│   ├── init.sql                 # Initial data
│   └── ott.db                   # SQLite database file
├── scripts/
│   ├── generate_thumbnail.sh    # FFmpeg thumbnail script
│   └── init_db.sh               # Database initialization
├── README.md                    # Project overview
├── README_ARCHITECTURE.md       # This file
└── .gitignore
```

---

## 11. Build and Deployment

### 11.1 Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install build-essential sqlite3 libsqlite3-dev ffmpeg

# CentOS/RHEL
sudo yum install gcc sqlite-devel ffmpeg
```

### 11.2 Build Instructions
```bash
cd server
make clean
make
```

### 11.3 Database Initialization
```bash
cd database
sqlite3 ott.db < schema.sql
sqlite3 ott.db < init.sql
```

### 11.4 Generate Thumbnails
```bash
cd scripts
./generate_thumbnail.sh ../videos/sample1.mp4 ../thumbnails/sample1.jpg
```

### 11.5 Run Server
```bash
cd server
./ott_server 8080
```

### 11.6 Access Web Interface
```
http://localhost:8080/
```

---

## 12. Testing Requirements

### 12.1 Functional Testing
- [ ] User login/logout
- [ ] Video list display with thumbnails
- [ ] Video playback from beginning
- [ ] Resume playback from last position
- [ ] Custom start position playback
- [ ] Watch position tracking (every 30s)
- [ ] Session timeout after 30 minutes

### 12.2 Performance Testing
- [ ] 2+ concurrent users streaming different videos
- [ ] 5+ concurrent users streaming same video
- [ ] Bandwidth usage monitoring
- [ ] Memory leak testing (valgrind)
- [ ] Thread pool stress testing (50+ connections)

### 12.3 Security Testing
- [ ] SQL injection attempts
- [ ] Path traversal attempts
- [ ] Session hijacking resistance
- [ ] Unauthorized access prevention
- [ ] XSS prevention in user inputs

---

## 13. Future Enhancements

### Phase 2 Features
- User registration functionality
- Video upload interface
- Playlist management
- Video quality selection (adaptive bitrate)
- Subtitle support

### Phase 3 Features
- Live streaming support
- Content recommendation algorithm
- Social features (comments, ratings)
- Mobile app support
- CDN integration

---

## 14. Troubleshooting

### Common Issues

**Issue: Video won't play**
- Check file permissions on video directory
- Verify video file is valid MP4
- Check browser console for errors
- Verify session is valid

**Issue: Thumbnails not displaying**
- Run thumbnail generation script
- Check thumbnail directory permissions
- Verify FFmpeg is installed

**Issue: Multiple users fail to connect**
- Check thread pool size (increase if needed)
- Monitor system resources (ulimit -n)
- Check for port exhaustion
- Review server logs for errors

**Issue: Session expires too quickly**
- Adjust SESSION_TIMEOUT in session.c
- Check system clock synchronization
- Verify session cleanup interval

---

## 15. References

- RFC 7233 - HTTP Range Requests: https://tools.ietf.org/html/rfc7233
- FFmpeg Documentation: https://ffmpeg.org/documentation.html
- SQLite C API: https://www.sqlite.org/capi3ref.html
- HTTP/1.1 Specification: https://tools.ietf.org/html/rfc2616
- POSIX Threads Programming: https://www.opengroup.org/austin/papers/posix_threads.html

---

**Document Version:** 1.0
**Last Updated:** 2025-10-27
**Author:** System Architecture Team
