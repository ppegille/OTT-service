# Enhancement Phase 1 - 실행 가이드

**작성일**: 2025-11-03
**목표**: 세션 관리 및 로그인 시스템 테스트

---

## 🎯 Phase 1에서 추가된 기능

### ✅ 새로운 기능
- [x] 쿠키 기반 세션 관리
- [x] UUID 형식 세션 ID 생성
- [x] 로그인 페이지 (ID만 입력)
- [x] 세션 검증 미들웨어
- [x] 세션 타임아웃 (30분)
- [x] 자동 세션 정리

### 🔄 변경 사항
- **인증 시스템**: 없음 → 세션 기반 로그인
- **접근 제어**: 누구나 → 로그인 필수
- **세션 관리**: 없음 → 쿠키 기반 세션
- **첫 페이지**: player.html → login.html

---

## 🚀 빠른 시작 (4단계)

### 1단계: 서버 재컴파일

**WSL2 터미널에서**:

```bash
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트
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
Compiling src/session.c...        ← 새로 추가!
Linking ott_server...
✓ Build complete!
Run with: ./ott_server
```

✅ **session.c**가 컴파일되면 성공!

### 2단계: 서버 실행

```bash
./ott_server
```

**예상 출력:**
```
=== OTT Streaming Server - Enhancement Phase 1 ===
    (Session Management & User Authentication)

✓ Session store initialized (max: 100 sessions)

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

**중요 확인 사항**:
- ✅ "Enhancement Phase 1" 표시
- ✅ "Session Management & User Authentication" 표시
- ✅ "Session store initialized" 표시

### 3단계: 브라우저 테스트

#### 3-1. 첫 접속 (세션 없음 → 로그인 페이지)

1. **Chrome 브라우저 열기**
2. **주소창**: `http://localhost:8080/`
3. **결과**: 자동으로 `/login`으로 리다이렉트

**서버 로그 확인:**
```
✓ Client connected: 127.0.0.1 (PID: 1234)
  [Child 1235] GET /
  [Child 1235] No valid session, redirecting to login
  [Child 1235] Connection closed
```

#### 3-2. 로그인 페이지

**확인 사항**:
- ✅ 보라색 그라디언트 배경
- ✅ "🎬 OTT" 로고
- ✅ "사용자 ID" 입력창
- ✅ "로그인" 버튼

#### 3-3. 로그인 시도

1. **사용자 ID 입력**: `testuser` (아무 ID나 가능)
2. **로그인 버튼 클릭**
3. **결과**: 플레이어 페이지로 이동

**서버 로그 확인:**
```
✓ Client connected: 127.0.0.1 (PID: 1234)
  [Child 1236] POST /login
✓ Session created: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx for user 'testuser' (total: 1)
✓ Login successful: testuser → session xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  [Child 1236] Connection closed
```

#### 3-4. 로그인 후 플레이어 접속

**자동으로 `/`로 리다이렉트됨**

**서버 로그 확인:**
```
✓ Client connected: 127.0.0.1 (PID: 1234)
  [Child 1237] GET /
  [Child 1237] Valid session: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  [Child 1237] Connection closed
```

#### 3-5. 비디오 재생

1. **비디오 재생 버튼 클릭**
2. **결과**: 비디오 정상 재생

**서버 로그 확인:**
```
✓ Client connected: 127.0.0.1 (PID: 1234)
  [Child 1238] GET /videos/test_video.mp4
  [Child 1238] Valid session: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  [Child 1238] Connection closed
```

### 4단계: 다중 사용자 테스트

#### 4-1. 시크릿 창에서 다른 사용자 로그인

1. **Chrome 시크릿 창 열기** (Ctrl+Shift+N)
2. `http://localhost:8080/` 접속
3. **로그인**: 다른 사용자 ID (예: `user2`)
4. **비디오 재생**

**서버 로그 확인:**
```
✓ Session created: yyyyyyyy-yyyy-yyyy-yyyy-yyyyyyyyyyyy for user 'user2' (total: 2)
✓ Login successful: user2 → session yyyyyyyy-yyyy-yyyy-yyyy-yyyyyyyyyyyy
```

