# 세션 공유 문제 해결 방법 비교

**작성일**: 2025-11-03
**문제**: Fork 기반 서버에서 세션 공유 불가

---

## 📊 해결 방법 비교표

| 방법 | 난이도 | 소요시간 | 안정성 | Fork 유지 | 권장도 |
|------|--------|----------|--------|-----------|--------|
| 1. Thread 기반 | ⭐⭐ | 30분 | ⭐⭐⭐⭐ | ❌ | ⭐⭐⭐⭐⭐ |
| 2. 공유 메모리 (shm) | ⭐⭐⭐⭐ | 2시간 | ⭐⭐⭐⭐⭐ | ✅ | ⭐⭐⭐ |
| 3. 파일 기반 | ⭐ | 20분 | ⭐⭐⭐ | ✅ | ⭐⭐⭐⭐ |
| 4. SQLite (DB) | ⭐⭐⭐ | 1시간 | ⭐⭐⭐⭐⭐ | ✅ | ⭐⭐⭐⭐ |
| 5. Single Process | ⭐ | 15분 | ⭐⭐ | ❌ | ⭐⭐ |

---

## 방법 1: Thread 기반 서버 ⭐⭐⭐⭐⭐

### 개념
Fork 대신 pthread 사용 → 모든 thread가 메모리 공유

### 구현
```c
#include <pthread.h>

void* handle_client_thread(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);

    // 기존 child process 코드 그대로 사용
    // ...

    close(client_fd);
    return NULL;
}

// Main loop
while (1) {
    client_fd = accept(...);

    int* client_fd_ptr = malloc(sizeof(int));
    *client_fd_ptr = client_fd;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client_thread, client_fd_ptr);
    pthread_detach(thread_id);
}
```

### 장점
- ✅ 세션 자동 공유
- ✅ 빠른 구현 (30분)
- ✅ Fork보다 가벼움
- ✅ 코드 변경 최소

### 단점
- ⚠️ Thread-safe 고려 필요
- ❌ Fork 패턴 포기

### 추가 작업
- Mutex 추가 (나중에 - 현재 MVP에서는 불필요)
- SIGCHLD 핸들러 제거

### 난이도
⭐⭐ 중하

---

## 방법 2: 공유 메모리 (Shared Memory) ⭐⭐⭐

### 개념
POSIX 공유 메모리로 모든 프로세스가 같은 세션 저장소 접근

### 구현
```c
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>

// 전역 변수
Session* session_store;  // 공유 메모리 포인터
sem_t* session_sem;      // 세마포어 (동기화)

void init_session_store() {
    // 공유 메모리 생성
    int shmid = shmget(IPC_PRIVATE,
                       sizeof(Session) * MAX_SESSIONS,
                       IPC_CREAT | 0666);

    session_store = (Session*)shmat(shmid, NULL, 0);

    // 세마포어 생성 (동기화)
    session_sem = sem_open("/ott_session_sem", O_CREAT, 0644, 1);

    // 초기화
    memset(session_store, 0, sizeof(Session) * MAX_SESSIONS);
}

char* create_session(const char* user_id) {
    sem_wait(session_sem);  // Lock

    // 세션 생성 로직 (기존과 동일)
    // ...

    sem_post(session_sem);  // Unlock
    return session_id;
}
```

### 장점
- ✅ Fork 패턴 유지
- ✅ 프로세스 격리 유지
- ✅ 성능 우수
- ✅ Production-ready

### 단점
- ⚠️ 구현 복잡 (semaphore 필요)
- ⚠️ 디버깅 어려움
- ⚠️ 시간 소요 (2시간)

### 난이도
⭐⭐⭐⭐ 어려움

---

## 방법 3: 파일 기반 세션 저장소 ⭐⭐⭐⭐

### 개념
세션을 파일에 저장 → 모든 프로세스가 파일 읽기/쓰기

### 구현
```c
#define SESSION_FILE "/tmp/ott_sessions.dat"

char* create_session(const char* user_id) {
    // 세션 생성
    Session new_session;
    generate_session_id(new_session.session_id);
    strncpy(new_session.user_id, user_id, USER_ID_LENGTH);
    new_session.created_at = time(NULL);
    new_session.last_accessed = time(NULL);
    new_session.is_active = 1;

    // 파일에 추가
    FILE* f = fopen(SESSION_FILE, "ab");
    fwrite(&new_session, sizeof(Session), 1, f);
    fclose(f);

    return new_session.session_id;
}

int validate_session(const char* session_id) {
    FILE* f = fopen(SESSION_FILE, "rb");
    if (!f) return 0;

    Session session;
    time_t now = time(NULL);

    while (fread(&session, sizeof(Session), 1, f) == 1) {
        if (session.is_active &&
            strcmp(session.session_id, session_id) == 0) {

            // 타임아웃 확인
            if (now - session.last_accessed > SESSION_TIMEOUT) {
                fclose(f);
                return 0;
            }

            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}
```

### 장점
- ✅ 구현 간단 (20분)
- ✅ Fork 패턴 유지
- ✅ 서버 재시작 후에도 세션 유지
- ✅ 디버깅 쉬움 (파일 직접 확인)

### 단점
- ⚠️ 성능 낮음 (파일 I/O)
- ⚠️ 파일 락 필요 (동시 쓰기)
- ⚠️ 만료 세션 정리 복잡

### 개선 방법
- 메모리 캐시 추가
- 주기적 파일 정리

### 난이도
⭐ 쉬움

---

## 방법 4: SQLite 데이터베이스 ⭐⭐⭐⭐

### 개념
세션을 SQLite DB에 저장 → 자동 동기화

