# OTT Video Streaming Server

![Project Progress](https://img.shields.io/badge/Progress-75%25-yellow)
![Language](https://img.shields.io/badge/Language-C-blue)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20WSL2-orange)
![License](https://img.shields.io/badge/License-MIT-green)

A high-performance OTT (Over-The-Top) video streaming platform built in C, supporting multiple concurrent users with features like authentication, resume playback, and HTTP range requests.

## 📋 Table of Contents

- [Features](#-features)
- [Demo](#-demo)
- [Architecture](#-architecture)
- [Tech Stack](#-tech-stack)
- [Installation](#-installation)
- [Quick Start](#-quick-start)
- [Usage](#-usage)
- [API Documentation](#-api-documentation)
- [Project Structure](#-project-structure)
- [Development](#-development)
- [Testing](#-testing)
- [Roadmap](#-roadmap)
- [Contributing](#-contributing)
- [License](#-license)

## ✨ Features

### Current Features (Phase 1 & 2 - ✅ Completed)
- ✅ **Multi-User Support**: Fork-based concurrent connection handling
- ✅ **HTTP Range Requests**: RFC 7233 compliant video streaming
- ✅ **User Authentication**: SQLite-based login with SHA-256 password hashing
- ✅ **Session Management**: Cookie-based session tracking with POSIX shared memory
- ✅ **Video Streaming**: Efficient HTTP-based video delivery with seek support
- ✅ **Web Interface**: Responsive HTML5 player with modern UI

### In Development (Phase 3 - 🔄 In Progress)
- 🔄 **Video Gallery**: Thumbnail-based video listing
- 🔄 **Watch History**: Track viewing progress and resume playback
- 🔄 **FFmpeg Integration**: Automatic thumbnail generation
- 🔄 **Custom Playback**: Start from specific time positions

### Planned Features
- ⏳ Video upload interface
- ⏳ Playlist management
- ⏳ Video recommendations
- ⏳ Quality selection (adaptive bitrate)

## 🎬 Demo

```bash
# Start the server
cd server && make && ./ott_server

# Access the platform
# Browser: http://localhost:8080

# Login with test account
# Username: testuser
# Password: password123
```

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    CLIENT (Web Browser)                      │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ Login Page   │─▶│ Video Gallery│─▶│ Video Player │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└────────────────────────────┬────────────────────────────────┘
                             │ HTTP/HTTPS
┌────────────────────────────▼────────────────────────────────┐
│                    SERVER (C Application)                    │
│  ┌────────────────────────────────────────────────────┐     │
│  │         HTTP Server (Multi-Process)                 │     │
│  │    main.c → http.c → streaming.c → session.c       │     │
│  └────────────────────────────────────────────────────┘     │
└────────────────────────────┬────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────┐
│                       DATA LAYER                             │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  SQLite DB   │  │ Video Files  │  │  Thumbnails  │      │
│  │   (ott.db)   │  │  (videos/)   │  │(thumbnails/) │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
```

For detailed architecture, see [README_ARCHITECTURE.md](README_ARCHITECTURE.md)

## 🛠️ Tech Stack

### Backend
- **Language**: C (POSIX standard)
- **Concurrency**: Fork-based multi-process architecture
- **IPC**: POSIX shared memory & semaphores
- **Database**: SQLite3
- **Protocol**: HTTP/1.1 with Range Requests (RFC 7233)

### Frontend
- **Core**: HTML5, CSS3, JavaScript
- **Video**: HTML5 `<video>` element with Media Source API
- **HTTP**: Fetch API for REST communication

### Tools
- **Video Processing**: FFmpeg (thumbnail generation)
- **Build**: GNU Make
- **Platform**: Linux / WSL2

## 📦 Installation

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential sqlite3 libsqlite3-dev ffmpeg

# WSL2 (Windows Subsystem for Linux)
# See WSL_설치가이드.md for detailed setup
```

### Build from Source

```bash
# Clone the repository
git clone https://github.com/ppegille/OTT-service.git
cd OTT-service

# Build the server
cd server
make clean
make

# Initialize database
cd ../
sqlite3 server/ott.db < server/database/schema.sql
sqlite3 server/ott.db < server/database/seed.sql
```

## 🚀 Quick Start

### 1. Start the Server

```bash
cd server
./ott_server
```

Expected output:
```
===========================================
OTT Streaming Server - Enhancement Phase 2
===========================================
- Database: ✓ SQLite integration
- Authentication: ✓ SHA-256 hashing
- Session: ✓ Shared memory
===========================================

[INFO] Database opened successfully
[INFO] Session store initialized with 100 slots
[INFO] Server listening on port 8080...
```

### 2. Access Web Interface

Open your browser and navigate to:
```
http://localhost:8080/
```

### 3. Login

Use one of the test accounts:

| Username | Password | Role |
|----------|----------|------|
| testuser | password123 | User |
| alice | alice123 | User |
| bob | bob123 | User |

### 4. Watch Videos

After login, you'll be redirected to the player page where you can:
- Play videos with full HTML5 controls
- Seek to any position using the progress bar
- Resume from where you left off (coming in Phase 3)

## 📖 Usage

### Testing Concurrent Connections

Open multiple browser tabs/windows and access the server simultaneously:

```bash
# Terminal 1: Start server
cd server && ./ott_server

# Terminal 2: Run automated concurrent test
cd tests && bash concurrent_test.sh
```

The server logs will show different child process IDs for each connection:

```
[INFO] New connection from 127.0.0.1:xxxxx
[INFO] Forked child process: PID 12345
[INFO] Child 12345: Processing GET /player.html
[INFO] Child 12345: Finished, exiting
```

### Watching Server Logs

```bash
# In server directory
./ott_server 2>&1 | tee server.log
```

### Stopping the Server

```bash
# Press Ctrl+C in the terminal
# Or from another terminal:
pkill ott_server
```

## 📡 API Documentation

### Authentication

#### POST /api/auth/login
Login and create session.

**Request:**
```json
{
  "username": "testuser",
  "password": "password123"
}
```

**Response (Success):**
```json
{
  "status": "success",
  "message": "Login successful"
}
```
Sets cookie: `session_id=<UUID>`

#### POST /api/auth/logout
Logout and destroy session.

**Response:**
```json
{
  "status": "success",
  "message": "Logged out successfully"
}
```

### Video Streaming

#### GET /api/stream/:video_id
Stream video with Range Request support.

**Headers:**
```
Range: bytes=0-1048575
```

**Response:**
```
HTTP/1.1 206 Partial Content
Content-Type: video/mp4
Content-Range: bytes 0-1048575/104857600
Accept-Ranges: bytes
```

For complete API documentation, see [README_ARCHITECTURE.md](README_ARCHITECTURE.md#5-api-endpoint-design)

## 📂 Project Structure

```
OTT-service/
├── server/
│   ├── src/
│   │   ├── main.c              # Entry point, HTTP server
│   │   ├── http.c              # HTTP request/response handling
│   │   ├── streaming.c         # Range request video streaming
│   │   ├── session.c           # Session management (shared memory)
│   │   ├── database.c          # SQLite operations
│   │   ├── crypto.c            # SHA-256 password hashing
│   │   ├── json.c              # JSON parsing/generation
│   │   └── video_scanner.c     # Video file management
│   ├── include/
│   │   ├── server.h            # Main server definitions
│   │   ├── database.h          # Database interface
│   │   ├── crypto.h            # Cryptography functions
│   │   ├── json.h              # JSON utilities
│   │   └── video_scanner.h     # Video management
│   ├── database/
│   │   ├── schema.sql          # Database schema
│   │   └── seed.sql            # Test data
│   ├── Makefile                # Build configuration
│   └── ott_server              # Compiled binary
├── client/
│   ├── login.html              # Login page
│   ├── gallery.html            # Video list (in development)
│   └── player.html             # Video player
├── videos/                     # Video storage
├── thumbnails/                 # Generated thumbnails
├── tests/
│   └── concurrent_test.sh      # Automated testing
├── practice/                   # Learning exercises
│   ├── exercise1_hello_server.c
│   └── exercise2_file_server.c
├── README.md                   # This file
├── README_ARCHITECTURE.md      # Detailed architecture
└── CLAUDE.md                   # Project requirements
```

## 💻 Development

### Compilation Options

```bash
# Debug build with symbols
make DEBUG=1

# Clean build
make clean
make

# Rebuild everything
make rebuild
```

### Adding a New Feature

1. **Plan**: Document the feature in markdown
2. **Implement**: Add code to appropriate modules
3. **Test**: Create test cases
4. **Document**: Update README and architecture docs

### Code Style

- Follow POSIX standards
- Use meaningful variable names
- Comment complex logic
- Maximum line length: 100 characters
- Indent with 4 spaces

## 🧪 Testing

### Manual Testing Checklist

- [ ] Single user login and video playback
- [ ] Multiple concurrent users (2+ simultaneous streams)
- [ ] Session persistence across page reloads
- [ ] Video seeking with range requests
- [ ] Logout functionality
- [ ] Invalid login attempts
- [ ] Network interruption handling

### Automated Testing

```bash
cd tests
bash concurrent_test.sh
```

This script tests:
- Concurrent connections (3 simultaneous requests)
- Fork-based process handling
- Zombie process prevention
- Session isolation

### Memory Leak Testing

```bash
# Install valgrind
sudo apt-get install valgrind

# Run with memory checking
valgrind --leak-check=full --show-leak-kinds=all ./ott_server
```

## 🗺️ Roadmap

### ✅ Phase 1: MVP - Basic Streaming (Completed)
- HTTP server with single-user support
- Range request video streaming
- Basic web player

### ✅ Phase 2: Multi-User Support (Completed)
- Fork-based concurrent connections
- SQLite user authentication
- Session management
- Security (SHA-256 hashing)

### 🔄 Phase 3: Enhancement (In Progress - 75% Complete)
- Video gallery with thumbnails
- FFmpeg thumbnail generation
- Watch history tracking
- Resume playback feature
- Custom start position

### ⏳ Phase 4: Polish (Planned)
- Performance optimization
- Security hardening
- Comprehensive testing
- Documentation finalization
- Project report

**Target Completion:** December 10, 2025

## 🤝 Contributing

This is an academic project for Network Programming course. External contributions are not currently accepted, but feedback and suggestions are welcome!

## 📄 License

This project is developed for educational purposes as part of a university course assignment.

## 📞 Support & Documentation

- **Quick Start**: See [QUICK_START_CHECKLIST.md](QUICK_START_CHECKLIST.md)
- **Restart Guide**: See [RESTART_GUIDE.md](RESTART_GUIDE.md)
- **Architecture**: See [README_ARCHITECTURE.md](README_ARCHITECTURE.md)
- **WSL Setup**: See [WSL_설치가이드.md](WSL_설치가이드.md)
- **Project Status**: See [PROJECT_STATUS.md](PROJECT_STATUS.md)

## 🙏 Acknowledgments

- Network Programming Course, Fall 2025
- FFmpeg project for video processing tools
- SQLite team for the excellent embedded database
- All open-source contributors

---

**Built with ❤️ using C and passion for systems programming**

**Last Updated:** 2025-11-03
