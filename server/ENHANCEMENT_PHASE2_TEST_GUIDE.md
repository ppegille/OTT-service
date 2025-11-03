# Enhancement Phase 2 테스트 가이드

**작성일**: 2025-11-03
**Phase**: Enhancement Phase 2 - Database Integration
**목표**: SQLite 데이터베이스 및 실제 사용자 인증 테스트

---

## 🚀 빠른 시작

### WSL2에서 컴파일 및 실행

```bash
# WSL2 터미널 열기
wsl

# 프로젝트 디렉토리로 이동
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트/server

# 클린 빌드
make clean
make

# 서버 실행
./ott_server
```

---

## 📋 예상 출력

### 성공적인 서버 시작

```
=== OTT Streaming Server - Enhancement Phase 2 ===
    (Database Integration & User Authentication)

Step 0: Initializing database...
✓ Database opened: database/ott.db
✓ Database schema created
✓ Database seeded with test data

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

### Test 1: 데이터베이스 초기화 확인

**실행**:
```bash
ls -la database/
```

**예상 결과**:
```
database/
├── schema.sql          (테이블 스키마)
├── seed.sql            (초기 데이터)
└── ott.db              (SQLite 데이터베이스 파일)
```

**DB 확인**:
```bash
sqlite3 database/ott.db "SELECT * FROM users;"
```

**예상 출력**:
```
1|alice|ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f|2025-11-03 10:00:00|
2|bob|ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f|2025-11-03 10:00:00|
3|testuser|ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f|2025-11-03 10:00:00|
```

---

### Test 2: 로그인 페이지 접근

**브라우저**: `http://localhost:8080/`

**예상 동작**:
1. 자동으로 `/login`으로 리디렉션
2. 비밀번호 필드가 포함된 로그인 폼 표시

**화면 구성**:
```
🎬 OTT
Video Streaming Platform

사용자 이름: [입력 필드]
비밀번호: [입력 필드]

[로그인 버튼]
```

**서버 로그**:
```
✓ Client connected: 127.0.0.1 (PID: 1234)
  [Child 1234] GET /
  [DEBUG] Cookie header: '(null)'
  [DEBUG] Parsed session_id: '(null)'
  [Child 1234] No valid session, redirecting to login
  [Child 1234] Connection closed
```

---

### Test 3: 로그인 성공 (올바른 비밀번호)

**입력**:
- 사용자 이름: `alice`
- 비밀번호: `password123`

**예상 동작**:
1. POST /login 요청
2. 데이터베이스에서 사용자 인증
3. SHA-256 비밀번호 검증
4. 세션 생성 및 쿠키 설정
5. 302 리디렉션 → `/`
6. player.html 표시

**서버 로그**:
```
✓ Client connected: 127.0.0.1 (PID: 1235)
  [Child 1235] POST /login
  [DEBUG] POST /login received
  [DEBUG] Body found, calling handle_login
  [DEBUG] POST body: 'username=alice&password=password123'
  [DEBUG] Parsed username: 'alice'
  [Auth] User 'alice' authenticated successfully (ID: 1)
✓ Session created: abc123-def4-5678-90ab-cdef12345678 for user 'alice' (ID: 1, total: 1)
  [Child 1235] Connection closed

✓ Client connected: 127.0.0.1 (PID: 1236)
  [Child 1236] GET /
  [DEBUG] Cookie header: 'session_id=abc123-def4-5678-90ab-cdef12345678'
  [DEBUG] Parsed session_id: 'abc123-def4-5678-90ab-cdef12345678'
  [Child 1236] Valid session: abc123-def4-5678-90ab-cdef12345678
  [Child 1236] Connection closed
```

**성공 조건**:
- ✅ 로그인 후 player.html 표시
- ✅ 다른 PID의 child process에서도 세션 인식
- ✅ 데이터베이스에서 인증 성공

---

### Test 4: 로그인 실패 (잘못된 비밀번호)

**입력**:
- 사용자 이름: `alice`
- 비밀번호: `wrongpassword`

**예상 동작**:
1. POST /login 요청
2. 데이터베이스 인증 실패
3. 에러 페이지 표시

**화면**:
```
❌ Login Failed

Invalid username or password

[Try Again 버튼]
```

**서버 로그**:
```
✓ Client connected: 127.0.0.1 (PID: 1237)
  [Child 1237] POST /login
  [DEBUG] POST body: 'username=alice&password=wrongpassword'
  [DEBUG] Parsed username: 'alice'
  [Auth] Invalid password for user 'alice'
❌ Authentication failed for user 'alice'
  [Child 1237] Connection closed
```

---

### Test 5: 존재하지 않는 사용자

**입력**:
- 사용자 이름: `nonexistent`
- 비밀번호: `password123`

**예상 동작**:
1. POST /login 요청
2. 데이터베이스에 사용자 없음
3. 에러 페이지 표시

**서버 로그**:
```
  [Auth] User 'nonexistent' not found
❌ Authentication failed for user 'nonexistent'
```

---

### Test 6: 멀티 유저 동시 로그인

**시나리오**:
1. 브라우저 탭 1: `alice`로 로그인
2. 브라우저 탭 2: `bob`으로 로그인
3. 각 탭에서 페이지 새로고침

**예상 동작**:
- 각 사용자의 세션이 독립적으로 유지
- 데이터베이스에서 각각 user_id가 다름 (alice=1, bob=2)