#### 4-2. 여러 탭에서 동일 사용자

1. **일반 Chrome 탭** 3개 열기
2. 모두 `http://localhost:8080/` 접속
3. **결과**: 모두 같은 세션으로 로그인됨 (쿠키 공유)

**서버 로그 확인:**
```
  [Child 1239] Valid session: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  [Child 1240] Valid session: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  [Child 1241] Valid session: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
```

**모두 같은 session_id 사용!**

---

## 🧪 고급 테스트

### 테스트 1: 쿠키 확인 (Chrome DevTools)

1. **F12** 눌러 개발자 도구 열기
2. **Application** 탭 클릭
3. **Cookies** → `http://localhost:8080` 선택
4. **확인**: `session_id` 쿠키 존재

**쿠키 속성 확인**:
- Name: `session_id`
- Value: `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`
- Path: `/`
- Max-Age: `1800` (30분)
- HttpOnly: `true`

### 테스트 2: 세션 타임아웃 (간단 버전)

**서버 설정 변경** (테스트용):

`server/include/server.h`:
```c
#define SESSION_TIMEOUT 60  // 1분으로 변경 (테스트용)
```

**테스트 절차**:
1. 서버 재컴파일 및 재시작
2. 로그인
3. **1분 대기**
4. 비디오 재생 시도
5. **결과**: 로그인 페이지로 리다이렉트

**서버 로그**:
```
⏰ Session expired: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx (user: testuser)
  [Child 1250] No valid session, redirecting to login
```

### 테스트 3: curl로 세션 흐름 확인

#### Step 1: 로그인 페이지 접속
```bash
curl -i http://localhost:8080/login
```

**예상 응답**:
```http
HTTP/1.1 200 OK
Content-Type: text/html
...
[login.html 내용]
```

#### Step 2: 로그인 (세션 획득)
```bash
curl -i -X POST http://localhost:8080/login \
  -d "user_id=testuser"
```

**예상 응답**:
```http
HTTP/1.1 302 Found
Location: /
Set-Cookie: session_id=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx; Path=/; Max-Age=1800; HttpOnly
Content-Length: 0
```

**세션 ID 추출**:
```bash
SESSION_ID=$(curl -i -X POST http://localhost:8080/login \
  -d "user_id=testuser" 2>&1 | \
  grep -i "set-cookie" | \
  sed 's/.*session_id=\([^;]*\).*/\1/')

echo "Session ID: $SESSION_ID"
```

#### Step 3: 세션으로 플레이어 접속
```bash
curl -i http://localhost:8080/ \
  -H "Cookie: session_id=$SESSION_ID"
```

**예상 응답**:
```http
HTTP/1.1 200 OK
Content-Type: text/html
...
[player.html 내용]
```

#### Step 4: 세션 없이 비디오 요청
```bash
curl -i http://localhost:8080/videos/test_video.mp4
```

**예상 응답**:
```http
HTTP/1.1 302 Found
Location: /login
Content-Length: 0
```

#### Step 5: 세션으로 비디오 요청
```bash
curl -i http://localhost:8080/videos/test_video.mp4 \
  -H "Cookie: session_id=$SESSION_ID" \
  -H "Range: bytes=0-1023"
```

**예상 응답**:
```http
HTTP/1.1 206 Partial Content
Content-Type: video/mp4
Content-Range: bytes 0-1023/FILESIZE
...
[비디오 데이터]
```

---

## ✅ Phase 1 완료 기준

다음을 모두 확인하면 Enhancement Phase 1 완료:

- [ ] 서버에 "Enhancement Phase 1" 표시
- [ ] "Session store initialized" 표시
- [ ] session.c 컴파일 성공
- [ ] 첫 접속 시 로그인 페이지로 리다이렉트
- [ ] 로그인 폼에서 ID 입력 및 제출
- [ ] 로그인 성공 시 세션 쿠키 발급
- [ ] 로그인 후 플레이어 페이지 접속
- [ ] 세션 있을 때 비디오 재생 가능
- [ ] 세션 없을 때 로그인 페이지로 리다이렉트
- [ ] 다중 사용자 각각 다른 세션 ID
- [ ] 쿠키에 session_id 저장 확인
- [ ] 세션 타임아웃 작동 (선택)