### 구현
```c
#include <sqlite3.h>

sqlite3* db;

void init_session_store() {
    sqlite3_open("/tmp/ott.db", &db);

    const char* sql =
        "CREATE TABLE IF NOT EXISTS sessions ("
        "session_id TEXT PRIMARY KEY,"
        "user_id TEXT,"
        "created_at INTEGER,"
        "last_accessed INTEGER"
        ");";

    sqlite3_exec(db, sql, NULL, NULL, NULL);
}

char* create_session(const char* user_id) {
    char session_id[SESSION_ID_LENGTH];
    generate_session_id(session_id);

    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO sessions VALUES ('%s', '%s', %ld, %ld);",
        session_id, user_id, time(NULL), time(NULL));

    sqlite3_exec(db, sql, NULL, NULL, NULL);

    return session_id;  // 주의: static 변수로 반환 필요
}

int validate_session(const char* session_id) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT last_accessed FROM sessions "
        "WHERE session_id = '%s';",
        session_id);

    // SQLite 쿼리 실행 및 타임아웃 확인
    // ...
}
```

### 장점
- ✅ Fork 패턴 유지
- ✅ Production-ready
- ✅ 자동 동기화
- ✅ 확장 가능 (users 테이블 추가 등)
- ✅ Enhancement Phase 2와 통합 가능

### 단점
- ⚠️ SQLite 라이브러리 필요
- ⚠️ 구현 시간 (1시간)
- ⚠️ 약간 복잡

### 난이도
⭐⭐⭐ 중상

---

## 방법 5: Single Process (select/epoll) ⭐⭐

### 개념
Fork 제거 → 단일 프로세스로 모든 연결 처리

### 구현
```c
#include <sys/select.h>

int main() {
    fd_set readfds;
    int client_fds[MAX_CLIENTS];

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        // 모든 클라이언트 소켓 추가
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] > 0) {
                FD_SET(client_fds[i], &readfds);
            }
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        // 새 연결 확인
        if (FD_ISSET(server_fd, &readfds)) {
            int client_fd = accept(server_fd, ...);
            // 클라이언트 배열에 추가
        }

        // 클라이언트 요청 처리
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (FD_ISSET(client_fds[i], &readfds)) {
                // 요청 처리
            }
        }
    }
}
```

### 장점
- ✅ 세션 공유 자동
- ✅ 구현 간단
- ✅ 리소스 사용 적음

### 단점
- ❌ 동시 처리 성능 낮음
- ❌ 블로킹 작업 시 전체 서버 대기
- ❌ Fork/Thread 패턴 포기

### 난이도
⭐ 쉬움

---

## 🎯 추천 순위

### 1위: Thread 기반 (pthread) ⭐⭐⭐⭐⭐
**이유**: 빠르고 간단하며 효과적

**추천 상황**:
- ✅ 빨리 완성하고 싶을 때
- ✅ MVP 단계
- ✅ 학습 프로젝트

### 2위: 파일 기반 ⭐⭐⭐⭐
**이유**: 구현 쉽고 디버깅 편함

**추천 상황**:
- ✅ Fork 패턴 유지하고 싶을 때
- ✅ 간단한 해결책 원할 때
- ✅ 세션 지속성 필요할 때

### 3위: SQLite ⭐⭐⭐⭐
**이유**: Enhancement Phase 2와 통합 가능

**추천 상황**:
- ✅ 어차피 DB 쓸 예정일 때
- ✅ Production 수준 원할 때
- ✅ 확장성 고려할 때

### 4위: 공유 메모리 ⭐⭐⭐
**이유**: 복잡하지만 가장 정석

**추천 상황**:
- ✅ 고성능 필요할 때
- ✅ 시스템 프로그래밍 학습 목적
- ✅ Fork 패턴 필수일 때

---

## 💡 상황별 권장 사항

### 빠른 완성이 목표 (시간 부족)
→ **Thread 기반** (30분)

### Fork 패턴 유지하고 싶음
→ **파일 기반** (20분) 또는 **SQLite** (1시간)

### 학습과 완성 둘 다
→ **Thread 기반** → 나중에 **공유 메모리**로 업그레이드

### Production 수준 원함
→ **SQLite** (Enhancement Phase 2와 통합)

---

## 📝 각 방법의 코드 변경 범위

### Thread 기반
- `main.c`: fork → pthread (50줄)
- `session.c`: 변경 없음

### 파일 기반
- `session.c`: 저장/로드 함수 추가 (100줄)
- `main.c`: 변경 없음

### SQLite
- `session.c`: DB 연동 (150줄)
- `Makefile`: -lsqlite3 추가
- `main.c`: 변경 없음

### 공유 메모리
- `session.c`: shm + semaphore (200줄)
- `main.c`: 약간 수정

---

## 🚀 결정 가이드

다음 질문에 답하세요:

1. **Fork 패턴을 꼭 유지해야 하나요?**
   - YES → 파일 기반 또는 SQLite
   - NO → Thread 기반 (강력 추천)

2. **어차피 나중에 DB를 쓸 예정인가요?**
   - YES → SQLite (지금 시작)
   - NO → Thread 기반 또는 파일 기반

3. **시간이 얼마나 있나요?**
   - 30분 → Thread 기반
   - 1시간 → SQLite
   - 2시간 → 공유 메모리

---

**어떤 방법을 선택하시겠습니까?**

1️⃣ Thread 기반 (30분, 가장 빠름)
2️⃣ 파일 기반 (20분, Fork 유지)
3️⃣ SQLite (1시간, 확장 가능)
4️⃣ 공유 메모리 (2시간, 정석)
5️⃣ 더 자세한 설명 필요
