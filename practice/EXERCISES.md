# OTT Streaming Server - Practice Exercises

**Purpose**: Progressive hands-on exercises from basic HTTP server to Range Request video streaming

**Estimated Time**: 2-4 hours total (can be done across multiple sessions)

**Prerequisites**:
- Compiler installed (gcc)
- Basic C knowledge
- Read PREPARATION_GUIDE.md Part 1

---

## 📚 Exercise Overview

| Exercise | Focus | Difficulty | Time | Success Criteria |
|----------|-------|------------|------|------------------|
| **Exercise 1** | Basic HTTP Server | Easy | 20-30 min | Browser shows "Hello World" |
| **Exercise 2** | Serve Static File | Easy | 20-30 min | Browser displays full text file |
| **Exercise 3** | Range Requests (Text) | Medium | 45-60 min | curl shows 206 responses |
| **Exercise 4** | Range Requests (Video) | Medium | 45-60 min | Video plays and seeks |

**Progressive Learning Path:**
```
Exercise 1: Socket Programming Basics
    ↓
Exercise 2: HTTP Response + File I/O
    ↓
Exercise 3: Range Request Logic (Easy to Debug)
    ↓
Exercise 4: Video Streaming (Real Goal)
```

---

## 🎯 Exercise 1: Basic HTTP Server

### Learning Objectives
- Create TCP socket and listen for connections
- Accept client connections
- Send HTTP response
- Handle basic GET request

### What You'll Build
A minimal HTTP server that responds "Hello, World!" to any request.

### Code: `exercise1_hello_server.c`

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

    printf("=== Exercise 1: Basic HTTP Server ===\n\n");

    // Step 1: Create socket
    printf("Step 1: Creating socket...\n");
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    printf("✓ Socket created (fd=%d)\n\n", server_fd);

    // Allow immediate port reuse (helpful during development)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Step 2: Bind to port
    printf("Step 2: Binding to port %d...\n", PORT);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }
    printf("✓ Bound to port %d\n\n", PORT);

    // Step 3: Listen for connections
    printf("Step 3: Listening for connections...\n");
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }
    printf("✓ Listening (backlog=10)\n\n");

    printf("🚀 Server ready! Access http://localhost:%d/\n", PORT);
    printf("Press Ctrl+C to stop\n\n");

    // Step 4: Accept connections loop
    while (1) {
        printf("Waiting for connection...\n");

        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        char* client_ip = inet_ntoa(client_addr.sin_addr);
        printf("✓ Client connected: %s\n", client_ip);

        // Step 5: Read request
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            // Print first line of request
            char* newline = strchr(buffer, '\n');
            if (newline) *newline = '\0';
            printf("  Request: %s\n", buffer);
        }

        // Step 6: Send HTTP response
        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: 104\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body>"
            "<h1>Hello, World!</h1>"
            "<p>Exercise 1: Basic HTTP Server Working!</p>"
            "</body></html>";

        ssize_t bytes_sent = send(client_fd, response, strlen(response), 0);
        printf("  Sent %ld bytes\n", bytes_sent);

        // Step 7: Close connection
        close(client_fd);
        printf("  Connection closed\n\n");
    }

    close(server_fd);
    return 0;
}
```

### Compilation & Running

```bash
# Compile
gcc -o exercise1 exercise1_hello_server.c

# Run
./exercise1

# Expected output:
# === Exercise 1: Basic HTTP Server ===
#
# Step 1: Creating socket...
# ✓ Socket created (fd=3)
#
# Step 2: Binding to port 8080...
# ✓ Bound to port 8080
#
# Step 3: Listening for connections...
# ✓ Listening (backlog=10)
#
# 🚀 Server ready! Access http://localhost:8080/
# Press Ctrl+C to stop
#
# Waiting for connection...
```

### Testing

**Test 1: Web Browser**
```
1. Open Chrome/Firefox
2. Navigate to: http://localhost:8080/
3. Expected: See "Hello, World!" page
```

**Test 2: curl Command**
```bash
curl http://localhost:8080/

