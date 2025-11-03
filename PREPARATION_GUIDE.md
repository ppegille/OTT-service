# OTT Streaming Server - Preparation Guide

**Purpose**: Comprehensive preparation before implementation - HTTP concepts, environment setup, C fundamentals, and testing strategy.

**Focus Area**: HTTP Range Requests for video streaming (your primary concern)

---

## 📚 Part 1: HTTP Range Requests - Deep Dive

### What Problem Does This Solve?

**Without Range Requests:**
- Must download entire 1GB video before playing
- Cannot seek/scrub to different positions
- Wastes bandwidth on content user may not watch
- Poor user experience (long wait times)

**With Range Requests:**
- Start playing immediately with first few MB
- Seek to any position instantly
- Browser requests only needed portions
- Efficient bandwidth usage

---

### HTTP Range Request Specification (RFC 7233)

#### Basic Request-Response Flow

**1. Client Initial Request:**
```http
GET /video.mp4 HTTP/1.1
Host: localhost:8080
Range: bytes=0-1048575
```

**Translation:** "Give me the first 1MB (bytes 0 through 1,048,575) of video.mp4"

**2. Server Response:**
```http
HTTP/1.1 206 Partial Content
Content-Type: video/mp4
Content-Length: 1048576
Content-Range: bytes 0-1048575/104857600
Accept-Ranges: bytes

[binary video data - exactly 1,048,576 bytes]
```

**Translation:**
- `206 Partial Content` = "I'm sending a portion, not the whole file"
- `Content-Range: bytes 0-1048575/104857600` = "This is bytes 0-1048575 out of total 104857600 bytes"
- `Accept-Ranges: bytes` = "I support byte-range requests for this resource"

---

### Range Header Formats

#### Format 1: First N bytes
```http
Range: bytes=0-1023
```
**Meaning:** Bytes 0 through 1023 (first 1024 bytes)

#### Format 2: From position to end
```http
Range: bytes=1000000-
```
**Meaning:** All bytes starting from byte 1,000,000 to end of file

#### Format 3: Last N bytes
```http
Range: bytes=-1024
```
**Meaning:** Last 1024 bytes of file

