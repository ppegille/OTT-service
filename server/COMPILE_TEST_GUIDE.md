# 공유 메모리 구현 컴파일 및 테스트 가이드

## ✅ 완료된 구현 사항

### 1. 공유 메모리 세션 저장소
- **파일**: `src/session.c`
- **구조체**: `SharedSessionStore` (POSIX 공유 메모리)
- **세마포어**: `/ott_session_sem` (동기화)
- **함수들**:
  - `init_session_store()` - 공유 메모리 및 세마포어 생성
  - `create_session()` - 스레드 안전 세션 생성
  - `validate_session()` - 스레드 안전 검증
  - `cleanup_session_store()` - 정리 핸들러

### 2. 메인 서버 수정
- **파일**: `src/main.c`
- **추가사항**:
  - SIGINT/SIGTERM 시그널 핸들러 → `cleanup_session_store()` 호출
  - `atexit()` 핸들러 → 비정상 종료 시 정리
  - 세션 검증 미들웨어 통합

### 3. Makefile 업데이트
- **LDFLAGS**: `-lpthread` 추가 (세마포어 지원)
- **SRCS**: `src/session.c` 포함

---

## 🚀 컴파일 방법 (WSL2에서 실행)

### Step 1: WSL2로 전환
```bash
# Git Bash에서 WSL2로 전환
wsl
```

### Step 2: 프로젝트 디렉토리로 이동
```bash
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트/server
```

### Step 3: 클린 빌드
```bash
make clean
```

**예상 출력:**
```
Cleaning build files...
rm -f ott_server src/main.o src/http.o src/streaming.o src/session.o
✓ Clean complete
```

### Step 4: 컴파일
```bash
make
```

**예상 출력:**
```
Compiling src/main.c...
Compiling src/http.c...
Compiling src/streaming.c...
Compiling src/session.c...
Linking ott_server...
✓ Build complete!
Run with: ./ott_server
```

### Step 5: 서버 실행
```bash
./ott_server
```

**예상 출력:**
```
=== OTT Streaming Server - Enhancement Phase 1 ===
    (Session Management & User Authentication)

✓ Shared memory created: ID=12345
✓ Session semaphore initialized: /ott_session_sem
✓ Session store initialized (Shared Memory)

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

---

## 🧪 테스트 시나리오

### Test 1: 로그인 페이지 접근
**브라우저**: `http://localhost:8080/`

**예상 동작:**
1. 세션 없음 → 자동으로 `/login`으로 리디렉션
2. 로그인 페이지 표시

**서버 로그:**
```
✓ Client connected: 127.0.0.1 (PID: 1234)
  [Child 1234] GET /
  [DEBUG] Cookie header: '(null)'
  [DEBUG] Parsed session_id: '(null)'
  [Child 1234] No valid session, redirecting to login
  [Child 1234] Connection closed
```

### Test 2: 로그인 시도
**브라우저**: 로그인 폼에서 사용자 ID 입력 후 제출

**예상 동작:**
1. POST /login 요청 전송
2. 서버가 세션 생성 (공유 메모리에 저장)
3. Set-Cookie 헤더로 session_id 반환
4. 302 리디렉션 → `/`

**서버 로그:**
```
✓ Client connected: 127.0.0.1 (PID: 1235)
  [Child 1235] POST /login
  [DEBUG] POST /login received
  [DEBUG] Body found, calling handle_login
  [Session] Created new session: abc123...
  [Child 1235] Connection closed
```

### Test 3: 로그인 후 자동 리디렉션 ⭐ **핵심 테스트**
**브라우저**: 로그인 성공 후 자동으로 `/`로 이동

**예상 동작:**
1. 브라우저가 쿠키와 함께 GET / 요청
2. **다른 child process**가 요청 처리
3. **공유 메모리에서** 세션 검증 ✅
4. 세션 유효 → `player.html` 제공

**서버 로그:**
```
✓ Client connected: 127.0.0.1 (PID: 1236)  ← 다른 PID!
  [Child 1236] GET /
  [DEBUG] Cookie header: 'session_id=abc123...'
  [DEBUG] Parsed session_id: 'abc123...'
  [Child 1236] Valid session: abc123...
  [Child 1236] Connection closed
```

**성공 조건:**
- ✅ PID가 다른데도 세션 검증 성공
- ✅ 로그인 루프 없음
- ✅ player.html 제공

### Test 4: 멀티 유저 동시 접속 (포크 메모리 격리 검증)
**브라우저**: 2개 이상의 브라우저 탭/창으로 동시 접속

**예상 동작:**
1. 탭 1: 사용자 "alice" 로그인 → 세션 1 생성
2. 탭 2: 사용자 "bob" 로그인 → 세션 2 생성
3. 두 세션이 **공유 메모리에 공존**
4. 각 탭에서 새로고침 시 각자의 세션 유지

**서버 로그:**
```
[Session] Created new session for 'alice': session-1
[Session] Created new session for 'bob': session-2
[Child 1240] Valid session: session-1 (alice)
[Child 1241] Valid session: session-2 (bob)
```