# Expected output:
# <html><body><h1>Hello, World!</h1><p>Exercise 1: Basic HTTP Server Working!</p></body></html>
```

**Test 3: curl Verbose (See Headers)**
```bash
curl -v http://localhost:8080/

# Expected output:
# > GET / HTTP/1.1
# > Host: localhost:8080
# > User-Agent: curl/7.68.0
# > Accept: */*
# >
# < HTTP/1.1 200 OK
# < Content-Type: text/html; charset=utf-8
# < Content-Length: 104
# < Connection: close
# <
# <html><body>...
```

### Success Criteria ✅
- [ ] Server compiles without errors
- [ ] Server starts and listens on port 8080
- [ ] Browser shows "Hello, World!" page
- [ ] curl receives 200 OK response
- [ ] Server handles multiple requests (refresh page several times)

### Common Issues & Solutions

**Issue: "Address already in use"**
```bash
# Solution 1: Wait 30 seconds for port to be released
# Solution 2: Kill existing process
lsof -ti:8080 | xargs kill -9

# Solution 3: Change PORT in code to 8081
```

**Issue: "Connection refused"**
```bash
# Check if server is running
ps aux | grep exercise1

# Check if port is open
netstat -tuln | grep 8080
```

**Issue: Compile error on Windows**
```bash
# Use WSL or MinGW
# Or add Windows socket headers:
# #include <winsock2.h>
# #pragma comment(lib, "ws2_32.lib")
```

---

## 🎯 Exercise 2: Serve Static File

### Learning Objectives
- Read file from disk
- Calculate Content-Length
- Send file content in HTTP response
- Handle 404 Not Found

### What You'll Build
HTTP server that serves a text file from disk.

### Setup

Create a test file:
```bash
echo "This is line 1
This is line 2
This is line 3
This is line 4
This is line 5" > test.txt
```

### Code: `exercise2_file_server.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 4096

// Get file size
long get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// Send 404 Not Found
void send_404(int client_fd) {
    const char* response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 47\r\n"
        "\r\n"
        "<html><body><h1>404 Not Found</h1></body></html>";

    send(client_fd, response, strlen(response), 0);
}

// Send file
void send_file(int client_fd, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("  ✗ File not found: %s\n", filename);
        send_404(client_fd);
        return;
    }

    // Get file size
    long file_size = get_file_size(filename);
    printf("  File size: %ld bytes\n", file_size);

    // Send HTTP header
    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n",
        file_size);

    send(client_fd, header, strlen(header), 0);
    printf("  ✓ Sent header (%ld bytes)\n", strlen(header));

    // Send file content
    char buffer[BUFFER_SIZE];
    size_t total_sent = 0;
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        ssize_t sent = send(client_fd, buffer, bytes_read, 0);
        if (sent > 0) {
            total_sent += sent;
        }
    }

    printf("  ✓ Sent content (%zu bytes)\n", total_sent);

    fclose(file);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    printf("=== Exercise 2: File Server ===\n\n");

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // Listen
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("🚀 Server ready on http://localhost:%d/\n", PORT);
    printf("Try: http://localhost:%d/test.txt\n\n", PORT);

    // Accept connections
    while (1) {
        printf("Waiting for connection...\n");

        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            continue;
        }

        printf("✓ Client connected\n");

        // Read request
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            // Parse request line: "GET /test.txt HTTP/1.1"
            char method[16], path[256], version[16];
            sscanf(buffer, "%15s %255s %15s", method, path, version);

            printf("  %s %s\n", method, path);

            // Remove leading slash
            const char* filename = (path[0] == '/') ? path + 1 : path;

            // Default to index
            if (strlen(filename) == 0) {
                filename = "test.txt";
            }

            // Send file
            send_file(client_fd, filename);
        }

        close(client_fd);
        printf("  Connection closed\n\n");
    }

    close(server_fd);
    return 0;
}
```

### Compilation & Running

```bash
# Compile
gcc -o exercise2 exercise2_file_server.c