**서버 로그**:
```
✓ Session created: session-1 for user 'alice' (ID: 1, total: 1)
✓ Session created: session-2 for user 'bob' (ID: 2, total: 2)
[Child 1240] Valid session: session-1
[Child 1241] Valid session: session-2
```

**데이터베이스 확인**:
```bash
sqlite3 database/ott.db "SELECT user_id, username, last_login FROM users WHERE last_login IS NOT NULL;"
```

---

### Test 7: 세션 유지 확인

**시나리오**:
1. `alice`로 로그인
2. player.html 표시 확인
3. 페이지 새로고침 (F5)
4. 다른 브라우저 탭에서 `http://localhost:8080/` 접속

**예상 동작**:
- 새로고침 시 로그인 상태 유지
- 다른 탭에서도 같은 세션으로 player.html 표시

**성공 조건**:
- ✅ 쿠키가 브라우저에 저장됨
- ✅ 세션 타임아웃 전까지 로그인 상태 유지
- ✅ 공유 메모리에서 세션 정보 공유

---

## 🔍 디버깅 가이드

### 문제 1: 컴파일 에러 - sqlite3 not found

**에러 메시지**:
```
fatal error: sqlite3.h: No such file or directory
```

**해결**:
```bash
sudo apt-get update
sudo apt-get install libsqlite3-dev
```

---

### 문제 2: 컴파일 에러 - openssl not found

**에러 메시지**:
```
fatal error: openssl/sha.h: No such file or directory
```

**해결**:
```bash
sudo apt-get install libssl-dev
```

---

### 문제 3: Database file not found

**증상**: 서버 시작 시 "Failed to open SQL file: database/schema.sql"

**확인**:
```bash
ls database/
```

**해결**: database/ 디렉토리에 schema.sql과 seed.sql이 있는지 확인

---

### 문제 4: 로그인 후에도 로그인 페이지로 계속 리디렉션

**원인**:
- 공유 메모리 또는 세마포어 문제
- 세션 생성 실패

**진단**:
```bash
# 서버 로그에서 다음 확인:
# 1. Session created 메시지가 있는가?
# 2. Valid session 메시지가 있는가?
# 3. PID가 다른가?
```

**해결**:
1. 서버 재시작 (Ctrl+C → ./ott_server)
2. 공유 메모리 정리:
```bash
ipcs -m | awk '/0x/ {print $2}' | xargs -I {} ipcrm -m {}
ipcs -s | grep ott_session_sem | awk '{print $2}' | xargs -I {} ipcrm -s {}
```

---

### 문제 5: 데이터베이스 권한 문제

**증상**: "attempt to write a readonly database"

**해결**:
```bash
chmod 666 database/ott.db
chmod 777 database/
```

---

## 📊 성공 기준 체크리스트

- [ ] 서버 컴파일 성공 (경고 무시 가능)
- [ ] 데이터베이스 초기화 성공
- [ ] users 테이블에 3명의 테스트 사용자 생성
- [ ] videos 테이블에 test_video.mp4 등록
- [ ] 로그인 폼에 비밀번호 필드 표시
- [ ] 올바른 비밀번호로 로그인 성공
- [ ] 잘못된 비밀번호로 로그인 실패 확인
- [ ] 로그인 후 player.html 정상 표시
- [ ] 페이지 새로고침 시 세션 유지
- [ ] 멀티 유저 동시 로그인 성공
- [ ] 데이터베이스에 last_login 업데이트 확인

---

## 🎯 테스트 계정 정보

모든 테스트 계정의 비밀번호: `password123`

| Username | User ID | 비밀번호 |
|----------|---------|----------|
| alice | 1 | password123 |
| bob | 2 | password123 |
| testuser | 3 | password123 |

---

## 🔧 유용한 명령어

### 데이터베이스 조회
```bash
# 모든 사용자 확인
sqlite3 database/ott.db "SELECT * FROM users;"

# 세션 정보 확인 (Phase 3에서 사용)
sqlite3 database/ott.db "SELECT * FROM watch_history;"

# 비디오 목록 확인
sqlite3 database/ott.db "SELECT * FROM videos;"

# 테이블 구조 확인
sqlite3 database/ott.db ".schema"
```

### 공유 메모리/세마포어 확인
```bash
# 공유 메모리 목록
ipcs -m

# 세마포어 목록
ipcs -s

# 정리
ipcs -m | awk '/0x/ {print $2}' | xargs -I {} ipcrm -m {}
ipcs -s | awk '{print $2}' | xargs -I {} ipcrm -s {}
```

### 서버 프로세스 확인
```bash
# 실행 중인 ott_server 확인
ps aux | grep ott_server

# 모든 프로세스 종료
pkill ott_server
```

---

## 📝 보고 형식

테스트 완료 후 다음과 같이 보고해 주세요:

```
✅ Phase 2 테스트 성공:
- 컴파일: OK
- 데이터베이스 초기화: OK
- 로그인 (올바른 비밀번호): OK
- 로그인 (잘못된 비밀번호): 실패 페이지 표시 OK
- 세션 유지: OK
- 멀티 유저: OK

또는

❌ Phase 2 테스트 실패:
- 문제: [구체적인 문제]
- 로그: [서버 로그 복사]
- 에러: [에러 메시지]
```

---

**작성일**: 2025-11-03
**Phase**: Enhancement Phase 2
**Status**: Ready for Testing 🚀