---

## 🐛 문제 해결

### 문제 1: 컴파일 에러
```
undefined reference to 'create_session'
```

**원인**: Makefile에 session.c 추가 안됨
**해결**:
```bash
# Makefile 확인
cat Makefile | grep session.c
# 출력: SRCS = src/main.c src/http.c src/streaming.c src/session.c

# 없으면 추가
```

### 문제 2: 로그인 페이지 404
```
404 Not Found
```

**원인**: login.html 파일 없음
**해결**:
```bash
ls -la ../client/login.html
# 파일이 있어야 함
```

### 문제 3: 로그인 후에도 계속 리다이렉트

**원인**: 쿠키가 설정되지 않음
**해결**:
1. Chrome DevTools → Application → Cookies 확인
2. session_id 쿠키가 있는지 확인
3. 서버 로그에서 "Set-Cookie" 응답 확인

### 문제 4: 세션이 바로 만료됨

**원인**: 시간 설정 문제
**해결**:
```c
// server/include/server.h 확인
#define SESSION_TIMEOUT 1800  // 30분이어야 함 (1800초)
```

---

## 📊 서버 로그 해석

### 정상 로그인 플로우
```
1. ✓ Client connected: 127.0.0.1 (PID: 1234)
2.   [Child 1235] GET /
3.   [Child 1235] No valid session, redirecting to login    ← 세션 없음
4.   [Child 1235] Connection closed

5. ✓ Client connected: 127.0.0.1 (PID: 1234)
6.   [Child 1236] GET /login                                ← 로그인 페이지
7.   [Child 1236] Connection closed

8. ✓ Client connected: 127.0.0.1 (PID: 1234)
9.   [Child 1237] POST /login                               ← 로그인 제출
10. ✓ Session created: xxxx... for user 'testuser' (total: 1)
11. ✓ Login successful: testuser → session xxxx...
12.   [Child 1237] Connection closed

13. ✓ Client connected: 127.0.0.1 (PID: 1234)
14.   [Child 1238] GET /                                     ← 리다이렉트 후
15.   [Child 1238] Valid session: xxxx...                    ← 세션 유효!
16.   [Child 1238] Connection closed
```

### 세션 만료 로그
```
⏰ Session expired: xxxx... (user: testuser)
  [Child 1250] No valid session, redirecting to login
```

### 다중 사용자 로그
```
✓ Session created: xxxx-1... for user 'user1' (total: 1)
✓ Session created: xxxx-2... for user 'user2' (total: 2)
✓ Session created: xxxx-3... for user 'user3' (total: 3)
```

---

## 🎯 다음 단계

Enhancement Phase 1 완료 후:

### Enhancement Phase 2: 고급 기능
1. **SQLite 데이터베이스 통합**
   - users, sessions, videos, watch_history 테이블
   - 데이터베이스 기반 인증

2. **비디오 목록 페이지**
   - 서버의 모든 비디오 표시
   - 썸네일 이미지

3. **시청 기록 관리**
   - 시청 위치 저장
   - 이어보기 기능

4. **FFmpeg 썸네일 생성**
   - 비디오 업로드 시 자동 생성

---

## 💡 핵심 명령어

```bash
# 컴파일
cd server && make clean && make

# 실행
./ott_server

# 브라우저 테스트
http://localhost:8080/

# curl 테스트 (세션 획득)
curl -i -X POST http://localhost:8080/login -d "user_id=testuser"

# curl 테스트 (세션으로 접속)
curl -i http://localhost:8080/ -H "Cookie: session_id=YOUR_SESSION_ID"

# 서버 종료
Ctrl + C
# 또는
pkill ott_server
```

---

**작성일**: 2025-11-03
**Phase**: Enhancement Phase 1
**다음 단계**: Database Integration (Enhancement Phase 2)

**잘 작동하길 바랍니다! 🚀**