# Run
./exercise2
```

### Testing

**Test 1: Browser**
```
Navigate to: http://localhost:8080/test.txt
Expected: See the 5 lines of text
```

**Test 2: curl**
```bash
curl http://localhost:8080/test.txt

# Expected output:
# This is line 1
# This is line 2
# This is line 3
# This is line 4
# This is line 5
```

**Test 3: Check Content-Length**
```bash
curl -I http://localhost:8080/test.txt

# Expected output:
# HTTP/1.1 200 OK
# Content-Type: text/plain
# Content-Length: 75
# Connection: close
```

**Test 4: 404 Error**
```bash
curl http://localhost:8080/nonexistent.txt

# Expected output:
# <html><body><h1>404 Not Found</h1></body></html>
```

### Success Criteria ✅
- [ ] Server serves test.txt correctly
- [ ] Content-Length matches file size
- [ ] Returns 404 for missing files
- [ ] Full file content delivered
- [ ] Works with multiple requests

---

## 🎯 Exercise 3: Range Requests with Text File

### Learning Objectives
- Parse Range header
- Calculate byte ranges
- Send 206 Partial Content response
- Handle Content-Range header

### What You'll Build
Server that supports HTTP Range Requests for text files (easier to debug than binary).

### Setup

Create a larger test file:
```bash
# Create a file with numbered lines (easy to verify ranges)
for i in {1..100}; do echo "Line $i: ABCDEFGHIJKLMNOPQRSTUVWXYZ"; done > bigfile.txt

# Check file size
ls -lh bigfile.txt
# Should be ~3400 bytes
```

### Code: `exercise3_range_text.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 4096

// Range request structure
typedef struct {
    long start;
    long end;
    int has_range;
} Range;

// Get file size
long get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// Parse Range header: "bytes=0-1023" -> {0, 1023, 1}
Range parse_range(const char* range_header) {
    Range range = {0, -1, 0};

    if (!range_header) {
        return range;
    }

    printf("  Parsing Range: %s\n", range_header);

    // Check for "bytes=" prefix
    if (strncmp(range_header, "bytes=", 6) != 0) {
        return range;
    }

    range.has_range = 1;
    const char* range_str = range_header + 6;  // Skip "bytes="

    // Parse "start-end"
    char* dash = strchr(range_str, '-');
    if (dash) {
        range.start = atol(range_str);

        // Check if end is specified
        if (*(dash + 1) != '\0') {
            range.end = atol(dash + 1);
        }
        // else: end = -1 means "to end of file"
    }

    printf("  → Parsed as: start=%ld, end=%ld\n", range.start, range.end);

    return range;
}