#### Format 4: Multiple ranges (optional for MVP)
```http
Range: bytes=0-1023, 2048-3071
```
**Meaning:** Two separate ranges (we'll skip this for MVP)

---

### Implementation Algorithm

**Step-by-Step Server Logic:**

```
1. Parse HTTP request → extract "Range" header value
   Example: "bytes=1000-2999" → start=1000, end=2999

2. Open video file → get total file size
   Example: stat("video.mp4") → size=104857600

3. Validate range:
   - If start > file_size → send 416 Range Not Satisfiable
   - If end == -1 (missing) → end = file_size - 1
   - If end > file_size - 1 → end = file_size - 1

4. Calculate content length:
   content_length = end - start + 1
   Example: 2999 - 1000 + 1 = 2000 bytes

5. Seek to start position:
   fseek(file, start, SEEK_SET)

6. Build HTTP response header:
   HTTP/1.1 206 Partial Content\r\n
   Content-Type: video/mp4\r\n
   Content-Length: 2000\r\n
   Content-Range: bytes 1000-2999/104857600\r\n
   Accept-Ranges: bytes\r\n
   \r\n

7. Send response header

8. Send file data in chunks:
   while (bytes_sent < content_length) {
       bytes_to_send = min(BUFFER_SIZE, content_length - bytes_sent)
       fread(buffer, 1, bytes_to_send, file)
       send(socket, buffer, bytes_to_send, 0)
       bytes_sent += bytes_to_send
   }

9. Close file, close socket
```

---

### C Code Skeleton

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 65536  // 64KB chunks

typedef struct {
    long start;
    long end;
    int has_range;
} RangeRequest;

// Parse "Range: bytes=1000-2999" header
RangeRequest parse_range_header(const char* range_header) {
    RangeRequest range = {0, -1, 0};

    if (!range_header || strncmp(range_header, "bytes=", 6) != 0) {
        return range;  // No range header
    }

    range.has_range = 1;
    const char* range_str = range_header + 6;  // Skip "bytes="

    // Parse "1000-2999" format
    char* dash = strchr(range_str, '-');
    if (dash) {
        range.start = atol(range_str);
        if (*(dash + 1) != '\0') {
            range.end = atol(dash + 1);
        }
    }

    return range;
}

// Stream video file with Range Request support
void stream_video(int client_socket, const char* filepath, RangeRequest range) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        // Send 404 Not Found
        const char* response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        return;
    }

    // Get file size
    struct stat file_stat;
    stat(filepath, &file_stat);
    long file_size = file_stat.st_size;

    // Determine range
    long start = 0, end = file_size - 1;
    if (range.has_range) {
        start = range.start;
        end = (range.end == -1) ? file_size - 1 : range.end;

        // Validate range
        if (start > file_size - 1 || start < 0) {
            const char* response = "HTTP/1.1 416 Range Not Satisfiable\r\n\r\n";
            send(client_socket, response, strlen(response), 0);
            fclose(file);
            return;
        }

        if (end > file_size - 1) {
            end = file_size - 1;
        }
    }

    long content_length = end - start + 1;

    // Build response header
    char header[512];
    if (range.has_range) {
        snprintf(header, sizeof(header),
            "HTTP/1.1 206 Partial Content\r\n"
            "Content-Type: video/mp4\r\n"
            "Content-Length: %ld\r\n"
            "Content-Range: bytes %ld-%ld/%ld\r\n"
            "Accept-Ranges: bytes\r\n"
            "\r\n",
            content_length, start, end, file_size);
    } else {
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: video/mp4\r\n"
            "Content-Length: %ld\r\n"
            "Accept-Ranges: bytes\r\n"
            "\r\n",
            content_length);
    }

    // Send header
    send(client_socket, header, strlen(header), 0);

    // Seek to start position
    fseek(file, start, SEEK_SET);

    // Send file data in chunks
    char buffer[BUFFER_SIZE];
    long bytes_sent = 0;

    while (bytes_sent < content_length) {
        size_t bytes_to_send = (content_length - bytes_sent < BUFFER_SIZE)
                               ? (content_length - bytes_sent)
                               : BUFFER_SIZE;

        size_t bytes_read = fread(buffer, 1, bytes_to_send, file);
        if (bytes_read <= 0) break;

        ssize_t sent = send(client_socket, buffer, bytes_read, 0);
        if (sent <= 0) break;

        bytes_sent += sent;
    }

    fclose(file);
}
```

---

### Testing Range Requests

#### Method 1: Browser DevTools (Visual)

1. Open Chrome DevTools (F12)
2. Go to Network tab
3. Load your video player page
4. Watch the network requests

**What to look for:**
```
Request #1: Range: bytes=0-1048575
Response: 206 Partial Content, Content-Range: bytes 0-1048575/104857600

[User seeks to 5:00 mark]

Request #2: Range: bytes=20000000-21048575
Response: 206 Partial Content, Content-Range: bytes 20000000-21048575/104857600
```

**Success indicators:**
- Status code is 206 (not 200)
- Content-Range header present
- Only requested bytes transferred (check "Size" column)

#### Method 2: curl Command Line

```bash
# Test 1: Request first 1KB
curl -v -r 0-1023 http://localhost:8080/video.mp4 -o test_chunk.bin

# Expected output:
# < HTTP/1.1 206 Partial Content
# < Content-Range: bytes 0-1023/104857600
# < Content-Length: 1024

# Test 2: Request middle portion
curl -v -r 1000000-1001023 http://localhost:8080/video.mp4 -o test_chunk2.bin

# Test 3: Request from position to end
curl -v -r 50000000- http://localhost:8080/video.mp4 -o test_chunk3.bin
```

#### Method 3: Automated Test Script

```bash
#!/bin/bash
# test_range_requests.sh

SERVER="http://localhost:8080"
VIDEO="/video.mp4"

echo "Testing Range Request Support..."

