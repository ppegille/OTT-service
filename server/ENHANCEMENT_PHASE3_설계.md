# Enhancement Phase 3 설계 문서

**작성일**: 2025-11-03
**Phase**: Enhancement Phase 3 - Video Gallery & Watch History
**목표**: 비디오 목록 페이지, 썸네일 생성, 시청 기록 관리

---

## 📋 Phase 3 목표

### 핵심 기능
1. **비디오 목록 페이지 (Gallery)**
   - 모든 비디오를 카드 형식으로 표시
   - 썸네일, 제목, 재생 시간 표시
   - 클릭 시 비디오 재생

2. **FFmpeg 썸네일 생성**
   - 비디오에서 자동으로 썸네일 추출
   - thumbnails/ 디렉토리에 저장
   - 서버 시작 시 자동 생성

3. **시청 기록 관리**
   - 사용자별 시청 위치 저장
   - watch_history 테이블 활용
   - 재생 중 주기적으로 위치 업데이트

4. **이어보기 기능**
   - 이전에 본 비디오는 마지막 위치부터 재생
   - "처음부터" / "이어보기" 선택 다이얼로그

---

## 🎨 UI/UX 설계

### 1. Gallery 페이지 (gallery.html)

**레이아웃**:
```
┌─────────────────────────────────────────┐
│  🎬 OTT     [사용자: alice] [로그아웃]   │
├─────────────────────────────────────────┤
│                                         │
│  ┌────────┐  ┌────────┐  ┌────────┐   │
│  │ [썸네일] │  │ [썸네일] │  │ [썸네일] │   │
│  │        │  │        │  │        │   │
│  │ 제목1   │  │ 제목2   │  │ 제목3   │   │
│  │ 10:25  │  │ 05:43  │  │ 15:00  │   │
│  └────────┘  └────────┘  └────────┘   │
│                                         │
│  ┌────────┐  ┌────────┐  ┌────────┐   │
│  │ [썸네일] │  │ [썸네일] │  │ [썸네일] │   │
│  ...                                   │
└─────────────────────────────────────────┘
```

**비디오 카드 구성**:
- 썸네일 이미지 (16:9 비율)
- 비디오 제목
- 재생 시간
- 이어보기 배지 (시청 기록이 있는 경우)

### 2. 이어보기 다이얼로그

```
┌────────────────────────────┐
│  비디오 재생                │
├────────────────────────────┤
│                            │
│  마지막 시청 위치:          │
│  05:23                     │
│                            │
│  [처음부터]  [이어보기]     │
│                            │
└────────────────────────────┘
```

---

## 🗄️ 데이터베이스 활용

### watch_history 테이블 (이미 생성됨)
```sql
CREATE TABLE watch_history (
    history_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    video_id INTEGER NOT NULL,
    last_position INTEGER DEFAULT 0,  -- 초 단위
    last_watched DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(user_id) REFERENCES users(user_id),
    FOREIGN KEY(video_id) REFERENCES videos(video_id),
    UNIQUE(user_id, video_id)
);
```

### videos 테이블 업데이트
- `thumbnail_path` 컬럼에 썸네일 경로 저장
- `duration` 컬럼에 비디오 길이 저장 (FFmpeg로 추출)

---

## 🔧 구현 계획

### Step 1: 비디오 목록 API

**엔드포인트**: `GET /api/videos`

**기능**:
- 데이터베이스에서 모든 비디오 조회
- JSON 형식으로 반환
- 사용자의 시청 기록 포함

**JSON 응답 예시**:
```json
{
  "videos": [
    {
      "video_id": 1,
      "title": "Test Video",
      "filename": "test_video.mp4",
      "thumbnail": "/thumbnails/test_video.jpg",
      "duration": 125,
      "file_size": 193600,
      "watched": true,
      "last_position": 45
    }
  ]
}
```

**구현 위치**: `src/main.c`

