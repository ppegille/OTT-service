# Makefile for OTT Streaming Server - Enhancement Phase 4
#
# Features:
#   - Header dependency tracking (automatic rebuilds on header changes)
#   - DEBUG/RELEASE build modes
#   - Optimized compilation flags
#   - Dependency file generation
#
# Usage:
#   make              - Build the server (default: DEBUG mode)
#   make release      - Build optimized release version
#   make debug        - Build debug version with symbols
#   make clean        - Remove build files
#   make run          - Build and run the server
#   make test         - Run tests

# ============================================================================
# Build Configuration
# ============================================================================

CC = gcc
TARGET = ott_server

# Build mode: DEBUG or RELEASE (default: DEBUG)
BUILD_MODE ?= DEBUG

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
DEP_DIR = $(BUILD_DIR)/deps

# ============================================================================
# Source and Object Files
# ============================================================================

# Source files
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/http.c \
       $(SRC_DIR)/streaming.c \
       $(SRC_DIR)/session.c \
       $(SRC_DIR)/database.c \
       $(SRC_DIR)/crypto.c \
       $(SRC_DIR)/json.c \
       $(SRC_DIR)/json_builder.c \
       $(SRC_DIR)/routes.c \
       $(SRC_DIR)/video_scanner.c \
       $(SRC_DIR)/ffmpeg_utils.c \
       $(SRC_DIR)/validation.c \
       $(SRC_DIR)/logger.c

# Header files (for dependency tracking)
HDRS = $(wildcard $(INC_DIR)/*.h)

# Object files (in build directory)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Dependency files
DEPS = $(patsubst $(SRC_DIR)/%.c,$(DEP_DIR)/%.d,$(SRCS))

# ============================================================================
# Compiler Flags
# ============================================================================

# Common flags
CFLAGS_COMMON = -Wall -Wextra -I$(INC_DIR)

# Debug flags
CFLAGS_DEBUG = $(CFLAGS_COMMON) -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined

# Release flags
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O3 -DNDEBUG -march=native -flto

# Linker flags
LDFLAGS_COMMON = -lpthread -lsqlite3 -lcrypto
LDFLAGS_DEBUG = $(LDFLAGS_COMMON) -fsanitize=address -fsanitize=undefined
LDFLAGS_RELEASE = $(LDFLAGS_COMMON) -flto

# Select flags based on build mode
ifeq ($(BUILD_MODE),RELEASE)
    CFLAGS = $(CFLAGS_RELEASE)
    LDFLAGS = $(LDFLAGS_RELEASE)
    BUILD_TYPE = Release
else
    CFLAGS = $(CFLAGS_DEBUG)
    LDFLAGS = $(LDFLAGS_DEBUG)
    BUILD_TYPE = Debug
endif

# ============================================================================
# Build Targets
# ============================================================================

# Default target
all: $(TARGET)
	@echo "✓ $(BUILD_TYPE) build complete!"
	@echo "Run with: ./$(TARGET)"

# Debug build
debug:
	@$(MAKE) BUILD_MODE=DEBUG all

# Release build
release:
	@$(MAKE) BUILD_MODE=RELEASE all

# Link target
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET) ($(BUILD_TYPE))..."
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile source files with dependency generation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR) $(DEP_DIR)
	@echo "Compiling $< ($(BUILD_TYPE))..."
	@mkdir -p $(dir $@)
	@mkdir -p $(DEP_DIR)
	$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*.d -c $< -o $@

# ============================================================================
# Directory Creation
# ============================================================================

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)

# ============================================================================
# Utility Targets
# ============================================================================

# Clean build files
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "✓ Clean complete"

# Clean all (including dependency files)
distclean: clean
	@echo "Removing all generated files..."
	@rm -f $(TARGET)
	@echo "✓ Distclean complete"

# Build and run
run: $(TARGET)
	@echo "Starting OTT Streaming Server ($(BUILD_TYPE))..."
	./$(TARGET)

# Run tests
test: $(TARGET)
	@echo "Running tests..."
	@bash ../tests/test_streaming.sh

# Show build configuration
info:
	@echo "Build Configuration:"
	@echo "  Build Mode:    $(BUILD_MODE)"
	@echo "  CC:            $(CC)"
	@echo "  CFLAGS:        $(CFLAGS)"
	@echo "  LDFLAGS:       $(LDFLAGS)"
	@echo "  Target:        $(TARGET)"
	@echo "  Sources:       $(words $(SRCS)) files"
	@echo "  Headers:       $(words $(HDRS)) files"

# Help
help:
	@echo "OTT Streaming Server - Makefile"
	@echo ""
	@echo "Build Targets:"
	@echo "  make          - Build server (DEBUG mode, default)"
	@echo "  make debug    - Build with debug symbols"
	@echo "  make release  - Build optimized release version"
	@echo ""
	@echo "Utility Targets:"
	@echo "  make clean    - Remove build files"
	@echo "  make distclean- Remove all generated files"
	@echo "  make run      - Build and run server"
	@echo "  make test     - Run test scripts"
	@echo "  make info     - Show build configuration"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Build Modes:"
	@echo "  DEBUG   - Debug symbols, address sanitizer, no optimization"
	@echo "  RELEASE - Optimized (-O3), LTO, native architecture"

# ============================================================================
# Dependency Inclusion
# ============================================================================

# Include dependency files (auto-rebuild on header changes)
-include $(DEPS)

# ============================================================================
# Phony Targets
# ============================================================================

.PHONY: all debug release clean distclean run test info help
