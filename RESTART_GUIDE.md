# 🔄 OTT 스트리밍 서버 재시작 가이드

**작성일**: 2025-11-03
**현재 상태**: MVP Phase 2 구현 완료, WSL2 환경에서 테스트 필요
**목표**: 동시 접속 처리 테스트 및 검증

---

## ⚠️ 중요: 환경 확인

이 프로젝트는 **Linux 환경**이 필요합니다 (fork(), POSIX signals 사용).
Windows Git Bash에서는 실행되지 않습니다.

**필수**: WSL2 (Windows Subsystem for Linux) 사용

---

## 🚀 빠른 시작 (3가지 옵션)

### **옵션 1: WSL2가 이미 설치된 경우** (추천)

```bash
# 1. WSL2 터미널 열기
wsl

# 2. 프로젝트 디렉토리 이동
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트

# 3. 서버 재컴파일
cd server
make clean
make

# 4. 서버 실행
./ott_server

# 5. 브라우저에서 테스트
# Chrome에서 http://localhost:8080/ 접속
```

### **옵션 2: WSL2가 설치되지 않은 경우**

1. `WSL_설치가이드.md` 파일 참조
2. PowerShell 관리자 권한으로 실행
3. `wsl --install` 명령어 실행
4. 컴퓨터 재시작
5. Ubuntu 초기 설정 (사용자명, 비밀번호)
6. 개발 도구 설치:
   ```bash
   sudo apt update
   sudo apt install build-essential
   ```
7. 위 "옵션 1" 단계 실행

### **옵션 3: WSL2 없이 현재 상태만 확인**

```bash
# Git Bash에서 (컴파일은 불가능, 파일 확인만 가능)
cd /c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트

# 프로젝트 파일 확인
ls -la server/src/
ls -la client/
ls -la videos/

# Phase 2 가이드 읽기
cat MVP_PHASE2_실행가이드.md

# 현재 할 일 확인
cat 1029ToDo.md
```

---

## 📝 단계별 상세 가이드 (WSL2 사용)

### **1️⃣ WSL2 터미널 열기**

**방법 A: Windows Terminal 사용**
- `Win + R` → `wsl` 입력 → Enter

**방법 B: 시작 메뉴**
- 시작 메뉴 → "Ubuntu" 검색 → 실행

**방법 C: PowerShell에서**
```powershell
wsl
```

**확인**: 프롬프트가 `username@hostname:~$` 형태여야 함

### **2️⃣ 환경 확인**

```bash
# Linux 환경인지 확인
uname -a
# 출력: Linux ... (NOT MINGW)

# 필수 도구 확인
gcc --version
make --version

# 없으면 설치
sudo apt update
sudo apt install build-essential
```

### **3️⃣ 프로젝트 디렉토리로 이동**

```bash
# Windows 경로를 WSL 경로로 변환
# C:\Users\dclab\... → /mnt/c/Users/dclab/...

cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트

# 현재 위치 확인
pwd
# 출력: /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트
```

### **4️⃣ 파일 구조 확인**

```bash
# 전체 구조 확인
ls -la

# 주요 파일 확인
ls -la server/src/
ls -la server/include/
ls -la client/
ls -la videos/
ls -la tests/
```

**예상 출력:**
```
server/src/
  - main.c (Phase 2 fork 구현)
  - http.c
  - streaming.c

server/include/
  - server.h

client/
  - player.html

videos/
  - test_video.mp4 (189K)

tests/
  - concurrent_test.sh
```

### **5️⃣ 서버 재컴파일**

```bash
cd server

# 이전 빌드 파일 제거
make clean

# 새로 컴파일
make
```

**성공 시 출력:**
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

**에러 발생 시:**
```bash
# 에러 메시지 전체 복사
# 다음 세션에서 에러 내용과 함께 도움 요청
```

### **6️⃣ 서버 실행**

```bash
# server 디렉토리에서
./ott_server
```

**정상 실행 시 출력:**
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

**중요 확인 사항:**
- ✅ "MVP Phase 2" 표시되어야 함
- ✅ "Fork-based Concurrent Connection Support" 표시되어야 함
- ✅ Port 8080에서 listening

### **7️⃣ 브라우저 테스트 (단일 사용자)**

1. **Chrome 브라우저 열기** (Windows에서)
2. **주소창에 입력**: `http://localhost:8080/`
3. **플레이어 페이지 확인**
4. **비디오 재생 버튼 클릭**

**서버 터미널에서 확인:**
```
✓ Client connected: 127.0.0.1 (PID: 1234)
  [Child 1235] GET /
  [Child 1235] Connection closed

✓ Client connected: 127.0.0.1 (PID: 1234)
  [Child 1236] GET /videos/test_video.mp4
  [Child 1236] Connection closed
```

### **8️⃣ 동시 접속 테스트 (Phase 2 핵심)**

#### 방법 A: 수동 브라우저 테스트

1. **Chrome에서 3개 탭 열기**
   - Tab 1: `http://localhost:8080/`
   - Tab 2: `http://localhost:8080/`
   - Tab 3: `http://localhost:8080/`

2. **각 탭에서 비디오 재생**
   - 모든 탭에서 Play 버튼 클릭

