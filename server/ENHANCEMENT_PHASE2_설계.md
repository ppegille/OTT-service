# Enhancement Phase 2 설계 문서

**작성일**: 2025-11-03
**Phase**: Enhancement Phase 2 - SQLite Database Integration
**목표**: 사용자 인증 시스템 및 비디오 메타데이터 관리

---

## 📋 Phase 2 목표

### 핵심 기능
1. **SQLite 데이터베이스 통합**
   - users 테이블: 사용자 정보 및 비밀번호
   - videos 테이블: 비디오 메타데이터

2. **실제 사용자 인증**
   - 비밀번호 해싱 (SHA-256)
   - 로그인 폼에 비밀번호 필드 추가
   - DB 기반 인증 검증

3. **비디오 메타데이터 관리**
   - 비디오 파일 스캔 및 DB 등록
   - 제목, 파일명, 썸네일 경로 저장
   - 비디오 목록 API 제공

---

## 🗄️ 데이터베이스 스키마

### 1. users 테이블
```sql
CREATE TABLE IF NOT EXISTS users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_login DATETIME
);
```

**필드 설명:**
- `user_id`: 자동 증가 Primary Key
- `username`: 고유한 사용자 이름
- `password_hash`: SHA-256 해시된 비밀번호
- `created_at`: 계정 생성 시각
- `last_login`: 마지막 로그인 시각

**초기 데이터:**
```sql
-- 테스트용 사용자 (비밀번호: password123)
INSERT INTO users (username, password_hash) VALUES
    ('alice', 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f'),
    ('bob', 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f'),
    ('testuser', 'ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f');
```

### 2. videos 테이블
```sql
CREATE TABLE IF NOT EXISTS videos (
    video_id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    filename TEXT UNIQUE NOT NULL,
    thumbnail_path TEXT,
    duration INTEGER,  -- 초 단위
    file_size INTEGER,  -- 바이트 단위
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

**필드 설명:**
- `video_id`: 자동 증가 Primary Key
- `title`: 비디오 제목 (파일명에서 추출)
- `filename`: videos/ 디렉토리의 실제 파일명
- `thumbnail_path`: 썸네일 이미지 경로 (Phase 3에서 FFmpeg로 생성)
- `duration`: 비디오 길이 (초)
- `file_size`: 파일 크기 (Range Request 계산용)
- `created_at`: DB 등록 시각

**초기 데이터:**
```sql
INSERT INTO videos (title, filename, file_size) VALUES
    ('Test Video', 'test_video.mp4', 193600);
```

### 3. watch_history 테이블 (Phase 3)
```sql
CREATE TABLE IF NOT EXISTS watch_history (
    history_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    video_id INTEGER NOT NULL,
    last_position INTEGER DEFAULT 0,  -- 초 단위
    last_watched DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(user_id) REFERENCES users(user_id),
    FOREIGN KEY(video_id) REFERENCES videos(video_id)
);
```

---

## 🔧 구현 계획

### Step 1: 데이터베이스 모듈 생성

**파일**: `src/database.c`, `include/database.h`

**주요 함수:**
```c
// 데이터베이스 초기화
int init_database(const char* db_path);

// 데이터베이스 종료
void close_database();

// 사용자 인증
int authenticate_user(const char* username, const char* password, int* user_id);

// 사용자 생성 (관리자 기능)
int create_user(const char* username, const char* password);

// 비디오 목록 조회
int get_videos(char* json_output, size_t max_len);

// 비디오 정보 조회
int get_video_by_id(int video_id, char* json_output, size_t max_len);

// 비디오 등록 (스캔 시)
int register_video(const char* filename, const char* title, long file_size);
```

### Step 2: SHA-256 비밀번호 해싱

**파일**: `src/crypto.c`, `include/crypto.h`

**주요 함수:**
```c
// SHA-256 해싱
void sha256_hash(const char* input, char* output_hex);

// 비밀번호 검증
int verify_password(const char* input_password, const char* stored_hash);
```

**구현 방법:**
- OpenSSL 라이브러리 사용: `#include <openssl/sha.h>`
- Makefile에 `-lcrypto` 추가

### Step 3: 로그인 페이지 업데이트

**파일**: `client/login.html`

**변경 사항:**
```html
<form method="POST" action="/login" id="loginForm">
    <input type="text" name="username" placeholder="사용자 이름" required>
    <input type="password" name="password" placeholder="비밀번호" required>
    <button type="submit">로그인</button>
</form>
```

### Step 4: 세션 관리 업데이트

**파일**: `src/session.c`

**변경 사항:**
```c
// 기존: create_session(user_id)
// 변경: create_session(user_id, username)

typedef struct {
    char session_id[SESSION_ID_LENGTH];
    int user_id;           // ← INT로 변경 (DB Primary Key)
    char username[64];     // ← 추가
    time_t created_at;
    time_t last_accessed;
    int is_active;
} Session;
```

### Step 5: 로그인 핸들러 수정

**파일**: `src/session.c` - `handle_login()`

**기존 로직:**
```c
void handle_login(int client_fd, const char* request_body) {
    char* user_id = parse_post_body(request_body, "user_id");

    if (user_id && strlen(user_id) > 0) {
        char* session_id = create_session(user_id);
        // ...
    }
}
```

**새로운 로직:**
```c
void handle_login(int client_fd, const char* request_body) {
    char* username = parse_post_body(request_body, "username");
    char* password = parse_post_body(request_body, "password");

    int user_id;
    if (authenticate_user(username, password, &user_id)) {
        // 인증 성공
        char* session_id = create_session(user_id, username);
        update_last_login(user_id);  // DB 업데이트
        // Set-Cookie 및 302 리디렉션
    } else {
        // 인증 실패
        send_login_error(client_fd, "Invalid username or password");
    }
}
```

