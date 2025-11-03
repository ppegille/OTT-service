# OTT Video Streaming Server Development Project

## Project Overview
Develop an OTT (Over-The-Top) streaming platform that allows multiple users to access and watch videos through web browsers (e.g., Chrome).

**Deadline:** December 10, 2025 (Friday)

## Core Requirements

### 1. Input Data Management
- Accept commercial MP4 video files as input
- Server maintains multiple video files in both file system and database
- Provide streaming service based on stored content

### 2. Thumbnail Extraction
- Automatically extract representative thumbnail images from video files
- Display thumbnails in the video list interface
- **Technology:** Use FFmpeg library for extraction

### 3. Web-Based User Interface
- Users access the streaming site through web browsers (Chrome, etc.)
- Default landing page: index.html with ID/Password login
- **Technology Stack:** HTML, CSS, JavaScript
- Simple and intuitive UI design

### 4. User Authentication System
- Implement ID/PASSWORD based login functionality
- Redirect to video list page upon successful authentication
- Session management for logged-in users

### 5. Video List Display
- Show all available videos managed by the server
- Each video entry includes:
  - Title
  - Thumbnail image
  - Play button

### 6. Video Selection and Playback
- Start streaming when user selects a specific video
- Smooth playback experience in the browser

### 7. Watch History Management (Resume Feature)
- Track user's viewing history
- For previously watched videos, offer options:
  - "Resume watching" from last position
  - "Start from beginning"
- Save viewing position and restore on re-login
- Provide video recommendations based on history

### 8. Custom Start Position Playback
- Allow users to start video from specific time points
- Example: "Start playback from 10:30"
- Time selection interface

### 9. High-Performance Multi-User Streaming Server
- Support simultaneous viewing by multiple users
- Efficient streaming server design considerations:
  - Server performance optimization
  - Network efficiency
- **Architecture:** TCP-based multi-programming server
  - Multi-process OR multi-thread implementation
  - Scalable connection handling

### 10. Network Programming Implementation
- Robust network programming foundation
- Handle concurrent user connections efficiently
- Implement proper error handling and recovery

## Technical Implementation Details

### Streaming Method
- Use HTTP Range Requests (HTTP 206 Partial Content)
  OR
- Implement via HTTP parameters
- Support seek functionality

### Technology Stack Recommendations
- **Backend:** C for server implementation
- **Frontend:** HTML5, CSS3, JavaScript
- **Video Processing:** FFmpeg
- **Database:** MySQL/PostgreSQL or SQLite for metadata
- **Protocol:** HTTP/HTTPS for streaming

### Performance Requirements
- Must support minimum 2+ simultaneous users
- Stable operation under concurrent access
- Efficient bandwidth utilization

## Deliverables

### 1. Project Report
Include the following sections:
- System architecture diagram
- Description of main functions and modules
- Implemented features and test results (with screenshots)
- Team member responsibilities and contribution percentages (total 100%)

### 2. Source Code
- Complete source code package
- Buildable makefile or project files
- Clear directory structure
- Documentation comments in code

## Development Guidelines

### Project Structure Suggestion
```
ott-streaming-server/
├── server/
│   ├── src/
│   │   ├── main.c (or .cpp)
│   │   ├── streaming.c
│   │   ├── auth.c
│   │   └── database.c
│   ├── include/
│   │   └── headers.h
│   └── Makefile
├── client/
│   ├── index.html
│   ├── css/
│   │   └── styles.css
│   ├── js/
│   │   └── player.js
│   └── assets/
├── videos/
│   └── (MP4 files)
├── thumbnails/
│   └── (generated thumbnails)
├── database/
│   └── schema.sql
└── README.md
```

### Key Features to Implement

1. **Authentication Module**
   - User login/logout
   - Session management
   - Password encryption

2. **Video Management Module**
   - Video file indexing
   - Metadata extraction
   - Thumbnail generation using FFmpeg

3. **Streaming Module**
   - HTTP server implementation
   - Range request handling
   - Bandwidth optimization

4. **Database Module**
   - User information
   - Video metadata
   - Watch history
   - Session data

5. **Client Interface**
   - Login page
   - Video gallery
   - Video player with controls
   - Resume/restart dialog

### Testing Checklist
- [ ] Single user video playback
- [ ] Multiple concurrent users (2+ simultaneous)
- [ ] Login/logout functionality
- [ ] Thumbnail generation for all videos
- [ ] Resume playback feature
- [ ] Custom time position start
- [ ] Network stability under load
- [ ] Database consistency

## Additional Notes
- Use open-source tools like FFmpeg for video processing
- Can be developed individually or as a team
- Focus on clean, maintainable code
- Consider security aspects (input validation, SQL injection prevention)
- Implement proper error logging

## Development Priorities
1. **Phase 1:** Basic server setup and video file handling
2. **Phase 2:** User authentication and database setup
3. **Phase 3:** Streaming implementation with range requests
4. **Phase 4:** Web interface development
5. **Phase 5:** Advanced features (resume, custom start position)
6. **Phase 6:** Performance optimization and testing

## Success Criteria
- Functional multi-user streaming server
- Stable concurrent access for 2+ users
- All required features implemented
- Clean, documented code
- Comprehensive test results in report