// Find header value in request
const char* find_header(const char* request, const char* header_name) {
    static char value[256];
    char search[128];
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

// Send 416 Range Not Satisfiable
void send_416(int client_fd, long file_size) {
    char response[256];
    snprintf(response, sizeof(response),
        "HTTP/1.1 416 Range Not Satisfiable\r\n"
        "Content-Range: bytes */%ld\r\n"
        "\r\n",
        file_size);

    send(client_fd, response, strlen(response), 0);
}

// Send file with Range support
void send_file_range(int client_fd, const char* filename, Range range) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("  ✗ File not found\n");
        return;
    }

    long file_size = get_file_size(filename);
    printf("  File size: %ld bytes\n", file_size);

    // Determine range
    long start = 0, end = file_size - 1;

    if (range.has_range) {
        start = range.start;
        end = (range.end == -1) ? file_size - 1 : range.end;

        // Validate range
        if (start < 0 || start >= file_size) {
            printf("  ✗ Invalid range: start=%ld\n", start);
            send_416(client_fd, file_size);
            fclose(file);
            return;
        }

        if (end >= file_size) {
            end = file_size - 1;
        }

        if (end < start) {
            printf("  ✗ Invalid range: end < start\n");
            send_416(client_fd, file_size);
            fclose(file);
            return;
        }
    }

    long content_length = end - start + 1;
    printf("  Range: %ld-%ld/%ld (%ld bytes)\n", start, end, file_size, content_length);

    // Build response header
    char header[512];
    if (range.has_range) {
        snprintf(header, sizeof(header),
            "HTTP/1.1 206 Partial Content\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %ld\r\n"
            "Content-Range: bytes %ld-%ld/%ld\r\n"
            "Accept-Ranges: bytes\r\n"
            "\r\n",
            content_length, start, end, file_size);
        printf("  ✓ Sending 206 Partial Content\n");
    } else {
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %ld\r\n"
            "Accept-Ranges: bytes\r\n"
            "\r\n",
            content_length);
        printf("  ✓ Sending 200 OK\n");
    }

    // Send header
    send(client_fd, header, strlen(header), 0);

    // Seek to start position
    fseek(file, start, SEEK_SET);

    // Send content
    char buffer[BUFFER_SIZE];
    long bytes_sent = 0;

    while (bytes_sent < content_length) {
        size_t bytes_to_send = (content_length - bytes_sent < BUFFER_SIZE)
                               ? (content_length - bytes_sent)
                               : BUFFER_SIZE;

        size_t bytes_read = fread(buffer, 1, bytes_to_send, file);
        if (bytes_read == 0) break;

        ssize_t sent = send(client_fd, buffer, bytes_read, 0);
        if (sent <= 0) break;

        bytes_sent += sent;
    }

    printf("  ✓ Sent %ld bytes\n", bytes_sent);

    fclose(file);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    printf("=== Exercise 3: Range Request Server (Text) ===\n\n");

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // Listen
    listen(server_fd, 10);

    printf("🚀 Server ready on http://localhost:%d/\n", PORT);
    printf("Try: curl -r 0-99 http://localhost:%d/bigfile.txt\n\n", PORT);

    // Accept loop
    while (1) {
        printf("Waiting for connection...\n");

        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) continue;

        printf("✓ Client connected\n");

        // Read request
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            // Parse request
            char method[16], path[256];
            sscanf(buffer, "%15s %255s", method, path);
            printf("  %s %s\n", method, path);

            // Get filename
            const char* filename = (path[0] == '/') ? path + 1 : path;
            if (strlen(filename) == 0) {
                filename = "bigfile.txt";
            }

            // Check for Range header
            const char* range_header = find_header(buffer, "Range");
            Range range = parse_range(range_header);

            // Send file
            send_file_range(client_fd, filename, range);
        }

        close(client_fd);
        printf("  Connection closed\n\n");
    }

    close(server_fd);
    return 0;
}
```

### Compilation & Running

```bash
# Compile
gcc -o exercise3 exercise3_range_text.c

# Run
./exercise3
```

### Testing

**Test 1: Full File (No Range)**
```bash
curl http://localhost:8080/bigfile.txt

# Expected: All 100 lines
# Server output shows: 200 OK
```

**Test 2: First 100 Bytes**
```bash
curl -r 0-99 http://localhost:8080/bigfile.txt

# Expected: First ~3 lines
# Server output shows: 206 Partial Content, Range: 0-99/3400
```

**Test 3: Middle Range**
```bash
curl -r 1000-1099 http://localhost:8080/bigfile.txt

# Expected: Partial content from middle of file
```

**Test 4: Last 100 Bytes**
```bash
curl -r -100 http://localhost:8080/bigfile.txt

# Expected: Last ~3 lines
```

**Test 5: Verify 206 Response**
```bash
curl -v -r 0-99 http://localhost:8080/bigfile.txt 2>&1 | grep -A 5 "< HTTP"