### Test 5: 세션 타임아웃 (30분)
**시간**: 30분 후

**예상 동작:**
1. 30분 후 페이지 새로고침
2. 세션 만료 → 자동 로그인 페이지로 리디렉션

---

## 🔍 문제 해결 가이드

### 문제 1: "semaphore already exists" 오류
**원인**: 이전 서버 종료 시 세마포어 정리 실패

**해결:**
```bash
# 세마포어 수동 제거
ipcs -s  # 세마포어 ID 확인
ipcrm -s [세마포어 ID]

# 또는 자동 정리 스크립트
ipcs -s | grep ott_session_sem | awk '{print $2}' | xargs -I {} ipcrm -s {}
```

### 문제 2: "shared memory already exists" 오류
**원인**: 이전 서버 종료 시 공유 메모리 정리 실패

**해결:**
```bash
# 공유 메모리 수동 제거
ipcs -m  # 공유 메모리 ID 확인
ipcrm -m [공유 메모리 ID]

# 또는 자동 정리
ipcs -m | awk '/0x/ {print $2}' | xargs -I {} ipcrm -m {}
```

### 문제 3: 여전히 로그인 루프 발생
**진단 방법:**
```bash
# 디버그 로그 확인
# 서버 실행 시 다음 로그를 찾으세요:

# ✅ 정상 (공유 메모리 작동):
[DEBUG] Cookie header: 'session_id=abc123...'
[DEBUG] Parsed session_id: 'abc123...'
[Child 1236] Valid session: abc123...

# ❌ 비정상 (세션 공유 실패):
[DEBUG] Cookie header: 'session_id=abc123...'
[DEBUG] Parsed session_id: 'abc123...'
[Child 1236] No valid session, redirecting to login
```

**해결:**
1. 서버 재시작: `Ctrl+C` → `./ott_server`
2. 브라우저 쿠키 삭제: F12 → Application → Cookies → localhost:8080 삭제
3. 세마포어/공유메모리 정리 후 재실행

### 문제 4: 컴파일 에러
**예상 에러:**
```
undefined reference to `sem_open'
undefined reference to `shmat'
```

**확인:**
```bash
grep LDFLAGS Makefile
# 출력: LDFLAGS = -lpthread
```

**해결:**
```bash
# Makefile에 -lpthread가 없다면 추가
make clean
make
```

---

## 📊 성공 확인 체크리스트

- [ ] **컴파일**: `make` 명령 성공 (경고 무시, "Build complete" 확인)
- [ ] **서버 시작**: "Session store initialized (Shared Memory)" 메시지 확인
- [ ] **로그인 페이지**: `http://localhost:8080/` → 로그인 페이지 표시
- [ ] **로그인 성공**: POST /login → 200 OK
- [ ] **자동 리디렉션**: 로그인 후 자동으로 player.html 표시 ⭐
- [ ] **세션 유지**: 페이지 새로고침 시 로그인 상태 유지
- [ ] **멀티 유저**: 2개 탭에서 각각 다른 사용자로 로그인 성공
- [ ] **PID 다름 확인**: 로그에서 각 요청의 PID가 다른지 확인
- [ ] **정상 종료**: `Ctrl+C` 시 "Shared memory detached and removed" 메시지

---

## 🎯 핵심 검증 포인트

### 공유 메모리 작동 확인
```
Process Flow:
1. Parent Process → init_session_store() → 공유 메모리 생성
2. Child Process A → handle_login() → 공유 메모리에 세션 저장
3. Child Process A → exit()
4. Child Process B → validate_session() → 공유 메모리에서 세션 읽기 ✅
5. Child Process B → player.html 제공

핵심: Child A와 Child B가 **같은 공유 메모리**를 보고 있어야 함!
```

### 기대 결과
- **이전 (Static Array)**: Child B가 세션을 못 찾음 → 로그인 루프
- **현재 (Shared Memory)**: Child B가 세션 찾음 → player.html 제공 ✅

---

## 📝 다음 단계 (테스트 성공 후)

1. **Enhancement Phase 1 완료 확인**
   - 세션 관리 ✅
   - 로그인/로그아웃 ✅
   - 멀티 유저 동시 접속 ✅

2. **Enhancement Phase 2 진행**
   - SQLite 데이터베이스 통합
   - 사용자 정보 영구 저장
   - 비디오 메타데이터 DB 저장

3. **Enhancement Phase 3 진행**
   - 시청 기록 관리
   - 재생 위치 저장/복원
   - 추천 시스템

---

## 💬 보고 형식

테스트 완료 후 다음과 같이 보고해 주세요:

```
✅ 성공:
- 컴파일: OK
- 로그인: OK
- 리디렉션: OK
- 세션 유지: OK
- 멀티 유저: OK

또는

❌ 실패:
- 문제: [구체적인 문제 설명]
- 로그: [서버 로그 복사]
- 브라우저: [브라우저 동작 설명]
```

---

**작성일**: 2025-11-03
**Phase**: Enhancement Phase 1 - Shared Memory Implementation
**Status**: Ready for Testing 🚀
