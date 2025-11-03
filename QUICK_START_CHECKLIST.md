# ⚡ 빠른 시작 체크리스트

**마지막 업데이트**: 2025-11-03
**현재 단계**: MVP Phase 2 테스트

---

## 🎯 오늘 할 일 (5분 체크리스트)

### ☑️ 시작 전 확인

- [ ] WSL2가 설치되어 있나요?
  - **확인 방법**: `Win + R` → `wsl` 입력 → Enter
  - **안되면**: `WSL_설치가이드.md` 참조

- [ ] Linux 터미널이 열렸나요?
  - **확인**: 프롬프트가 `username@hostname:~$` 형태
  - **아니면**: Git Bash가 아닌 WSL2를 사용해야 함

### ☑️ 프로젝트 준비

```bash
# 1. 프로젝트 디렉토리로 이동
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트

# 2. 현재 위치 확인
pwd

# 3. 파일 확인
ls -la server/src/
```

### ☑️ 컴파일 및 실행

```bash
# 1. 서버 디렉토리 이동
cd server

# 2. 컴파일
make clean && make

# 3. 실행
./ott_server
```

**확인 사항:**
- [ ] "MVP Phase 2" 메시지 표시
- [ ] "Fork-based Concurrent Connection Support" 표시
- [ ] "Server is listening" 표시
- [ ] Port 8080에서 대기 중

### ☑️ 브라우저 테스트

```bash
# 브라우저 열기 (Windows에서)
# Chrome → http://localhost:8080/
```

- [ ] 플레이어 페이지 로드됨
- [ ] 비디오 재생 버튼 클릭 가능
- [ ] 비디오 재생됨
- [ ] Seek (건너뛰기) 작동함

### ☑️ 동시 접속 테스트

**3개 탭 테스트:**
- [ ] Tab 1: `http://localhost:8080/` 열기
- [ ] Tab 2: `http://localhost:8080/` 열기
- [ ] Tab 3: `http://localhost:8080/` 열기
- [ ] 모든 탭에서 동시에 재생
- [ ] 각 탭이 독립적으로 작동

**서버 로그 확인:**
- [ ] 각 연결마다 다른 Child PID 표시
- [ ] "Connection closed" 정상 표시

### ☑️ 자동 테스트 (선택)

```bash
# 새 터미널 열기
wsl

# 테스트 실행
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트/tests
bash concurrent_test.sh
```

- [ ] 모든 테스트 통과 (5/5)

---

## ✅ Phase 2 완료 확인

**모든 항목 체크되면 Phase 2 완료!**

- [ ] WSL2 환경에서 컴파일 성공
- [ ] "MVP Phase 2" 서버 정상 실행
- [ ] 단일 사용자 재생 성공
- [ ] 3개 탭 동시 재생 성공
- [ ] 각 연결마다 다른 PID 확인
- [ ] 독립적인 Seek 동작 확인
- [ ] 좀비 프로세스 없음

---

## 🚨 문제 발생 시

| 증상 | 해결 방법 |
|------|----------|
| `make: command not found` | `sudo apt install build-essential` |
| `Address already in use` | `pkill ott_server` |
| `Permission denied` | `chmod +x ott_server` |
| 브라우저 연결 안됨 | 서버 실행 확인, 방화벽 확인 |
| WSL 실행 안됨 | PowerShell에서 `wsl --install` |

**상세 해결책**: `RESTART_GUIDE.md` 참조

---

## 📞 다음 단계

Phase 2 완료 후:

```
"Phase 2 테스트 성공했습니다! 다음 단계 진행해주세요"
```

또는

```
"1029ToDo.md 읽고 세션 관리 구현해주세요"
```

---

## 🎯 빠른 명령어

```bash
# 전체 과정 한번에
wsl
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트
cd server && make clean && make && ./ott_server

# 새 터미널에서 테스트
wsl
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트/tests
bash concurrent_test.sh

# 서버 종료
Ctrl + C
```

---

**예상 소요 시간**: 10-15분
**필수 환경**: WSL2 + Ubuntu
**테스트 URL**: http://localhost:8080/

**화이팅! 🚀**