3. **서버 터미널 확인**
   ```
   ✓ Client connected: 127.0.0.1 (PID: 1234)
     [Child 1235] GET /
   ✓ Client connected: 127.0.0.1 (PID: 1234)
     [Child 1236] GET /
   ✓ Client connected: 127.0.0.1 (PID: 1234)
     [Child 1237] GET /
   ```

**성공 기준:**
- ✅ 3개 탭 모두 비디오 재생됨
- ✅ 각 연결마다 **다른 Child PID** 표시
- ✅ 각 탭이 독립적으로 Seek/Pause 작동

#### 방법 B: 자동 테스트 스크립트

```bash
# 새 터미널 창 열기 (서버는 계속 실행 중)
wsl

# 프로젝트 디렉토리 이동
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트

# 테스트 스크립트 실행
cd tests
bash concurrent_test.sh
```

**성공 시 출력:**
```
=== OTT 서버 동시 접속 테스트 ===

[1/5] 서버 연결 테스트...
✓ 서버가 응답합니다

[2/5] 기본 페이지 로드 테스트...
✓ 플레이어 페이지 로드 성공

[3/5] 비디오 스트리밍 테스트...
✓ 비디오 스트리밍 작동

[4/5] Range Request 테스트...
✓ Range Request 지원 확인

[5/5] 동시 접속 테스트 (3명)...
✓ 동시 접속 처리 성공

✅ 모든 테스트 통과!
```

### **9️⃣ 좀비 프로세스 확인**

```bash
# 새 터미널에서
wsl

# 프로세스 확인
ps aux | grep ott_server

# 좀비 프로세스 확인 (없어야 정상)
ps aux | grep defunct
# 출력: (아무것도 없음)
```

**정상:**
- Parent process 1개 (listening)
- Child processes: 요청 처리 후 자동 종료
- Defunct (zombie): 0개

### **🔟 서버 종료**

```bash
# 서버 실행 중인 터미널에서
Ctrl + C

# 또는 다른 터미널에서 강제 종료
pkill ott_server
```

---

## ✅ Phase 2 완료 기준

다음을 모두 확인하면 Phase 2 완료:

- [ ] WSL2 환경에서 컴파일 성공
- [ ] 서버 실행 시 "MVP Phase 2" 표시
- [ ] 단일 사용자 비디오 재생 성공
- [ ] 3개 브라우저 탭 동시 재생 성공
- [ ] 서버 로그에 다른 Child PID 표시
- [ ] 각 탭이 독립적으로 Seek 작동
- [ ] 좀비 프로세스 없음 (ps aux | grep defunct)
- [ ] 자동 테스트 스크립트 통과 (선택)

---

## 🐛 문제 해결

### 문제 1: "Address already in use"
```bash
# 원인: 이전 서버가 아직 실행 중
# 해결:
pkill ott_server

# 또는
lsof -ti:8080 | xargs kill -9
```

### 문제 2: "Permission denied"
```bash
# 원인: 실행 권한 없음
# 해결:
chmod +x ott_server
```

### 문제 3: "make: command not found"
```bash
# 원인: 개발 도구 미설치
# 해결:
sudo apt update
sudo apt install build-essential
```

### 문제 4: "No such file or directory" (video)
```bash
# 원인: 비디오 파일 경로 문제
# 확인:
ls -la ../videos/test_video.mp4

# test_video.mp4가 없으면 샘플 비디오 필요
# (최소 1MB 이상의 MP4 파일)
```

### 문제 5: 브라우저에서 연결 안됨
```bash
# Windows 방화벽 확인
# 또는 WSL IP 주소로 접속 시도

# WSL IP 확인:
ip addr show eth0 | grep inet

# 브라우저에서:
http://172.x.x.x:8080/
```

---

## 📋 다음 단계 (Phase 2 완료 후)

Phase 2가 성공적으로 완료되면:

1. **1029ToDo.md** 파일의 "다음 단계 로드맵" 참조
2. 다음 기능 구현 시작:
   - 세션 관리 (쿠키 기반)
   - 간단한 로그인 페이지
   - 세션 검증

요청 예시:
```
"Phase 2 테스트 완료했습니다! 이제 세션 관리 기능 추가해주세요"
```

---

## 📞 현재 상태 요약

**프로젝트 진행률**: 약 40%

- ✅ Preparation Phase: 100% 완료
- ✅ MVP Phase 1: 100% 완료
- 🔄 MVP Phase 2: 95% 완료 (테스트만 남음)
- ⏳ Enhancement Phase 1: 0%
- ⏳ Enhancement Phase 2: 0%
- ⏳ Polish Phase: 0%

**마감**: 2025년 12월 10일 (약 5주 남음)

**다음 작업**: WSL2에서 Phase 2 동시 접속 테스트

---

## 🎯 핵심 명령어 요약

```bash
# WSL2 열기
wsl

# 프로젝트 이동
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트

# 컴파일
cd server && make clean && make

# 실행
./ott_server

# 테스트 (새 터미널)
cd tests && bash concurrent_test.sh

# 종료
Ctrl + C
# 또는
pkill ott_server

# 프로세스 확인
ps aux | grep ott_server
ps aux | grep defunct
```

---

**작성**: 2025-11-03
**목적**: OTT 스트리밍 서버 재시작 가이드
**환경**: WSL2 + Ubuntu + GCC
**포트**: 8080
**테스트 URL**: http://localhost:8080/

**잘 작동하길 바랍니다! 🚀**