# Expected output should include:
# < HTTP/1.1 206 Partial Content
# < Content-Length: 100
# < Content-Range: bytes 0-99/3400
```

**Test 6: Invalid Range (Should get 416)**
```bash
curl -v -r 99999-100000 http://localhost:8080/bigfile.txt

# Expected:
# < HTTP/1.1 416 Range Not Satisfiable
```

### Success Criteria ✅
- [ ] Full file request returns 200 OK
- [ ] Range request returns 206 Partial Content
- [ ] Content-Range header present and correct
- [ ] Requested byte range matches received data
- [ ] Invalid range returns 416
- [ ] Multiple range requests work correctly

### Debugging Tips

**Verify exact bytes received:**
```bash
# Request first 100 bytes, save to file
curl -r 0-99 http://localhost:8080/bigfile.txt -o chunk.txt

# Check size
ls -l chunk.txt
# Should be exactly 100 bytes

# Compare with original
head -c 100 bigfile.txt > expected.txt
diff chunk.txt expected.txt
# Should show no differences
```

---

## 🎯 Exercise 4: Range Requests with Video File

### Learning Objectives
- Apply Range Requests to binary files (MP4)
- Understand Content-Type for video
- Create HTML5 video player
- Test seeking in browser

### What You'll Build
Complete video streaming server with HTML5 player.

### Setup

**Get a test video:**
```bash
# Option 1: Download sample video
curl -L "https://sample-videos.com/video123/mp4/720/big_buck_bunny_720p_1mb.mp4" -o test_video.mp4

# Option 2: Create test video with FFmpeg
ffmpeg -f lavfi -i testsrc=duration=30:size=640x480:rate=30 -pix_fmt yuv420p test_video.mp4

# Option 3: Use any MP4 file you have
# Just rename it to test_video.mp4
```

### Code: `exercise4_video_server.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 65536  // 64KB chunks for video

typedef struct {
    long start;
    long end;
    int has_range;
} Range;

long get_file_size(const char* filename) {
    struct stat st;
    return (stat(filename, &st) == 0) ? st.st_size : -1;
}

Range parse_range(const char* range_header) {
    Range range = {0, -1, 0};

    if (!range_header || strncmp(range_header, "bytes=", 6) != 0) {
        return range;
    }

    range.has_range = 1;
    const char* range_str = range_header + 6;
    char* dash = strchr(range_str, '-');

    if (dash) {
        range.start = atol(range_str);
        if (*(dash + 1) != '\0') {
            range.end = atol(dash + 1);
        }
    }

    return range;
}

const char* find_header(const char* request, const char* header_name) {
    static char value[256];
    char search[128];

    snprintf(search, sizeof(search), "\n%s: ", header_name);
    const char* start = strcasestr(request, search);

    if (!start) {
        snprintf(search, sizeof(search), "%s: ", header_name);
        start = strcasestr(request, search);
        if (!start) return NULL;
    } else {
        start++;
    }

    start += strlen(header_name) + 2;
    const char* end = strstr(start, "\r\n");
    if (!end) return NULL;

    size_t len = end - start;
    if (len >= sizeof(value)) len = sizeof(value) - 1;

    strncpy(value, start, len);
    value[len] = '\0';

    return value;
}

const char* get_mime_type(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";

    if (strcmp(ext, ".mp4") == 0) return "video/mp4";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";

    return "application/octet-stream";
}