# Test 1: First 1024 bytes
echo "Test 1: First 1KB"
RESPONSE=$(curl -s -I -r 0-1023 "${SERVER}${VIDEO}")
if echo "$RESPONSE" | grep -q "206 Partial Content"; then
    echo "✓ PASS: 206 response received"
else
    echo "✗ FAIL: Expected 206, got something else"
    exit 1
fi

# Test 2: Content-Range header present
echo "Test 2: Content-Range header"
if echo "$RESPONSE" | grep -q "Content-Range: bytes 0-1023"; then
    echo "✓ PASS: Content-Range header correct"
else
    echo "✗ FAIL: Content-Range header missing or incorrect"
    exit 1
fi

# Test 3: Middle range
echo "Test 3: Middle range (1000-1999)"
RESPONSE=$(curl -s -I -r 1000-1999 "${SERVER}${VIDEO}")
if echo "$RESPONSE" | grep -q "Content-Range: bytes 1000-1999"; then
    echo "✓ PASS: Middle range works"
else
    echo "✗ FAIL: Middle range incorrect"
    exit 1
fi

echo "All tests passed!"
```

---

### Common Pitfalls & Solutions

#### Pitfall 1: Off-by-One Errors

**Problem:**
```c
content_length = end - start;  // WRONG! Missing +1
```

**Why it's wrong:**
- Range "0-1023" means bytes 0, 1, 2, ..., 1023 = 1024 bytes
- Formula: end - start = 1023 - 0 = 1023 (wrong!)
- Correct: end - start + 1 = 1024

**Solution:**
```c
content_length = end - start + 1;  // CORRECT
```

#### Pitfall 2: Not Validating Range

**Problem:**
```c
// User requests: Range: bytes=999999999-
// File size: 1000000
// Server crashes or sends wrong data
```

**Solution:**
```c
if (start >= file_size || start < 0) {
    send_416_range_not_satisfiable(socket);
    return;
}

if (end >= file_size || end < start) {
    end = file_size - 1;
}
```

#### Pitfall 3: Forgetting Accept-Ranges Header

**Problem:**
- Browser doesn't know server supports ranges
- Always requests full file, even when seeking
- Defeats entire purpose

**Solution:**
```c
// Always include this header, even in 200 responses
"Accept-Ranges: bytes\r\n"
```

#### Pitfall 4: Incorrect fseek() Usage

**Problem:**
```c
fseek(file, start, SEEK_CUR);  // WRONG! Relative to current position
```

**Solution:**
```c
fseek(file, start, SEEK_SET);  // CORRECT! Absolute position from start
```

#### Pitfall 5: Buffer Overflow on Last Chunk

**Problem:**
```c
while (bytes_sent < content_length) {
    fread(buffer, 1, BUFFER_SIZE, file);  // May read too much on last iteration
    send(socket, buffer, BUFFER_SIZE, 0);  // Sends more than requested
    bytes_sent += BUFFER_SIZE;
}
```

**Solution:**
```c
while (bytes_sent < content_length) {
    size_t bytes_to_send = min(BUFFER_SIZE, content_length - bytes_sent);
    size_t bytes_read = fread(buffer, 1, bytes_to_send, file);
    send(socket, buffer, bytes_read, 0);
    bytes_sent += bytes_read;
}
```

---

## 🛠️ Part 2: Development Environment Setup

### Windows (Current OS) Setup

#### Option A: WSL2 (Recommended)

**Why WSL2?**
- Native Linux environment on Windows
- Better socket programming support
- Easier to follow Unix-based tutorials
- More similar to deployment environment

**Installation:**
```powershell
# In PowerShell (Administrator)
wsl --install

# Restart computer

# Install Ubuntu
wsl --install -d Ubuntu

# After restart, open Ubuntu terminal
# Update packages
sudo apt update && sudo apt upgrade -y

# Install development tools
sudo apt install build-essential gcc gdb make -y
sudo apt install sqlite3 libsqlite3-dev -y
sudo apt install ffmpeg -y

