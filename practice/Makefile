# Makefile for OTT Streaming Practice Exercises

CC = gcc
CFLAGS = -Wall -Wextra -g

# Targets
all: exercise1 exercise2 exercise3 exercise4

exercise1: exercise1_hello_server.c
	$(CC) $(CFLAGS) -o exercise1 exercise1_hello_server.c
	@echo "✓ Built exercise1 - Basic HTTP Server"

exercise2: exercise2_file_server.c
	$(CC) $(CFLAGS) -o exercise2 exercise2_file_server.c
	@echo "✓ Built exercise2 - File Server"

exercise3: exercise3_range_text.c
	$(CC) $(CFLAGS) -o exercise3 exercise3_range_text.c
	@echo "✓ Built exercise3 - Range Requests (Text)"

exercise4: exercise4_video_server.c
	$(CC) $(CFLAGS) -o exercise4 exercise4_video_server.c
	@echo "✓ Built exercise4 - Video Streaming"

clean:
	rm -f exercise1 exercise2 exercise3 exercise4
	@echo "✓ Cleaned build files"

test_files:
	@echo "Creating test files..."
	@echo -e "This is line 1\nThis is line 2\nThis is line 3\nThis is line 4\nThis is line 5" > test.txt
	@for i in {1..100}; do echo "Line $$i: ABCDEFGHIJKLMNOPQRSTUVWXYZ"; done > bigfile.txt
	@echo "✓ Created test.txt and bigfile.txt"
	@echo ""
	@echo "Note: You'll need to provide your own test_video.mp4 for exercise 4"
	@echo "Place an MP4 file in this directory and rename it to test_video.mp4"

help:
	@echo "OTT Streaming Practice Exercises - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make all         - Build all exercises"
	@echo "  make exercise1   - Build exercise 1 (Basic HTTP Server)"
	@echo "  make exercise2   - Build exercise 2 (File Server)"
	@echo "  make exercise3   - Build exercise 3 (Range Requests - Text)"
	@echo "  make exercise4   - Build exercise 4 (Video Streaming)"
	@echo "  make test_files  - Create test.txt and bigfile.txt"
	@echo "  make clean       - Remove compiled binaries"
	@echo "  make help        - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  1. make all"
	@echo "  2. make test_files"
	@echo "  3. ./exercise1"

.PHONY: all clean test_files help