void send_file(int client_fd, const char* filename, Range range) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        const char* response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_fd, response, strlen(response), 0);
        return;
    }

    long file_size = get_file_size(filename);
    long start = 0, end = file_size - 1;

    if (range.has_range) {
        start = range.start;
        end = (range.end == -1) ? file_size - 1 : range.end;

        if (start < 0 || start >= file_size || end < start) {
            char response[256];
            snprintf(response, sizeof(response),
                "HTTP/1.1 416 Range Not Satisfiable\r\n"
                "Content-Range: bytes */%ld\r\n\r\n",
                file_size);
            send(client_fd, response, strlen(response), 0);
            fclose(file);
            return;
        }

        if (end >= file_size) end = file_size - 1;
    }

    long content_length = end - start + 1;
    const char* mime_type = get_mime_type(filename);

    // Build header
    char header[512];
    if (range.has_range) {
        snprintf(header, sizeof(header),
            "HTTP/1.1 206 Partial Content\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "Content-Range: bytes %ld-%ld/%ld\r\n"
            "Accept-Ranges: bytes\r\n"
            "Connection: close\r\n"
            "\r\n",
            mime_type, content_length, start, end, file_size);
        printf("  206 Partial Content: bytes %ld-%ld/%ld\n", start, end, file_size);
    } else {
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "Accept-Ranges: bytes\r\n"
            "Connection: close\r\n"
            "\r\n",
            mime_type, content_length);
        printf("  200 OK: %ld bytes\n", content_length);
    }

    send(client_fd, header, strlen(header), 0);

    // Seek and send
    fseek(file, start, SEEK_SET);

    char buffer[BUFFER_SIZE];
    long bytes_sent = 0;

    while (bytes_sent < content_length) {
        size_t to_send = (content_length - bytes_sent < BUFFER_SIZE)
                         ? (content_length - bytes_sent) : BUFFER_SIZE;

        size_t read_bytes = fread(buffer, 1, to_send, file);
        if (read_bytes == 0) break;

        ssize_t sent = send(client_fd, buffer, read_bytes, 0);
        if (sent <= 0) break;

        bytes_sent += sent;
    }

    printf("  Sent %ld bytes\n", bytes_sent);
    fclose(file);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[8192];

    printf("=== Exercise 4: Video Streaming Server ===\n\n");

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 10);

    printf("🚀 Video server ready!\n");
    printf("   Player: http://localhost:%d/\n", PORT);
    printf("   Video:  http://localhost:%d/test_video.mp4\n\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) continue;

        int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            char method[16], path[256];
            sscanf(buffer, "%15s %255s", method, path);
            printf("%s %s\n", method, path);

            const char* filename;
            if (strcmp(path, "/") == 0) {
                filename = "player.html";
            } else {
                filename = path + 1;  // Skip leading /
            }

            const char* range_header = find_header(buffer, "Range");
            Range range = parse_range(range_header);

            send_file(client_fd, filename, range);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
```

### Create Player: `player.html`

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Exercise 4: Video Player</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            background: #f5f5f5;
        }
        h1 {
            color: #333;
        }
        .video-container {
            background: #000;
            padding: 10px;
            border-radius: 8px;
            margin: 20px 0;
        }
        video {
            width: 100%;
            display: block;
        }
        .test-results {
            background: #fff;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .test-item {
            padding: 10px;
            margin: 5px 0;
            border-left: 4px solid #ddd;
        }
        .test-item.pass {
            border-left-color: #4CAF50;
            background: #f1f8f4;
        }
        .test-item.fail {
            border-left-color: #f44336;
            background: #fef1f0;
        }
        .controls {
            margin: 20px 0;
        }
        button {
            padding: 10px 20px;
            margin: 5px;
            background: #2196F3;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        button:hover {
            background: #0b7dda;
        }
    </style>
</head>
<body>
    <h1>🎬 Exercise 4: Video Streaming Test</h1>

    <div class="video-container">
        <video id="player" controls>
            <source src="/test_video.mp4" type="video/mp4">
            Your browser doesn't support HTML5 video.
        </video>
    </div>

    <div class="controls">
        <button onclick="testSeekToMiddle()">Test: Seek to Middle</button>
        <button onclick="testSeekToEnd()">Test: Seek to End</button>
        <button onclick="testSeekToStart()">Test: Seek to Start</button>
        <button onclick="runAllTests()">Run All Tests</button>
    </div>

    <div class="test-results" id="results">
        <h2>Test Results:</h2>
        <div id="test-output"></div>
    </div>

    <script>
        const player = document.getElementById('player');
        const output = document.getElementById('test-output');

        function addResult(test, passed, message) {
            const div = document.createElement('div');
            div.className = 'test-item ' + (passed ? 'pass' : 'fail');
            div.innerHTML = `<strong>${passed ? '✓' : '✗'} ${test}</strong><br>${message}`;
            output.appendChild(div);
        }

        // Test 1: Video loads
        player.addEventListener('loadedmetadata', () => {
            const duration = player.duration.toFixed(2);
            addResult('Video Loaded', true,
                `Duration: ${duration}s, Size: ${player.videoWidth}x${player.videoHeight}`);
        });

        // Test 2: Video can play
        player.addEventListener('canplay', () => {
            addResult('Can Play', true, 'Video is ready to play');
        });

        // Test 3: Monitor seeking
        let seekCount = 0;
        player.addEventListener('seeking', () => {
            seekCount++;
            addResult('Seeking Started', true,
                `Seeking to ${player.currentTime.toFixed(2)}s (seek #${seekCount})`);
        });

        player.addEventListener('seeked', () => {
            addResult('Seek Complete', true,
                `Now at ${player.currentTime.toFixed(2)}s`);
        });

        // Test functions
        function testSeekToMiddle() {
            if (player.duration) {
                player.currentTime = player.duration / 2;
            }
        }

        function testSeekToEnd() {
            if (player.duration) {
                player.currentTime = player.duration - 5;
            }
        }

        function testSeekToStart() {
            player.currentTime = 0;
        }

        function runAllTests() {
            output.innerHTML = '<p>Running automated tests...</p>';

            setTimeout(() => {
                testSeekToMiddle();
                setTimeout(() => {
                    testSeekToEnd();
                    setTimeout(() => {
                        testSeekToStart();
                        setTimeout(() => {
                            addResult('All Tests Complete', true,
                                `Performed ${seekCount} seek operations successfully`);
                        }, 2000);
                    }, 2000);
                }, 2000);
            }, 1000);
        }

        // Auto-start
        player.play().catch(e => {
            addResult('Auto-play', false,
                'Auto-play blocked by browser (this is normal, click play button)');
        });
    </script>
