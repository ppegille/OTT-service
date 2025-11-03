# MVP Phase 2 - 실행 가이드

**축하합니다!** MVP Phase 1을 성공적으로 완료하셨습니다. 이제 동시 접속 처리를 추가하겠습니다.

---

## 📋 Phase 2에서 추가된 기능

### ✅ 새로운 기능
- [x] Fork-based 동시 접속 처리
- [x] 여러 사용자 동시 스트리밍 지원
- [x] 좀비 프로세스 방지 (SIGCHLD 시그널 처리)
- [x] 각 클라이언트별 독립적인 프로세스 처리

### 🔄 변경 사항
- **프로세스 모델**: 단일 연결 → Fork 기반 다중 연결
- **동시성**: 1명 → 무제한 동시 사용자 (시스템 리소스 한도 내)
- **안정성**: 좀비 프로세스 자동 제거

---

## 🚀 빠른 시작 (3단계)

### 1단계: 서버 재컴파일

기존 빌드 파일을 제거하고 새로 컴파일합니다:

```bash
cd server
make clean
make
```

**예상 출력:**
```
Cleaning build files...
✓ Clean complete
Compiling src/main.c...
Compiling src/http.c...
Compiling src/streaming.c...
Linking ott_server...
✓ Build complete!
Run with: ./ott_server
```

✅ 에러 없이 컴파일되면 성공!

### 2단계: Phase 2 서버 실행

```bash
./ott_server
```

**예상 출력:**
```
=== OTT Streaming Server - MVP Phase 2 ===
    (Fork-based Concurrent Connection Support)

Step 1: Creating socket...
✓ Socket created successfully

Step 2: Binding to port 8080...
✓ Successfully bound to port 8080

Step 3: Listening for connections...
✓ Server is listening

🚀 OTT Streaming Server is running!
   Access the player at: http://localhost:8080/
   Press Ctrl+C to stop the server

Waiting for connection...
```

✅ **"Fork-based Concurrent Connection Support"** 메시지가 나오면 Phase 2가 실행 중입니다!

### 3단계: 동시 접속 테스트

#### 방법 1: 브라우저 다중 탭으로 수동 테스트

1. **Chrome 브라우저**를 엽니다
2. **첫 번째 탭**: `http://localhost:8080/` 접속 → 비디오 재생
3. **두 번째 탭** (Ctrl+T): `http://localhost:8080/` 접속 → 비디오 재생
4. **세 번째 탭** (Ctrl+T): `http://localhost:8080/` 접속 → 비디오 재생

**서버 터미널 확인:**
```
Waiting for connection...
✓ Client connected: 127.0.0.1 (PID: 12345)
  [Child 12346] GET /
  → 200 OK: XXXX bytes
  [Child 12346] Connection closed

✓ Client connected: 127.0.0.1 (PID: 12345)
  [Child 12347] GET /
  → 200 OK: XXXX bytes
  [Child 12347] Connection closed

✓ Client connected: 127.0.0.1 (PID: 12345)
  [Child 12348] GET /videos/test_video.mp4
  → 206 Partial Content: bytes 0-65535/XXXXXX
  [Child 12348] Connection closed

✓ Client connected: 127.0.0.1 (PID: 12345)
  [Child 12349] GET /videos/test_video.mp4
  → 206 Partial Content: bytes 0-65535/XXXXXX
  [Child 12349] Connection closed
```

**확인 포인트:**
- ✅ 각 연결마다 다른 Child PID가 표시됨
- ✅ 여러 탭에서 동시에 비디오 재생 가능
- ✅ 한 탭의 재생이 다른 탭에 영향을 주지 않음
- ✅ Seeking 작업이 독립적으로 작동

#### 방법 2: 자동 테스트 스크립트 사용

자동 테스트 스크립트를 실행합니다:

```bash
cd ../tests
bash concurrent_test.sh
```

**예상 출력:**
```
========================================
OTT Server - 동시 접속 테스트
========================================

테스트 1: 서버 응답 확인
✓ 서버가 응답합니다

테스트 2: 동시 3명 사용자 스트리밍
  사용자 1 시작...
  사용자 2 시작...
  사용자 3 시작...
  모든 사용자 완료 대기 중...
✓ 3명의 사용자가 동시에 스트리밍 성공

테스트 3: 프로세스 격리 확인
✓ 각 연결이 독립적인 프로세스에서 처리됨

========================================
✅ 모든 테스트 통과!
========================================
```