```c
// GET /api/videos
if (strcmp(req.method, "GET") == 0 && strcmp(req.path, "/api/videos") == 0) {
    char json_output[8192];

    // 세션에서 user_id 가져오기
    int user_id = get_user_id_from_session(session_id);

    if (get_videos_with_history(user_id, json_output, sizeof(json_output)) == 0) {
        send_json_response(client_fd, json_output);
    } else {
        send_500_error(client_fd);
    }

    close(client_fd);
    exit(0);
}
```

### Step 2: Gallery 페이지 생성

**파일**: `client/gallery.html`

**주요 기능**:
1. `/api/videos`에서 비디오 목록 가져오기
2. 각 비디오를 카드 형식으로 표시
3. 카드 클릭 시 player 페이지로 이동
4. 시청 기록이 있으면 "이어보기" 배지 표시

**JavaScript 구조**:
```javascript
async function loadVideos() {
    const response = await fetch('/api/videos');
    const data = await response.json();

    const gallery = document.getElementById('video-gallery');

    data.videos.forEach(video => {
        const card = createVideoCard(video);
        gallery.appendChild(card);
    });
}

function createVideoCard(video) {
    // 비디오 카드 HTML 생성
    // 썸네일, 제목, 재생 시간 표시
    // 클릭 이벤트 추가
}
```

### Step 3: FFmpeg 썸네일 생성

**파일**: `src/thumbnail.c`, `include/thumbnail.h`

**주요 함수**:
```c
// 비디오에서 썸네일 추출
int generate_thumbnail(const char* video_path, const char* thumbnail_path);

// 모든 비디오의 썸네일 생성
int generate_all_thumbnails();

// 비디오 길이 추출
int get_video_duration(const char* video_path);
```

**FFmpeg 명령어**:
```bash
# 썸네일 추출 (5초 지점에서)
ffmpeg -i video.mp4 -ss 00:00:05 -vframes 1 -vf scale=320:-1 thumbnail.jpg

# 비디오 길이 추출
ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 video.mp4
```

**구현 방법**:
```c
int generate_thumbnail(const char* video_path, const char* thumbnail_path) {
    char cmd[1024];

    // FFmpeg 명령어 생성
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -i '%s' -ss 00:00:05 -vframes 1 -vf scale=320:-1 '%s' -y 2>/dev/null",
             video_path, thumbnail_path);

    // 시스템 명령어 실행
    int ret = system(cmd);

    return (ret == 0) ? 0 : -1;
}
```

### Step 4: 시청 위치 저장

**엔드포인트**: `POST /api/watch-progress`

**요청 Body**:
```json
{
  "video_id": 1,
  "position": 125
}
```

**기능**:
- 사용자의 시청 위치를 데이터베이스에 저장
- watch_history 테이블 업데이트
- JavaScript에서 주기적으로 호출 (10초마다)

**구현**:
```c
// Handle POST /api/watch-progress
void handle_watch_progress(int client_fd, const char* request_body, int user_id) {
    // Parse video_id and position from JSON
    int video_id = parse_json_int(request_body, "video_id");
    int position = parse_json_int(request_body, "position");

    // Update database
    if (update_watch_position(user_id, video_id, position) == 0) {
        send_json_response(client_fd, "{\"status\":\"success\"}");
    } else {
        send_json_response(client_fd, "{\"status\":\"error\"}");
    }
}
```

### Step 5: Player 페이지 업데이트

**파일**: `client/player.html`

**추가 기능**:
1. URL에서 video_id 파라미터 읽기
2. 시청 기록 확인
3. 이어보기 다이얼로그 표시
4. 재생 중 주기적으로 위치 저장