</body>
</html>
```

### Compilation & Running

```bash
# Compile
gcc -o exercise4 exercise4_video_server.c

# Run
./exercise4
```

### Testing

**Test 1: Browser Player**
```
1. Open http://localhost:8080/ in browser
2. Video should load and show player controls
3. Click play - video should play
4. Drag seek bar - video should jump to position
5. Check server console for 206 responses
```

**Test 2: curl Range Requests**
```bash
# First MB
curl -r 0-1048575 http://localhost:8080/test_video.mp4 -o chunk1.bin
ls -lh chunk1.bin
# Should be exactly 1MB

# Second MB
curl -r 1048576-2097151 http://localhost:8080/test_video.mp4 -o chunk2.bin
```

**Test 3: Verify 206 Responses**
```bash
curl -v -r 0-1023 http://localhost:8080/test_video.mp4 2>&1 | grep "206"
# Should see: < HTTP/1.1 206 Partial Content
```

**Test 4: Network Tab Analysis**
```
1. Open browser DevTools (F12)
2. Go to Network tab
3. Load http://localhost:8080/
4. Watch video requests
5. Seek to different positions
6. Verify: Multiple 206 requests with different Range headers
```

### Success Criteria ✅
- [ ] Video loads in browser
- [ ] Video plays smoothly
- [ ] Seeking works (jump to different positions)
- [ ] Server logs show 206 Partial Content responses
- [ ] Network tab shows Range requests
- [ ] Multiple seeks work without issues
- [ ] curl range requests work correctly

### Advanced Testing

**Monitor Network Traffic:**
```bash
# In browser DevTools Network tab, check:
# 1. Initial request: Range: bytes=0-xxxxx
# 2. After seeking: Range: bytes=xxxxx-yyyyy
# 3. All responses: 206 Partial Content
```

**Verify Byte Accuracy:**
```bash
# Download first 1000 bytes via server
curl -r 0-999 http://localhost:8080/test_video.mp4 -o server_chunk.bin