---

## 🧪 상세 테스트 가이드

### 테스트 1: 기본 동시 재생

**목표**: 2-3개 탭에서 동시에 비디오가 재생되는지 확인

**절차**:
1. 3개의 브라우저 탭을 엽니다
2. 모든 탭에서 `http://localhost:8080/` 접속
3. 각 탭에서 비디오 재생 버튼을 클릭
4. 모든 비디오가 정상적으로 재생되는지 확인

**성공 기준**:
- ✅ 3개 탭 모두 비디오 재생됨
- ✅ 서버 터미널에 다른 Child PID 표시됨
- ✅ 재생 속도나 버퍼링에 문제 없음

### 테스트 2: Seek 독립성 테스트

**목표**: 한 사용자의 Seek가 다른 사용자에게 영향을 주지 않는지 확인

**절차**:
1. 탭 1: 비디오 처음부터 재생
2. 탭 2: 비디오 중간 지점으로 Seek
3. 탭 3: 비디오 끝 부분으로 Seek
4. 각 탭이 독립적으로 동작하는지 확인

**성공 기준**:
- ✅ 각 탭이 서로 다른 위치에서 재생
- ✅ 한 탭의 Seek가 다른 탭에 영향 없음
- ✅ 서버 로그에서 각각 다른 Range 요청 확인

### 테스트 3: 부하 테스트 (5명 동시 접속)

**절차**:
1. 5개의 브라우저 탭 열기
2. 모든 탭에서 동시에 비디오 재생
3. 각 탭에서 무작위로 Seek 수행
4. 5분간 안정적으로 재생되는지 확인

**성공 기준**:
- ✅ 5개 탭 모두 안정적으로 재생
- ✅ 프레임 드롭이나 버퍼링 최소화
- ✅ 서버 프로세스가 크래시되지 않음

### 테스트 4: curl로 동시 요청 테스트

새 터미널 창에서:

```bash
# 터미널 1
curl -v -r 0-1023 http://localhost:8080/videos/test_video.mp4 -o chunk1.bin &

# 터미널 2
curl -v -r 1024-2047 http://localhost:8080/videos/test_video.mp4 -o chunk2.bin &

# 터미널 3
curl -v -r 2048-3071 http://localhost:8080/videos/test_video.mp4 -o chunk3.bin &

# 모든 작업 완료 대기
wait
```

**확인 사항**:
```bash
ls -lh chunk*.bin
```

**예상 출력:**
```
-rw-r--r-- 1 user user 1.0K chunk1.bin
-rw-r--r-- 1 user user 1.0K chunk2.bin
-rw-r--r-- 1 user user 1.0K chunk3.bin
```

✅ 세 파일 모두 정확히 1KB(1024 bytes)여야 함

---

## 📊 Phase 2 아키텍처 설명

### Fork-based 프로세스 모델

```
                    Parent Process
                  (Server Listening)
                         |
        +----------------+----------------+
        |                |                |
    Connection 1    Connection 2    Connection 3
        |                |                |
   fork() → Child   fork() → Child   fork() → Child
     PID: 101         PID: 102         PID: 103
        |                |                |
   Handle Request   Handle Request   Handle Request
        |                |                |
     exit(0)          exit(0)          exit(0)
```

**동작 원리**:
1. **Parent Process**: 포트 8080에서 연결 대기 (무한 루프)
2. **클라이언트 연결**: `accept()`로 새 연결 수락
3. **Fork**: `fork()`로 자식 프로세스 생성
4. **Child Process**:
   - HTTP 요청 읽기 및 파싱
   - 비디오 스트리밍 처리
   - 연결 종료 후 `exit(0)`
5. **Parent Process**:
   - 자식 프로세스를 생성한 후 즉시 다음 연결 대기
   - `SIGCHLD` 시그널로 종료된 자식 프로세스 자동 회수

### 좀비 프로세스 방지

```c
// SIGCHLD 시그널 핸들러
void sigchld_handler(int sig) {
    (void)sig;
    // 종료된 모든 자식 프로세스 회수
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
```

**이점**:
- 자식 프로세스가 종료되면 자동으로 리소스 회수
- 좀비 프로세스(Zombie Process) 방지
- 시스템 리소스 효율적 관리

---

