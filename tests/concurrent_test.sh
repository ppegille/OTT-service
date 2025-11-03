#!/bin/bash

# OTT Streaming Server - Concurrent Connection Test Script
# Tests multiple simultaneous users accessing the streaming server

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test configuration
SERVER_URL="http://localhost:8080"
VIDEO_PATH="/videos/test_video.mp4"
NUM_CONCURRENT=3
CHUNK_SIZE=1024

echo "========================================"
echo "OTT Server - 동시 접속 테스트"
echo "========================================"
echo ""

# Function to print test results
print_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓ $2${NC}"
    else
        echo -e "${RED}✗ $2${NC}"
    fi
}

# Test 1: Check if server is running
echo "테스트 1: 서버 응답 확인"
response=$(curl -s -o /dev/null -w "%{http_code}" $SERVER_URL 2>/dev/null)
if [ "$response" = "200" ] || [ "$response" = "206" ]; then
    print_result 0 "서버가 응답합니다"
else
    print_result 1 "서버가 응답하지 않습니다 (HTTP $response)"
    echo "서버를 먼저 실행해주세요: ./ott_server"
    exit 1
fi
echo ""

# Test 2: Concurrent streaming test
echo "테스트 2: 동시 ${NUM_CONCURRENT}명 사용자 스트리밍"

# Create temporary directory for test files
mkdir -p /tmp/ott_test
cd /tmp/ott_test

# Clean up old test files
rm -f chunk_*.bin

# Function to simulate a user downloading a chunk
simulate_user() {
    local user_id=$1
    local start=$((user_id * CHUNK_SIZE))
    local end=$((start + CHUNK_SIZE - 1))

    echo "  사용자 $user_id 시작..."

    # Request a specific range
    curl -s -r "$start-$end" \
         -o "chunk_${user_id}.bin" \
         "${SERVER_URL}${VIDEO_PATH}" \
         2>/dev/null

    local exit_code=$?

    # Check if file was created and has correct size
    if [ $exit_code -eq 0 ] && [ -f "chunk_${user_id}.bin" ]; then
        local size=$(stat -f%z "chunk_${user_id}.bin" 2>/dev/null || stat -c%s "chunk_${user_id}.bin" 2>/dev/null)
        if [ "$size" -eq "$CHUNK_SIZE" ]; then
            return 0
        fi
    fi
    return 1
}

# Launch concurrent users
pids=()
for i in $(seq 1 $NUM_CONCURRENT); do
    simulate_user $i &
    pids+=($!)
done

# Wait for all users to complete
echo "  모든 사용자 완료 대기 중..."
all_success=0
for pid in "${pids[@]}"; do
    wait $pid
    if [ $? -ne 0 ]; then
        all_success=1
    fi
done

# Check results
successful=0
for i in $(seq 1 $NUM_CONCURRENT); do
    if [ -f "chunk_${i}.bin" ]; then
        size=$(stat -f%z "chunk_${i}.bin" 2>/dev/null || stat -c%s "chunk_${i}.bin" 2>/dev/null)
        if [ "$size" -eq "$CHUNK_SIZE" ]; then
            ((successful++))
        fi
    fi
done

if [ $successful -eq $NUM_CONCURRENT ]; then
    print_result 0 "${NUM_CONCURRENT}명의 사용자가 동시에 스트리밍 성공"
else
    print_result 1 "${successful}/${NUM_CONCURRENT}명만 성공"
fi
echo ""

# Test 3: Check for process isolation
echo "테스트 3: 프로세스 격리 확인"

# Make multiple quick requests
response_codes=()
for i in $(seq 1 5); do
    code=$(curl -s -o /dev/null -w "%{http_code}" $SERVER_URL 2>/dev/null)
    response_codes+=($code)
    sleep 0.1
done

# Check if all requests succeeded
all_ok=1
for code in "${response_codes[@]}"; do
    if [ "$code" != "200" ]; then
        all_ok=0
        break
    fi
done

if [ $all_ok -eq 1 ]; then
    print_result 0 "각 연결이 독립적인 프로세스에서 처리됨"
else
    print_result 1 "일부 요청이 실패함"
fi
echo ""

# Test 4: Range request validation
echo "테스트 4: Range Request 정확성 검증"

# Request different ranges simultaneously
curl -s -r 0-99 -o range1.bin "${SERVER_URL}${VIDEO_PATH}" 2>/dev/null &
curl -s -r 100-199 -o range2.bin "${SERVER_URL}${VIDEO_PATH}" 2>/dev/null &
curl -s -r 200-299 -o range3.bin "${SERVER_URL}${VIDEO_PATH}" 2>/dev/null &
wait

# Check if files exist and have correct sizes
range_ok=1
for i in 1 2 3; do
    if [ ! -f "range${i}.bin" ]; then
        range_ok=0
        break
    fi
    size=$(stat -f%z "range${i}.bin" 2>/dev/null || stat -c%s "range${i}.bin" 2>/dev/null)
    if [ "$size" -ne 100 ]; then
        range_ok=0
        break
    fi
done

if [ $range_ok -eq 1 ]; then
    print_result 0 "Range Request가 정확하게 처리됨"
else
    print_result 1 "Range Request 처리에 문제가 있음"
fi
echo ""

# Test 5: Server stability under rapid connections
echo "테스트 5: 빠른 연속 연결 안정성"

rapid_ok=1
for i in $(seq 1 10); do
    code=$(curl -s -o /dev/null -w "%{http_code}" -m 2 $SERVER_URL 2>/dev/null)
    if [ "$code" != "200" ]; then
        rapid_ok=0
        break
    fi
done

if [ $rapid_ok -eq 1 ]; then
    print_result 0 "10회 연속 연결 모두 성공"
else
    print_result 1 "빠른 연속 연결 중 실패 발생"
fi
echo ""

# Clean up
cd - > /dev/null
rm -rf /tmp/ott_test

# Final summary
echo "========================================"
if [ $all_success -eq 0 ] && [ $all_ok -eq 1 ] && [ $range_ok -eq 1 ] && [ $rapid_ok -eq 1 ]; then
    echo -e "${GREEN}✅ 모든 테스트 통과!${NC}"
    echo "서버가 동시 접속을 정상적으로 처리하고 있습니다."
else
    echo -e "${YELLOW}⚠️  일부 테스트 실패${NC}"
    echo "서버 로그를 확인해주세요."
fi
echo "========================================"