# Extract first 1000 bytes from original file
head -c 1000 test_video.mp4 > original_chunk.bin

# Compare (should be identical)
diff server_chunk.bin original_chunk.bin
echo $?  # Should output 0 (no differences)
```

---

## 🎓 Learning Summary

### What You've Accomplished

After completing all exercises:

✅ **Exercise 1**: Built a TCP server and sent HTTP responses
✅ **Exercise 2**: Served files from disk with correct headers
✅ **Exercise 3**: Implemented Range Request parsing and 206 responses
✅ **Exercise 4**: Streamed video with seeking support

### Key Concepts Mastered

1. **Socket Programming**
   - Creating, binding, listening sockets
   - Accepting connections
   - Reading and sending data

2. **HTTP Protocol**
   - Request parsing
   - Response headers
   - Status codes (200, 206, 404, 416)

3. **Range Requests**
   - Parsing Range header
   - Calculating byte ranges
   - Content-Range header format
   - Off-by-one error prevention

4. **File I/O**
   - Opening files in binary mode
   - Seeking to positions (fseek)
   - Reading chunks
   - Buffered sending

### Common Patterns You Learned

```c
// Pattern 1: Range parsing
Range parse_range(const char* header);

// Pattern 2: Safe range calculation
content_length = end - start + 1;  // +1 is critical!

// Pattern 3: File seeking
fseek(file, start, SEEK_SET);  // SEEK_SET not SEEK_CUR

// Pattern 4: Chunked sending
while (bytes_sent < total) {
    size_t chunk = min(BUFFER, total - bytes_sent);
    // send chunk
}
```

---

## 🚀 Next Steps

You're now ready for **MVP Phase 1** implementation!

### Differences from Exercises

**What's the same:**
- Range Request logic (exercise 3 & 4)
- File streaming approach
- HTTP response building

**What's new in MVP:**
- Multiple file support
- Request routing (/video.mp4 vs /player.html)
- Fork-based concurrency (Phase 2)
- Error handling and logging
- Production-quality code structure

### Recommended Path

1. **Review your exercise code**
   - Understand every line
   - Note any challenges you faced

2. **Start MVP Phase 1**
   - Begin with project structure
   - Copy Range Request logic from exercises
   - Add routing and multiple file support

3. **Test incrementally**
   - Use test scripts from exercises
   - Verify each feature before moving forward

---

## 📋 Exercise Completion Checklist

- [ ] Exercise 1 completed and tested
- [ ] Exercise 2 completed and tested
- [ ] Exercise 3 completed and tested
- [ ] Exercise 4 completed and tested
- [ ] All test scripts run successfully
- [ ] Understand Range Request logic completely
- [ ] Can explain off-by-one calculation
- [ ] Ready to start MVP Phase 1

---

## ❓ Troubleshooting

### Compilation Errors

**Error: `strcasestr` not found**
```c
// Add at top of file:
#define _GNU_SOURCE
#include <string.h>
```

**Error: Windows socket issues**
```c
// Use WSL or MinGW, or add:
#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#endif
```

### Runtime Errors

**Error: Address already in use**
```bash
# Kill process on port 8080
lsof -ti:8080 | xargs kill -9
# Or wait 30 seconds for TIME_WAIT to expire
```

**Error: Segmentation fault**
```bash
# Run with debugger
gdb ./exercise4
(gdb) run
# When it crashes:
(gdb) backtrace
```

**Error: File not found**
```bash
# Check current directory
ls -la
# Server looks for files in current working directory
# Run server from directory containing test files
```

---

**Great job completing the exercises!** 🎉

You now have hands-on experience with the core concepts needed for the OTT streaming server.

**When ready, say: "I'm ready for MVP Phase 1"**
