# Makefile for HTTP Server Practice Exercises
# Network Programming Final Project

CC = gcc
CFLAGS = -Wall -g
TARGETS = exercise1 exercise2 exercise3 exercise4

# Default target
.PHONY: all
all: $(TARGETS)
	@echo "✅ All exercises built successfully!"
	@echo "📚 Run each exercise with: ./exercise[1-4]"

# Exercise 1: Basic HTTP Server
exercise1: exercise1_hello_server.c
	$(CC) $(CFLAGS) -o $@ $<
	@echo "✅ Exercise 1 (Basic HTTP Server) built"

# Exercise 2: File Server
exercise2: exercise2_file_server.c
	$(CC) $(CFLAGS) -o $@ $<
	@echo "✅ Exercise 2 (File Server) built"

# Exercise 3: Range Request Text Server
exercise3: exercise3_range_text.c
	$(CC) $(CFLAGS) -o $@ $<
	@echo "✅ Exercise 3 (Range Request Text) built"

# Exercise 4: Video Streaming Server
exercise4: exercise4_video_server.c
	$(CC) $(CFLAGS) -o $@ $<
	@echo "✅ Exercise 4 (Video Streaming) built"

# Create test files for exercises
.PHONY: test_files
test_files:
	@echo "Creating test files..."
	@echo -e "Line 1\nLine 2\nLine 3\nLine 4\nLine 5" > test.txt
	@for i in {1..100}; do echo "Line $$i: ABCDEFGHIJKLMNOPQRSTUVWXYZ"; done > bigfile.txt
	@echo "✅ Test files created: test.txt, bigfile.txt"
	@echo "📝 Note: For exercise 4, you need to add a test_video.mp4 file manually"

# Clean up compiled files
.PHONY: clean
clean:
	rm -f $(TARGETS)
	@echo "🧹 Cleaned all executables"

# Clean everything including test files
.PHONY: cleanall
cleanall: clean
	rm -f test.txt bigfile.txt
	@echo "🧹 Cleaned all executables and test files"

# Run tests for each exercise
.PHONY: test
test:
	@echo "🧪 Testing all exercises..."
	@echo "----------------------------------------"
	@echo "Test Exercise 1:"
	@./exercise1 & SERVER_PID=$$!; \
	sleep 1; \
	curl -s http://localhost:8080/ | head -n 1; \
	kill $$SERVER_PID 2>/dev/null || true
	@echo "----------------------------------------"
	@echo "Test Exercise 2 (needs test.txt):"
	@if [ -f test.txt ]; then \
		./exercise2 & SERVER_PID=$$!; \
		sleep 1; \
		curl -s http://localhost:8080/test.txt | head -n 2; \
		kill $$SERVER_PID 2>/dev/null || true; \
	else \
		echo "⚠️  test.txt not found. Run 'make test_files' first"; \
	fi
	@echo "----------------------------------------"
	@echo "✅ Basic tests completed"

# Kill any servers running on port 8080
.PHONY: kill
kill:
	@echo "🔍 Killing processes on port 8080..."
	@lsof -ti:8080 | xargs kill -9 2>/dev/null || echo "✅ No processes found on port 8080"

# Help target
.PHONY: help
help:
	@echo "================================================"
	@echo "  Network Programming - Practice Exercises"
	@echo "================================================"
	@echo ""
	@echo "Available targets:"
	@echo "  make all         - Build all exercises"
	@echo "  make exercise1   - Build exercise 1 (Basic HTTP)"
	@echo "  make exercise2   - Build exercise 2 (File Server)"
	@echo "  make exercise3   - Build exercise 3 (Range Text)"
	@echo "  make exercise4   - Build exercise 4 (Video Stream)"
	@echo "  make test_files  - Create test files"
	@echo "  make test        - Run basic tests"
	@echo "  make clean       - Remove executables"
	@echo "  make cleanall    - Remove executables and test files"
	@echo "  make kill        - Kill processes on port 8080"
	@echo "  make help        - Show this help message"
	@echo ""
	@echo "Quick start:"
	@echo "  1. make all          # Build everything"
	@echo "  2. make test_files   # Create test files"
	@echo "  3. ./exercise1       # Run exercise 1"
	@echo ""
	@echo "Test an exercise:"
	@echo "  Terminal 1: ./exercise1"
	@echo "  Terminal 2: curl http://localhost:8080/"
	@echo ""

# Individual run targets for convenience
.PHONY: run1 run2 run3 run4
run1: exercise1
	./exercise1

run2: exercise2
	./exercise2

run3: exercise3
	./exercise3

run4: exercise4
	./exercise4

# Default if no target specified
.DEFAULT_GOAL := help