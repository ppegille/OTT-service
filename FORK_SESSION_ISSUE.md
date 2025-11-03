# Fork 기반 서버의 세션 공유 문제

**발견일**: 2025-11-03
**문제**: 로그인 성공 후 세션이 유지되지 않음

---

## 🐛 문제 원인

### Fork 프로세스 메모리 격리

```
Parent Process (PID: 1234)
├── session_store[] (메모리 A)
│
├─ Child 1 (로그인 처리)
│  └── session_store[] (메모리 A의 복사본)
│      └── 세션 생성! ✅
│      └── 프로세스 종료
│
└─ Child 2 (GET / 처리)
   └── session_store[] (메모리 A의 복사본)
       └── 세션 없음! ❌ (Child 1의 변경사항이 없음)
```

**핵심 문제**:
- Child process는 parent의 메모리를 **복사**함 (Copy-on-Write)
- Child 1이 세션을 생성해도, Child 2는 그 세션을 볼 수 없음
- 각 child는 독립적인 메모리 공간을 가짐

---

## 💡 해결 방법

### 방법 1: 공유 메모리 (Shared Memory) ⭐ 추천
```c
#include <sys/shm.h>
#include <sys/ipc.h>

// 공유 메모리 생성
int shmid = shmget(IPC_PRIVATE, sizeof(Session) * MAX_SESSIONS, IPC_CREAT | 0666);
Session* session_store = (Session*)shmat(shmid, NULL, 0);
```

**장점**:
- 모든 프로세스가 같은 메모리 공유
- 빠름

**단점**:
- 동기화 필요 (mutex/semaphore)
- 복잡함

### 방법 2: 파일 기반 세션 저장소
```c
// 세션 생성 시
void save_session_to_file(Session* session) {
    FILE* f = fopen("/tmp/sessions.dat", "ab");
    fwrite(session, sizeof(Session), 1, f);
    fclose(f);
}

// 세션 검증 시
Session* load_sessions() {
    FILE* f = fopen("/tmp/sessions.dat", "rb");
    // 모든 세션 읽기
    fclose(f);
}
```

**장점**:
- 간단함
- 서버 재시작 후에도 세션 유지

**단점**:
- 느림 (파일 I/O)
- 파일 동기화 필요

### 방법 3: Single-threaded with select/epoll
```c
// Fork 대신 select()로 다중 연결 처리
fd_set readfds;
select(max_fd + 1, &readfds, NULL, NULL, NULL);
```

**장점**:
- 메모리 공유 문제 없음
- 간단함

**단점**:
- 동시 처리 성능 낮음
- 블로킹 작업 시 다른 클라이언트 대기

### 방법 4: Thread 기반 서버
```c
pthread_t thread_id;
pthread_create(&thread_id, NULL, handle_client, (void*)&client_fd);
```

**장점**:
- 메모리 자동 공유
- Fork보다 가벼움

**단점**:
- Thread-safe 코드 필요
- Mutex/lock 관리

---

## 🚀 빠른 해결책 (임시)

### 옵션 A: 공유 메모리 구현 (1시간)
가장 안정적이지만 구현 시간 필요

### 옵션 B: Thread 기반으로 변경 (30분)
Fork → pthread로 변경

### 옵션 C: Single Process (15분)
Fork 제거, 단일 프로세스로 처리

---

## 📝 권장 해결 순서

### 1단계: 문제 확인
재컴파일하고 다음 로그 확인:

```bash
cd server && make clean && make && ./ott_server
```

로그인 후 `/`로 리다이렉트될 때:
```
[DEBUG] Cookie header: 'session_id=xxx...'
[DEBUG] Parsed session_id: 'xxx...'
[Child XXX] No valid session, redirecting to login  ← 이것이 나오면 확정!
```

### 2단계: Thread 기반으로 빠르게 변경 (추천)

**main.c 수정**:
```c
#include <pthread.h>

// Child process 핸들러를 thread 함수로 변경
void* handle_client_thread(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);

    // 기존 child process 코드 그대로
    // ...

    close(client_fd);
    return NULL;
}

// Main loop에서
while (1) {
    client_fd = accept(...);

    int* client_fd_ptr = malloc(sizeof(int));
    *client_fd_ptr = client_fd;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client_thread, client_fd_ptr);
    pthread_detach(thread_id);  // 자동 정리
}
```

**장점**:
- 세션 자동 공유 (같은 메모리 공간)
- 최소한의 코드 변경
- 30분 내 완료 가능

---

## 🎯 추천 해결책

**단기 (MVP)**: Thread 기반으로 변경
**장기 (Production)**: 공유 메모리 + Mutex

---

**다음 단계**: Thread 기반 서버로 변경하시겠습니까?