## 🔍 디버깅 및 모니터링

### 실시간 프로세스 모니터링

서버가 실행 중인 상태에서 새 터미널을 열고:

```bash
# 서버 프로세스와 자식 프로세스 확인
ps aux | grep ott_server
```

**예상 출력:**
```
user  12345  0.0  0.1  ott_server (parent)
user  12346  0.0  0.1  ott_server (child)
user  12347  0.0  0.1  ott_server (child)
```

### 네트워크 연결 상태 확인

```bash
# 포트 8080의 연결 확인
netstat -antp | grep 8080
```

**예상 출력:**
```
tcp  0  0  0.0.0.0:8080  0.0.0.0:*  LISTEN  12345/ott_server
tcp  0  0  127.0.0.1:8080  127.0.0.1:54321  ESTABLISHED  12346/ott_server
tcp  0  0  127.0.0.1:8080  127.0.0.1:54322  ESTABLISHED  12347/ott_server
```

### 시스템 리소스 사용량 확인

```bash
# CPU 및 메모리 사용량 모니터링
top -p $(pgrep -d',' ott_server)
```

---

## 📈 성능 벤치마크

### 동시 접속 한계 테스트

**테스트 도구**: Apache Bench (ab)

```bash
# 10명의 동시 사용자, 총 100번의 요청
ab -n 100 -c 10 http://localhost:8080/
```

**예상 결과**:
```
Concurrency Level:      10
Time taken for tests:   X.XXX seconds
Complete requests:      100
Failed requests:        0
```

✅ Failed requests가 0이면 성공!

---

## 🐛 문제 해결

### 문제 1: "Too many open files" 오류

**원인**: 시스템의 파일 디스크립터 한계

**해결**:
```bash
# 현재 한계 확인
ulimit -n

# 한계 증가 (임시)
ulimit -n 4096
```

### 문제 2: 좀비 프로세스 발생

**확인**:
```bash
ps aux | grep defunct
```

**해결**:
- 서버를 Ctrl+C로 종료하고 재시작
- SIGCHLD 핸들러가 제대로 등록되었는지 확인

### 문제 3: 포트가 이미 사용 중

**해결**:
```bash
# 기존 서버 프로세스 종료
pkill ott_server

# 또는 포트 사용 프로세스 강제 종료
lsof -ti:8080 | xargs kill -9
```

### 문제 4: 동시 접속 시 느려짐

**원인**:
- 비디오 파일이 너무 큼
- 시스템 리소스 부족

**해결**:
1. 더 작은 비디오 파일로 테스트
2. 시스템 리소스 확인 (`top`, `free -h`)
3. 버퍼 크기 조정 (server.h의 BUFFER_SIZE)

---

## ✅ Phase 2 완료 체크리스트

다음 모든 항목이 작동하면 **MVP Phase 2 완료**:

- [x] 서버 재컴파일 성공
- [x] "Fork-based Concurrent Connection Support" 메시지 표시
- [ ] 브라우저 3개 탭에서 동시 재생 성공
- [ ] 각 연결마다 다른 Child PID 표시
- [ ] Seek 작업이 독립적으로 동작
- [ ] curl 동시 요청 테스트 성공
- [ ] 5분 이상 안정적으로 동작
- [ ] 좀비 프로세스 없음 (`ps aux | grep defunct`)

**모두 체크되었나요?** 축하합니다! 🎉

---

## 📝 다음 단계: 세션 관리 추가

MVP Phase 2가 완료되면:

1. **세션 관리**: 쿠키 기반 UUID 세션
2. **간단한 로그인 페이지**: ID만 입력 (비밀번호 없음)
3. **세션 검증**: 로그인한 사용자만 비디오 접근

**준비되셨나요?** "세션 관리 추가해주세요" 라고 말씀해주세요!

---

## 💡 유용한 명령어 요약

```bash
# 서버 빌드 및 실행
cd server
make clean && make
./ott_server

# 동시 테스트 실행
cd ../tests
bash concurrent_test.sh

# 프로세스 모니터링
ps aux | grep ott_server
netstat -antp | grep 8080

# 서버 중지
Ctrl + C (서버 터미널에서)
# 또는
pkill ott_server
```

---

**축하합니다!** 🎉

동시 접속을 지원하는 OTT 스트리밍 서버를 완성하셨습니다!

질문이 있으시면 언제든지 물어보세요!