**JavaScript 추가**:
```javascript
// URL에서 video_id 가져오기
const urlParams = new URLSearchParams(window.location.search);
const videoId = urlParams.get('video_id');

// 시청 기록 확인
async function checkWatchHistory(videoId) {
    const response = await fetch(`/api/watch-history/${videoId}`);
    const data = await response.json();

    if (data.last_position > 0) {
        showResumeDialog(data.last_position);
    }
}

// 재생 위치 저장 (10초마다)
setInterval(() => {
    const position = Math.floor(video.currentTime);
    saveWatchProgress(videoId, position);
}, 10000);

async function saveWatchProgress(videoId, position) {
    await fetch('/api/watch-progress', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({video_id: videoId, position: position})
    });
}
```

---

## 📁 파일 구조

### 새로 생성될 파일
```
server/
├── src/
│   ├── thumbnail.c      ⭐ 새로 추가 (FFmpeg 썸네일 생성)
│   └── json.c           ⭐ 새로 추가 (JSON 파싱/생성)
├── include/
│   ├── thumbnail.h      ⭐ 새로 추가
│   └── json.h           ⭐ 새로 추가
└── ...

client/
├── gallery.html         ⭐ 새로 추가 (비디오 목록 페이지)
├── player.html          (업데이트: 이어보기 기능)
└── ...

thumbnails/              ⭐ 새로 생성 (썸네일 저장)
├── test_video.jpg
└── ...
```

### Makefile 업데이트
```makefile
SRCS = src/main.c src/http.c src/streaming.c src/session.c \
       src/database.c src/crypto.c src/thumbnail.c src/json.c
```

---

## 🧪 테스트 시나리오

### Test 1: 비디오 목록 페이지
1. 로그인 성공
2. `/gallery` 또는 `/` 접속
3. 모든 비디오가 카드 형식으로 표시
4. 썸네일 이미지 정상 표시

### Test 2: 썸네일 생성
1. 서버 시작
2. videos/ 디렉토리의 모든 비디오 스캔
3. thumbnails/ 디렉토리에 썸네일 자동 생성
4. 데이터베이스에 thumbnail_path 업데이트

### Test 3: 비디오 재생 및 시청 위치 저장
1. Gallery에서 비디오 클릭
2. Player 페이지로 이동
3. 비디오 재생
4. 10초마다 시청 위치 자동 저장
5. 데이터베이스 확인

### Test 4: 이어보기 기능
1. 비디오를 중간까지 시청 후 종료
2. 같은 비디오를 다시 클릭
3. "이어보기" 다이얼로그 표시
4. "이어보기" 선택 시 마지막 위치부터 재생
5. "처음부터" 선택 시 0초부터 재생

---

## 📊 구현 단계별 예상 시간

| 단계 | 작업 | 예상 시간 |
|------|------|-----------|
| 1 | JSON 유틸리티 함수 | 30분 |
| 2 | /api/videos 엔드포인트 | 40분 |
| 3 | gallery.html 페이지 | 1시간 |
| 4 | FFmpeg 썸네일 생성 | 1시간 |
| 5 | 시청 위치 저장 API | 30분 |
| 6 | Player 이어보기 기능 | 1시간 |
| 7 | 테스트 및 디버깅 | 1시간 |
| **합계** | | **약 5-6시간** |

---

## 🎯 완료 조건

- [ ] Gallery 페이지에 모든 비디오 표시
- [ ] 썸네일 이미지 자동 생성
- [ ] 비디오 클릭 시 재생
- [ ] 시청 위치 자동 저장 (10초마다)
- [ ] 이어보기 다이얼로그 표시
- [ ] 처음부터/이어보기 선택 가능
- [ ] 멀티 유저 각자의 시청 기록 관리
- [ ] 데이터베이스에 시청 기록 저장 확인

---

## 🚀 다음 단계 (Polish Phase)

Phase 3 완료 후:
1. 코드 정리 및 리팩토링
2. 에러 처리 강화
3. 보안 검토
4. 성능 최적화
5. 프로젝트 보고서 작성

---

**작성일**: 2025-11-03
**Status**: Ready to Implement 🚀