# Verify installations
gcc --version      # Should show GCC version
sqlite3 --version  # Should show SQLite version
ffmpeg -version    # Should show FFmpeg version
```

#### Option B: MinGW (Native Windows)

**Installation:**
```
1. Download MSYS2: https://www.msys2.org/
2. Install MSYS2
3. Open MSYS2 terminal

# Update package database
pacman -Syu

# Install toolchain
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-sqlite3
pacman -S mingw-w64-x86_64-ffmpeg

# Add to PATH:
C:\msys64\mingw64\bin
```

#### Option C: Docker (Containerized)

**Create Dockerfile:**
```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    gdb \
    make \
    sqlite3 \
    libsqlite3-dev \
    ffmpeg \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
EXPOSE 8080

CMD ["/bin/bash"]
```

**Build and run:**
```bash
docker build -t ott-dev .
docker run -it -v ${PWD}:/app -p 8080:8080 ott-dev
```

---

### Recommended: VS Code Setup

**Extensions:**
- C/C++ (Microsoft) - IntelliSense, debugging
- C/C++ Extension Pack
- Makefile Tools
- SQLite Viewer
- Rest Client (for API testing)

**Create `.vscode/launch.json` for debugging:**
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug OTT Server",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/server/ott_server",
            "args": ["8080"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/server",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```

---

## 📖 Part 3: C Programming Essentials

### Socket Programming Fundamentals

#### Minimal TCP Server Structure

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        exit(1);
    }

    // Allow port reuse (important for development)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Bind to port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    // 3. Listen for connections
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    // 4. Accept connections loop
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

        // 5. Read request
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Request:\n%s\n", buffer);

            // 6. Send response
            const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello!";
            send(client_fd, response, strlen(response), 0);
        }

        // 7. Close client connection
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
```

**Compile and run:**
```bash
gcc -o simple_server simple_server.c
./simple_server

# In another terminal:
curl http://localhost:8080/
# Should see: Hello!
```

---

### HTTP Request Parsing

```c
typedef struct {
    char method[16];      // GET, POST, etc.
    char path[256];       // /video.mp4
    char version[16];     // HTTP/1.1
} HTTPRequest;

HTTPRequest parse_http_request(const char* request) {
    HTTPRequest req = {0};

    // Parse first line: "GET /video.mp4 HTTP/1.1"
    sscanf(request, "%15s %255s %15s", req.method, req.path, req.version);

    return req;
}

// Find header value
const char* get_header(const char* request, const char* header_name) {
    static char value[256];
    char search[128];
    snprintf(search, sizeof(search), "%s: ", header_name);

    const char* header_start = strstr(request, search);
    if (!header_start) return NULL;

    header_start += strlen(search);
    const char* header_end = strstr(header_start, "\r\n");
    if (!header_end) return NULL;

    size_t len = header_end - header_start;
    if (len >= sizeof(value)) len = sizeof(value) - 1;

    strncpy(value, header_start, len);
    value[len] = '\0';

    return value;
}

// Usage:
const char* range = get_header(request, "Range");
if (range) {
    printf("Range header: %s\n", range);
}
```

---

### File I/O for Video Streaming

```c
#include <sys/stat.h>

