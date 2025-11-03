# Enhancement Phase 1 - 세션 관리 시스템 설계

**작성일**: 2025-11-03
**목표**: 쿠키 기반 세션 관리 및 간단한 로그인 시스템 구현

---

## 🎯 기능 요구사항

### 1. 세션 관리
- UUID 형식의 세션 ID 생성
- 쿠키를 통한 세션 ID 전달
- 인메모리 세션 저장소 (해시맵 또는 배열)
- 세션 타임아웃: 30분 (1800초)
- 세션 자동 정리 메커니즘

### 2. 로그인 시스템
- ID만 입력 (비밀번호 없음 - MVP)
- 로그인 성공 시 세션 생성
- 세션 쿠키 발급
- player.html로 리다이렉트

### 3. 세션 검증
- 모든 비디오 요청에서 세션 확인
- 유효한 세션: 요청 처리
- 유효하지 않은 세션: login.html로 리다이렉트

---

## 📊 데이터 구조 설계

### Session 구조체
```c
#define SESSION_ID_LENGTH 37  // UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
#define USER_ID_LENGTH 64
#define MAX_SESSIONS 100

typedef struct {
    char session_id[SESSION_ID_LENGTH];
    char user_id[USER_ID_LENGTH];
    time_t created_at;
    time_t last_accessed;
    int is_active;
} Session;
```

### 세션 저장소
```c
// 간단한 배열 기반 세션 저장소
Session session_store[MAX_SESSIONS];
int session_count = 0;

// 또는 향후 확장을 위한 링크드 리스트
typedef struct SessionNode {
    Session session;
    struct SessionNode* next;
} SessionNode;
```

---

## 🔧 핵심 함수 설계

### 1. 세션 ID 생성
```c
/**
 * Generate UUID-like session ID
 * Format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
 */
void generate_session_id(char* session_id);
```

**구현 방법**:
- `time(NULL)` + `rand()` 조합
- 또는 `/dev/urandom` 읽기 (Linux)
- UUID v4 형식 사용

### 2. 세션 생성
```c
/**
 * Create new session for user
 * Returns: session_id on success, NULL on failure
 */
char* create_session(const char* user_id);
```

**처리 과정**:
1. 세션 ID 생성
2. Session 구조체 초기화
3. session_store에 추가
4. 세션 ID 반환

### 3. 세션 검증
```c
/**
 * Validate session by session_id
 * Returns: 1 if valid, 0 if invalid
 */
int validate_session(const char* session_id);
```

**검증 조건**:
- 세션 ID가 저장소에 존재
- is_active == 1
- 마지막 접근 시간이 30분 이내

### 4. 세션 갱신
```c
/**
 * Update last_accessed time for session
 */
void refresh_session(const char* session_id);
```

### 5. 세션 삭제
```c
/**
 * Invalidate session (logout)
 */
void destroy_session(const char* session_id);
```

### 6. 세션 정리
```c
/**
 * Remove expired sessions (timeout > 30 minutes)
 */
void cleanup_expired_sessions();
```

---

## 🍪 쿠키 처리

### 쿠키 파싱
```c
/**
 * Parse Cookie header and extract session_id
 * Cookie: session_id=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 */
char* parse_cookie(const char* cookie_header);
```

### 쿠키 생성
```c
/**
 * Generate Set-Cookie header
 * Set-Cookie: session_id=VALUE; Path=/; Max-Age=1800
 */
void generate_set_cookie_header(char* buffer, const char* session_id);
```

**쿠키 속성**:
- `session_id`: 세션 ID 값
- `Path=/`: 모든 경로에서 유효
- `Max-Age=1800`: 30분 (1800초)
- `HttpOnly`: JavaScript 접근 방지 (선택)

---

## 🌐 HTTP 요청/응답 처리

### 로그인 POST 요청 처리
```c
/**
 * Handle POST /login
 * Request body: user_id=USERNAME
 */
void handle_login(int client_fd, const char* request_body);
```

**처리 과정**:
1. POST body에서 user_id 파싱
2. user_id 검증 (빈 문자열 아님)
3. 세션 생성
4. Set-Cookie 헤더 포함 302 리다이렉트 응답
5. Location: /

**응답 예시**:
```http
HTTP/1.1 302 Found
Location: /
Set-Cookie: session_id=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx; Path=/; Max-Age=1800
Content-Length: 0

```

### 세션 검증 미들웨어
```c
/**
 * Check if request has valid session
 * Returns: 1 if valid, 0 if invalid
 */
int check_session_middleware(const char* request);
```

**처리 과정**:
1. Cookie 헤더 파싱
2. session_id 추출
3. validate_session() 호출
4. 유효하면 1 반환, 무효하면 0 반환

### 로그인 페이지 리다이렉트
```c
/**
 * Send 302 redirect to /login
 */
void redirect_to_login(int client_fd);
```

**응답**:
```http
HTTP/1.1 302 Found
Location: /login
Content-Length: 0

```

---

## 📄 파일 구조 변경

### 새로운 파일 추가
```
server/
├── src/
│   ├── main.c           (수정: 세션 미들웨어 추가)
│   ├── http.c           (수정: POST 처리 추가)
│   ├── streaming.c      (유지)
│   └── session.c        ⭐ 새로 추가
├── include/
│   └── server.h         (수정: 세션 함수 선언 추가)
└── Makefile             (수정: session.c 컴파일 추가)

client/
├── player.html          (유지)
└── login.html           ⭐ 새로 추가
```

---

## 🔄 요청 처리 플로우