### Step 6: 비디오 목록 API

**파일**: `src/main.c`

**새로운 엔드포인트:**
```c
// GET /api/videos - 비디오 목록 JSON 반환
if (strcmp(req.method, "GET") == 0 && strcmp(req.path, "/api/videos") == 0) {
    char json_output[8192];
    if (get_videos(json_output, sizeof(json_output)) == 0) {
        send_json_response(client_fd, json_output);
    } else {
        send_500_error(client_fd);
    }
}
```

**JSON 응답 예시:**
```json
{
    "videos": [
        {
            "video_id": 1,
            "title": "Test Video",
            "filename": "test_video.mp4",
            "thumbnail": "/thumbnails/test_video.jpg",
            "duration": 120,
            "file_size": 193600
        }
    ]
}
```

---

## 📂 파일 구조 변경

### 추가될 파일
```
server/
├── src/
│   ├── database.c      ⭐ 새로 추가
│   ├── crypto.c        ⭐ 새로 추가
│   └── ...
├── include/
│   ├── database.h      ⭐ 새로 추가
│   ├── crypto.h        ⭐ 새로 추가
│   └── server.h        (수정)
├── database/
│   ├── schema.sql      ⭐ 새로 추가 (스키마 정의)
│   ├── seed.sql        ⭐ 새로 추가 (초기 데이터)
│   └── ott.db          (실행 시 자동 생성)
└── Makefile            (수정: -lsqlite3 -lcrypto 추가)
```

### Makefile 변경
```makefile
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = -lpthread -lsqlite3 -lcrypto

SRCS = src/main.c src/http.c src/streaming.c src/session.c \
       src/database.c src/crypto.c

OBJS = $(SRCS:.c=.o)
TARGET = ott_server
```

---

## 🔒 보안 고려사항

### 1. SQL Injection 방지
```c
// ❌ 위험: 직접 문자열 조합
sprintf(sql, "SELECT * FROM users WHERE username='%s'", username);

// ✅ 안전: Prepared Statement 사용
sqlite3_stmt* stmt;
sqlite3_prepare_v2(db, "SELECT * FROM users WHERE username = ?", -1, &stmt, NULL);
sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
```

### 2. 비밀번호 해싱
```c
// SHA-256 해싱
void sha256_hash(const char* input, char* output_hex) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash);

    // Hex 문자열 변환
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output_hex + (i * 2), "%02x", hash[i]);
    }
    output_hex[64] = '\0';
}
```

### 3. 세션 보안
- HttpOnly 쿠키 플래그 사용
- SameSite 속성 설정 (CSRF 방지)
- 세션 타임아웃 유지 (30분)

---

## 🧪 테스트 시나리오

### Test 1: 데이터베이스 초기화
```bash
./ott_server
# 로그 확인:
# ✓ Database initialized: database/ott.db
# ✓ Created 3 tables: users, videos, watch_history
# ✓ Inserted 3 test users
```

### Test 2: 로그인 성공
```
1. 브라우저: http://localhost:8080/
2. Username: alice
3. Password: password123
4. 예상: 로그인 성공 → player.html
```

### Test 3: 로그인 실패
```
1. Username: alice
2. Password: wrongpassword
3. 예상: "Invalid username or password" 메시지
```

### Test 4: 비디오 목록 API
```bash
curl http://localhost:8080/api/videos
# 예상 응답:
# {"videos":[{"video_id":1,"title":"Test Video",...}]}
```

### Test 5: 멀티 유저 세션
```
1. 탭 1: alice로 로그인
2. 탭 2: bob으로 로그인
3. 각 탭에서 세션 유지 확인
4. DB에서 user_id가 다른지 확인
```

---

## 📊 구현 단계별 예상 시간

| 단계 | 작업 | 예상 시간 |
|------|------|-----------|
| 1 | 데이터베이스 스키마 및 초기화 | 30분 |
| 2 | database.c 기본 함수 구현 | 1시간 |
| 3 | crypto.c SHA-256 해싱 | 30분 |
| 4 | login.html 비밀번호 필드 추가 | 10분 |
| 5 | handle_login DB 인증 통합 | 30분 |
| 6 | 세션 구조체 수정 (user_id INT) | 20분 |
| 7 | 비디오 목록 API 구현 | 40분 |
| 8 | 컴파일 및 테스트 | 30분 |
| **합계** | | **약 4시간** |

---

## 🎯 완료 조건

- [ ] SQLite 데이터베이스 초기화 성공
- [ ] users 테이블에 테스트 사용자 3명 생성
- [ ] videos 테이블에 test_video.mp4 등록
- [ ] 로그인 폼에 비밀번호 필드 추가
- [ ] SHA-256 비밀번호 해싱 구현
- [ ] DB 기반 사용자 인증 성공
- [ ] 잘못된 비밀번호로 로그인 실패 확인
- [ ] 세션에 user_id (INT) 저장
- [ ] /api/videos 엔드포인트 구현
- [ ] 멀티 유저 동시 로그인 테스트

---

## 🚀 다음 단계 (Phase 3)

Phase 2 완료 후 구현할 기능:
1. FFmpeg 썸네일 생성
2. 비디오 목록 페이지 (gallery.html)
3. 시청 기록 저장
4. 이어보기 기능
5. 사용자 지정 재생 위치

---

**작성일**: 2025-11-03
**Status**: Ready to Implement 🚀