// Get file size
long get_file_size(const char* filepath) {
    struct stat st;
    if (stat(filepath, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// Check if file exists
int file_exists(const char* filepath) {
    return access(filepath, F_OK) == 0;
}

// Safe file reading with seeking
size_t read_file_range(const char* filepath, long start, long length,
                       char* buffer, size_t buffer_size) {
    FILE* file = fopen(filepath, "rb");
    if (!file) return 0;

    // Seek to start position
    if (fseek(file, start, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }

    // Read up to length bytes
    size_t bytes_to_read = (length < buffer_size) ? length : buffer_size;
    size_t bytes_read = fread(buffer, 1, bytes_to_read, file);

    fclose(file);
    return bytes_read;
}
```

---

### Memory Management Best Practices

```c
// Always initialize pointers
char* buffer = NULL;

// Allocate
buffer = malloc(BUFFER_SIZE);
if (!buffer) {
    perror("malloc failed");
    return;
}

// Use
memset(buffer, 0, BUFFER_SIZE);  // Zero initialize

// Free
free(buffer);
buffer = NULL;  // Prevent double-free

// For strings
char* str = strdup("original");  // Allocates and copies
// ... use str ...
free(str);
```

**Common mistakes:**
```c
// WRONG: Returning pointer to local variable
char* get_string() {
    char buffer[256];
    strcpy(buffer, "data");
    return buffer;  // DANGLING POINTER!
}

// RIGHT: Allocate on heap
char* get_string() {
    char* buffer = malloc(256);
    strcpy(buffer, "data");
    return buffer;  // Caller must free()
}
```

---

## 🧪 Part 4: Testing Strategy

### Testing Pyramid

```
           /\
          /  \        E2E Tests (Few)
         /____\       - Full user workflows
        /      \      - Browser automation
       /________\     Integration Tests (Some)
      /          \    - API endpoint tests
     /____________\   - Multi-component tests
    /              \  Unit Tests (Many)
   /________________\ - Function-level tests
```

### Phase 1 Testing (MVP Streaming)

**Test 1: Server Starts**
```bash
#!/bin/bash
# test_server_start.sh

./ott_server 8080 &
SERVER_PID=$!
sleep 1

# Check if server is listening
if curl -s http://localhost:8080/ > /dev/null; then
    echo "✓ Server started successfully"
    kill $SERVER_PID
    exit 0
else
    echo "✗ Server failed to start"
    kill $SERVER_PID
    exit 1
fi
```

**Test 2: Range Request Handling**
```bash
#!/bin/bash
# test_range_requests.sh

# Test first 1KB
RESPONSE=$(curl -s -i -r 0-1023 http://localhost:8080/video.mp4)

# Check status code
if echo "$RESPONSE" | head -1 | grep -q "206"; then
    echo "✓ 206 response correct"
else
    echo "✗ Expected 206 Partial Content"
    exit 1
fi

# Check Content-Range header
if echo "$RESPONSE" | grep -q "Content-Range: bytes 0-1023"; then
    echo "✓ Content-Range header correct"
else
    echo "✗ Content-Range header missing or wrong"
    exit 1
fi

echo "All tests passed!"
```

**Test 3: Video Playback**
```html
<!-- test_player.html -->
<!DOCTYPE html>
<html>
<head>
    <title>Test Player</title>
</head>
<body>
    <h1>Video Streaming Test</h1>
    <video id="player" controls width="640" height="360">
        <source src="http://localhost:8080/video.mp4" type="video/mp4">
    </video>

    <div id="test-results"></div>

    <script>
        const player = document.getElementById('player');
        const results = document.getElementById('test-results');

        let tests = {
            canLoad: false,
            canPlay: false,
            canSeek: false
        };

        // Test 1: Video loads
        player.addEventListener('loadedmetadata', () => {
            tests.canLoad = true;
            results.innerHTML += '<p>✓ Video loaded</p>';
        });

        // Test 2: Video plays
        player.addEventListener('playing', () => {
            tests.canPlay = true;
            results.innerHTML += '<p>✓ Video playing</p>';

            // Test 3: Seeking after 2 seconds
            setTimeout(() => {
                player.currentTime = player.duration / 2;
            }, 2000);
        });

        // Test 3: Seeking works
        player.addEventListener('seeked', () => {
            tests.canSeek = true;
            results.innerHTML += '<p>✓ Seeking works</p>';

            // Summary
            setTimeout(() => {
                const allPassed = tests.canLoad && tests.canPlay && tests.canSeek;
                if (allPassed) {
                    results.innerHTML += '<h2 style="color: green;">✓ All tests passed!</h2>';
                } else {
                    results.innerHTML += '<h2 style="color: red;">✗ Some tests failed</h2>';
                }
            }, 1000);
        });

        // Auto-play for testing
        player.play();
    </script>
</body>
</html>
```

### Phase 2 Testing (Concurrent Users)

**Concurrent User Simulator:**
```python
#!/usr/bin/env python3
# test_concurrent_users.py

import requests
import threading
import time

SERVER = "http://localhost:8080"
VIDEO = "/video.mp4"
NUM_USERS = 3

def simulate_user(user_id):
    """Simulate a user streaming video"""
    print(f"User {user_id}: Starting stream")

    try:
        # Stream first 5MB
        headers = {'Range': 'bytes=0-5242879'}
        response = requests.get(f"{SERVER}{VIDEO}", headers=headers, stream=True)

        if response.status_code != 206:
            print(f"User {user_id}: FAIL - Expected 206, got {response.status_code}")
            return False

        bytes_received = 0
        for chunk in response.iter_content(chunk_size=8192):
            bytes_received += len(chunk)
            time.sleep(0.01)  # Simulate network delay

        print(f"User {user_id}: SUCCESS - Received {bytes_received} bytes")
        return True

    except Exception as e:
        print(f"User {user_id}: ERROR - {e}")
        return False

# Start concurrent users
threads = []
start_time = time.time()

for i in range(NUM_USERS):
    t = threading.Thread(target=simulate_user, args=(i+1,))
    t.start()
    threads.append(t)

# Wait for all to complete
for t in threads:
    t.join()

elapsed = time.time() - start_time
print(f"\nCompleted {NUM_USERS} concurrent users in {elapsed:.2f} seconds")
```

---

## 📋 Part 5: Pre-Implementation Checklist

### Knowledge Verification

- [ ] I understand what HTTP Range Requests solve
- [ ] I can explain the difference between 200 OK and 206 Partial Content
- [ ] I know how to parse "Range: bytes=start-end" header
- [ ] I understand file seeking with fseek()
- [ ] I can calculate content_length = end - start + 1

### Environment Verification

```bash
# Run these commands to verify setup

# GCC compiler
gcc --version
# Expected: gcc (Ubuntu/MinGW) X.X.X

# Make
make --version
# Expected: GNU Make X.X

# SQLite3
sqlite3 --version
# Expected: 3.X.X

# FFmpeg
ffmpeg -version
# Expected: ffmpeg version X.X.X

# Curl (for testing)
curl --version
# Expected: curl X.X.X

# Optional: Python (for test scripts)
python3 --version
# Expected: Python 3.X.X
```

### Project Setup Verification

- [ ] Created project directory: `ott-streaming-server/`
- [ ] Created subdirectories: `server/`, `client/`, `videos/`, `tests/`
- [ ] Have at least one test video file (MP4 format)
- [ ] Text editor/IDE configured with C support
- [ ] Terminal/command line comfortable

### Next Steps

When all checkboxes are complete, you're ready to:

1. Start MVP Phase 1 implementation
2. Create minimal HTTP server
3. Implement Range Request parsing
4. Test video streaming

---

## 🎓 Additional Learning Resources

### HTTP Range Requests
- RFC 7233 Official Spec: https://tools.ietf.org/html/rfc7233
- MDN Web Docs: https://developer.mozilla.org/en-US/docs/Web/HTTP/Range_requests

### C Socket Programming
- Beej's Guide to Network Programming: https://beej.us/guide/bgnet/
- Unix Socket Tutorial: https://www.tutorialspoint.com/unix_sockets/

### Video Streaming Concepts
- HTTP Streaming Basics: https://developer.mozilla.org/en-US/docs/Web/Guide/Audio_and_video_delivery
- HTML5 Video: https://developer.mozilla.org/en-US/docs/Web/HTML/Element/video

### Debugging Tools
- GDB Tutorial: https://www.gdbtutorial.com/
- Valgrind Guide: https://valgrind.org/docs/manual/quick-start.html
- Wireshark HTTP Analysis: https://www.wireshark.org/

---

**Next Document**: When ready to implement, we'll create `MVP_PHASE1_GUIDE.md` with step-by-step coding instructions.

**Questions?** Let me know if you need clarification on any concept before we start coding!