### 로그인 플로우
```
사용자 → GET /login → 서버
                      ↓
                  login.html 반환
                      ↓
사용자 입력 (ID)
                      ↓
사용자 → POST /login (user_id=john)
                      ↓
                  세션 생성
                      ↓
                  Set-Cookie 응답
                      ↓
                  302 Redirect → /
                      ↓
사용자 → GET / (with Cookie)
                      ↓
                  세션 검증 ✅
                      ↓
                  player.html 반환
```

### 비디오 요청 플로우
```
사용자 → GET /videos/test.mp4 (with Cookie)
                      ↓
                  Cookie 파싱
                      ↓
                  세션 검증
                      ↓
          ┌──────────┴──────────┐
        유효 ✅              무효 ❌
          ↓                     ↓
    비디오 스트리밍      302 Redirect → /login
```

---

## 🎨 UI 설계

### login.html
```html
<!DOCTYPE html>
<html>
<head>
    <title>OTT Login</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        .login-container {
            background: white;
            padding: 40px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            width: 300px;
        }
        h1 {
            text-align: center;
            color: #333;
            margin-bottom: 30px;
        }
        input {
            width: 100%;
            padding: 12px;
            margin: 10px 0;
            border: 1px solid #ddd;
            border-radius: 5px;
            box-sizing: border-box;
        }
        button {
            width: 100%;
            padding: 12px;
            background: #667eea;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover {
            background: #5568d3;
        }
    </style>
</head>
<body>
    <div class="login-container">
        <h1>🎬 OTT Streaming</h1>
        <form method="POST" action="/login">
            <input type="text" name="user_id" placeholder="사용자 ID" required>
            <button type="submit">로그인</button>
        </form>
    </div>
</body>
</html>
```

---

## 🧪 테스트 계획

### 테스트 케이스

#### 1. 로그인 성공
```bash
# 1. 로그인 페이지 접속
curl -i http://localhost:8080/login

# 2. 로그인 POST
curl -i -X POST http://localhost:8080/login \
  -d "user_id=testuser"

# 예상: 302 Redirect + Set-Cookie 헤더
```

#### 2. 세션 검증 성공
```bash
# 1. 로그인으로 세션 획득
SESSION_ID=$(curl -i -X POST http://localhost:8080/login \
  -d "user_id=testuser" 2>&1 | grep -i "set-cookie" | \
  sed 's/.*session_id=\([^;]*\).*/\1/')

# 2. 세션으로 플레이어 접속
curl -i http://localhost:8080/ \
  -H "Cookie: session_id=$SESSION_ID"

# 예상: 200 OK + player.html
```

#### 3. 세션 없이 비디오 요청
```bash
# 세션 없이 비디오 요청
curl -i http://localhost:8080/videos/test_video.mp4

# 예상: 302 Redirect → /login
```

#### 4. 세션 타임아웃
```bash
# 1. 로그인
# 2. 30분 대기 (또는 서버에서 타임아웃 시간 1분으로 테스트)
# 3. 비디오 요청
# 예상: 302 Redirect → /login
```

#### 5. 동시 다중 사용자 세션
```bash
# 3명의 사용자가 각각 로그인
# 각각 다른 세션 ID 확인
# 모두 독립적으로 비디오 재생 가능
```

---

## 📊 구현 순서

### Phase 1.1: 세션 관리 코어 (우선순위 높음)
1. ✅ session.c 생성
2. ✅ 세션 데이터 구조 정의
3. ✅ 세션 ID 생성 함수
4. ✅ 세션 CRUD 함수 (Create, Read, Update, Delete)
5. ✅ 세션 만료 정리 함수

### Phase 1.2: HTTP 처리 확장
1. ✅ POST 요청 파싱 함수
2. ✅ Cookie 헤더 파싱 함수
3. ✅ Set-Cookie 헤더 생성 함수
4. ✅ 302 Redirect 응답 함수

### Phase 1.3: 로그인 시스템
1. ✅ login.html 생성
2. ✅ POST /login 핸들러 구현
3. ✅ 세션 생성 및 쿠키 발급

### Phase 1.4: 세션 미들웨어
1. ✅ main.c에 세션 검증 로직 추가
2. ✅ 유효하지 않은 세션 리다이렉트

### Phase 1.5: 테스트 및 검증
1. ✅ 수동 테스트 (브라우저)
2. ✅ curl 기반 자동 테스트
3. ✅ 다중 사용자 동시 세션 테스트

---

## 🔒 보안 고려사항 (향후)

현재 MVP에서는 구현하지 않지만, 나중에 추가할 사항:

1. **HTTPS**: 쿠키를 암호화된 연결로 전송
2. **CSRF 방지**: CSRF 토큰 추가
3. **세션 고정 공격 방지**: 로그인 시 세션 ID 재생성
4. **Secure 쿠키**: `Secure` 속성 추가 (HTTPS 전용)
5. **HttpOnly 쿠키**: JavaScript 접근 방지
6. **Rate Limiting**: 로그인 시도 제한

---

## 📝 Makefile 수정

```makefile
# session.c 추가
SOURCES = src/main.c src/http.c src/streaming.c src/session.c
OBJECTS = $(SOURCES:.c=.o)

# 나머지는 동일
```

---

## ✅ 완료 기준

Enhancement Phase 1 완료 조건:

- [ ] session.c 구현 완료
- [ ] login.html 생성
- [ ] POST /login 처리 구현
- [ ] 세션 쿠키 발급 작동
- [ ] 세션 검증 미들웨어 작동
- [ ] 유효하지 않은 세션 리다이렉트 작동
- [ ] 다중 사용자 독립 세션 확인
- [ ] 세션 타임아웃 작동

---

**다음 단계**: session.c 구현부터 시작

**예상 소요 시간**: 2-3시간

**작성일**: 2025-11-03
