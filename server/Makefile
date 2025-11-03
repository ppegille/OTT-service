# Makefile for OTT Streaming Server - Enhancement Phase 3
#
# Usage:
#   make        - Build the server
#   make clean  - Remove build files
#   make run    - Build and run the server
#   make test   - Run tests

CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = -lpthread -lsqlite3 -lcrypto

# Source files
SRCS = src/main.c src/http.c src/streaming.c src/session.c src/database.c src/crypto.c src/json.c src/video_scanner.c
OBJS = $(SRCS:.c=.o)

# Target executable
TARGET = ott_server

# Default target
all: $(TARGET)
	@echo "✓ Build complete!"
	@echo "Run with: ./$(TARGET)"

# Build target
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile source files
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -f $(TARGET) $(OBJS)
	@echo "✓ Clean complete"

# Build and run
run: $(TARGET)
	@echo "Starting OTT Streaming Server..."
	./$(TARGET)

# Run tests
test:
	@echo "Running tests..."
	@bash ../tests/test_streaming.sh

# Help
help:
	@echo "OTT Streaming Server - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build the server"
	@echo "  make clean    - Remove build files"
	@echo "  make run      - Build and run server"
	@echo "  make test     - Run test scripts"
	@echo "  make help     - Show this help"

.PHONY: all clean run test help
