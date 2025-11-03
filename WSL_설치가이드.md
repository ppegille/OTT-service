# WSL2 개발 환경 설치 가이드 (처음부터 끝까지)

**목적**: Windows에서 Linux 개발 환경 구축하기

**소요 시간**: 약 30분 ~ 1시간

---

## 📋 설치 순서

1. WSL2 설치 (Windows 기능 활성화)
2. Ubuntu 설치
3. 개발 도구 설치 (gcc, make, sqlite3, ffmpeg)
4. 환경 확인
5. 첫 번째 예제 실행

---

## 🔧 1단계: WSL2 설치

### 방법 A: 자동 설치 (가장 쉬움, Windows 10 버전 2004 이상)

#### 1-1. PowerShell 관리자 권한으로 실행

1. **시작 메뉴** 클릭
2. **PowerShell** 검색
3. **Windows PowerShell** 우클릭
4. **관리자 권한으로 실행** 선택

#### 1-2. WSL 설치 명령어 실행

PowerShell에 다음 명령어를 입력하고 Enter:

```powershell
wsl --install
```

**예상 출력:**
```
Installing: Virtual Machine Platform
Installing: Windows Subsystem for Linux
Installing: Ubuntu
...
```

#### 1-3. 컴퓨터 재시작

```powershell
# 재시작 명령어 (또는 수동으로 재시작)
Restart-Computer
```

⚠️ **중요**: 재시작 후 자동으로 Ubuntu 설치 창이 나타날 수 있습니다.

---

### 방법 B: 수동 설치 (방법 A가 안 되는 경우)

#### 1-1. PowerShell 관리자 권한 실행

위 방법 A의 1-1과 동일

#### 1-2. WSL 기능 활성화

```powershell
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
```

#### 1-3. Virtual Machine 기능 활성화

```powershell
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
```

#### 1-4. 컴퓨터 재시작

```powershell
Restart-Computer
```

#### 1-5. WSL2 Linux 커널 업데이트 패키지 다운로드

재시작 후:

1. 웹 브라우저 열기
2. 다음 링크 접속:
   ```
   https://aka.ms/wsl2kernel
   ```
3. 다운로드된 파일 실행: `wsl_update_x64.msi`
4. 설치 완료

#### 1-6. WSL2를 기본 버전으로 설정

PowerShell (관리자)에서:

```powershell
wsl --set-default-version 2
```

---

## 🐧 2단계: Ubuntu 설치

### 2-1. Microsoft Store에서 Ubuntu 설치

#### 옵션 1: PowerShell로 설치 (빠름)

```powershell
wsl --install -d Ubuntu
```

#### 옵션 2: Microsoft Store에서 수동 설치

1. **Microsoft Store** 앱 실행
2. 검색창에 **"Ubuntu"** 입력
3. **Ubuntu** (또는 Ubuntu 22.04 LTS) 선택
4. **설치** 버튼 클릭
5. 설치 완료 대기 (약 5-10분)

### 2-2. Ubuntu 첫 실행

#### 실행 방법

**방법 1**: 시작 메뉴에서 **Ubuntu** 검색 → 클릭

**방법 2**: PowerShell에서
```powershell
wsl
```

#### 첫 실행 시 설정

Ubuntu가 처음 실행되면 초기 설정 진행:

```
Installing, this may take a few minutes...
Please create a default UNIX user account. The username does not need to match your Windows username.
For more information visit: https://aka.ms/wslusers
Enter new UNIX username:
```

**1. 사용자 이름 입력**
```
Enter new UNIX username: dclab
```
(영문 소문자만 사용, 공백 없이)

**2. 비밀번호 입력**
```
New password:
```
⚠️ **중요**: 비밀번호 입력 시 화면에 아무것도 표시되지 않습니다. (보안 기능)

**3. 비밀번호 확인**
```
Retype new password:
```

**4. 설정 완료**
```
Installation successful!
To run a command as administrator (user "root"), use "sudo <command>".
See "man sudo_root" for details.

dclab@DESKTOP:~$
```

✅ 이 화면이 나오면 성공!

---

## 📦 3단계: 개발 도구 설치

Ubuntu 터미널이 열린 상태에서 진행합니다.

### 3-1. 패키지 목록 업데이트

```bash
sudo apt update
```

**"sudo"란?**: 관리자 권한으로 명령 실행 (비밀번호 입력 필요)

**예상 출력:**
```
[sudo] password for dclab:
Hit:1 http://archive.ubuntu.com/ubuntu jammy InRelease
Get:2 http://security.ubuntu.com/ubuntu jammy-security InRelease [110 kB]
...
Reading package lists... Done
```

### 3-2. 패키지 업그레이드 (선택사항, 권장)

```bash
sudo apt upgrade -y
```

**"-y"**: 모든 질문에 자동으로 "예" 응답

**소요 시간**: 약 5-10분 (처음 설치 시)

### 3-3. 개발 도구 설치

#### GCC 컴파일러 및 빌드 도구 설치

```bash
sudo apt install build-essential -y
```

**"build-essential"이란?**: gcc, g++, make 등 C/C++ 컴파일에 필요한 모든 도구 포함

**예상 출력:**
```
Reading package lists... Done
Building dependency tree... Done
The following NEW packages will be installed:
  binutils gcc g++ make ...
...
Setting up build-essential (12.9ubuntu3) ...
```

#### SQLite3 설치

```bash
sudo apt install sqlite3 libsqlite3-dev -y
```

**libsqlite3-dev**: C 프로그램에서 SQLite를 사용하기 위한 헤더 파일 포함

#### FFmpeg 설치

```bash
sudo apt install ffmpeg -y
```

**FFmpeg**: 비디오 파일 처리 및 썸네일 생성에 필요

#### 추가 유용한 도구 설치 (선택사항)

```bash
sudo apt install curl git vim -y
```

- **curl**: 웹에서 파일 다운로드, API 테스트
- **git**: 버전 관리
- **vim**: 텍스트 편집기 (또는 nano 사용 가능)

---

## ✅ 4단계: 설치 확인

모든 도구가 제대로 설치되었는지 확인합니다.

### 4-1. GCC 확인

```bash
gcc --version
```

**예상 출력:**
```
gcc (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0
Copyright (C) 2021 Free Software Foundation, Inc.
...
```

버전이 표시되면 성공! ✅

### 4-2. Make 확인

```bash
make --version
```

**예상 출력:**
```
GNU Make 4.3
Built for x86_64-pc-linux-gnu
...
```

### 4-3. SQLite 확인

```bash
sqlite3 --version
```

**예상 출력:**
```
3.37.2 2022-01-06 13:25:41 ...
```

### 4-4. FFmpeg 확인

```bash
ffmpeg -version
```

**예상 출력:**
```
ffmpeg version 4.4.2-0ubuntu0.22.04.1
...
```

### 4-5. 모든 도구 한 번에 확인

```bash
echo "=== 설치 확인 ==="
echo "GCC: $(gcc --version | head -1)"
echo "Make: $(make --version | head -1)"
echo "SQLite: $(sqlite3 --version)"
echo "FFmpeg: $(ffmpeg -version | head -1)"
echo "완료!"
```

**예상 출력:**
```
=== 설치 확인 ===
GCC: gcc (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0
Make: GNU Make 4.3
SQLite: 3.37.2 2022-01-06 13:25:41 ...
FFmpeg: ffmpeg version 4.4.2-0ubuntu0.22.04.1
완료!
```

모두 표시되면 **설치 완료!** 🎉

---

## 📂 5단계: Windows 파일 접근하기

WSL에서 Windows 파일에 접근하는 방법을 알아봅니다.

### Windows 드라이브는 `/mnt/` 아래에 있습니다

- **C 드라이브**: `/mnt/c/`
- **D 드라이브**: `/mnt/d/`

### 프로젝트 폴더로 이동

```bash
# Windows 경로: C:\Users\dclab\Desktop\2025_2학기\네트워크프로그래밍\기말프로젝트
# WSL 경로:
cd "/mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트"
```

⚠️ **주의**: 경로에 한글이나 공백이 있으면 따옴표로 감싸야 합니다.

### 현재 위치 확인

```bash
pwd
```

**예상 출력:**
```
/mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트
```

### 파일 목록 보기

```bash
ls -la
```

**예상 출력:**
```
total 16
drwxr-xr-x 1 dclab dclab 4096 Oct 28 09:00 .
drwxr-xr-x 1 dclab dclab 4096 Oct 28 08:00 ..
-rw-r--r-- 1 dclab dclab 1234 Oct 28 09:00 CLAUDE.md
drwxr-xr-x 1 dclab dclab 4096 Oct 28 09:00 practice
...
```

---

## 🚀 6단계: 첫 번째 예제 실행해보기

이제 실제로 코드를 컴파일하고 실행해봅니다!

### 6-1. practice 폴더로 이동

```bash
cd practice
```

### 6-2. 파일 확인

```bash
ls -la
```

**예상 출력:**
```
-rw-r--r-- 1 dclab dclab 3163 Oct 28 09:13 exercise1_hello_server.c
-rw-r--r-- 1 dclab dclab 3552 Oct 28 09:14 exercise2_file_server.c
-rw-r--r-- 1 dclab dclab 38655 Oct 28 09:12 EXERCISES.md
...
```

### 6-3. Exercise 1 컴파일

```bash
gcc -o exercise1 exercise1_hello_server.c
```

**설명:**
- `gcc`: C 컴파일러
- `-o exercise1`: 출력 파일 이름을 "exercise1"로 지정
- `exercise1_hello_server.c`: 소스 코드 파일

**성공하면 아무 출력도 없습니다!** (에러가 없다는 뜻)

### 6-4. 컴파일된 파일 확인

```bash
ls -l exercise1
```

**예상 출력:**
```
-rwxr-xr-x 1 dclab dclab 17856 Oct 28 10:00 exercise1
```

녹색으로 표시되고 `x` (실행 권한)가 있으면 성공!

### 6-5. 프로그램 실행

```bash
./exercise1
```

**예상 출력:**
```
=== Exercise 1: Basic HTTP Server ===

Step 1: Creating socket...
✓ Socket created (fd=3)

Step 2: Binding to port 8080...
✓ Bound to port 8080

Step 3: Listening for connections...
✓ Listening (backlog=10)

🚀 Server ready! Access http://localhost:8080/
Press Ctrl+C to stop

Waiting for connection...
```

✅ **성공!** 서버가 실행 중입니다!

### 6-6. 서버 테스트

**새 PowerShell 창 열기** (기존 창은 그대로 두기):

1. 시작 메뉴 → PowerShell 실행
2. WSL 실행:
   ```powershell
   wsl
   ```
3. 테스트 명령어:
   ```bash
   curl http://localhost:8080/
   ```

**예상 출력:**
```html
<html><body><h1>Hello, World!</h1><p>Exercise 1: Basic HTTP Server Working!</p></body></html>
```

🎉 **완벽합니다!** HTTP 서버가 정상 작동합니다!

### 6-7. 서버 종료

원래 서버가 실행 중인 터미널로 돌아가서:

```
Ctrl + C
```

**예상 출력:**
```
^C
dclab@DESKTOP:~/practice$
```

서버가 종료되고 명령 프롬프트로 돌아옵니다.

---

## 💡 유용한 WSL 명령어

### Windows에서 WSL 실행

```powershell
# 기본 배포판 실행
wsl

# 특정 배포판 실행
wsl -d Ubuntu

# 특정 명령어 실행
wsl ls -la

# 특정 디렉토리에서 실행
wsl --cd "/mnt/c/Users/dclab/Desktop"
```

### WSL 상태 확인

```powershell
# 설치된 배포판 목록
wsl --list --verbose

# 또는 짧게
wsl -l -v
```

**예상 출력:**
```
  NAME      STATE           VERSION
* Ubuntu    Running         2
```

### WSL 종료

```powershell
# 모든 WSL 인스턴스 종료
wsl --shutdown

# 특정 배포판만 종료
wsl --terminate Ubuntu
```

### Ubuntu 재시작

```powershell
wsl --shutdown
wsl
```

---

## 🔧 문제 해결 (Troubleshooting)

### 문제 1: "wsl: command not found"

**원인**: WSL이 설치되지 않았거나 PATH에 없음

**해결 방법**:
1. Windows 버전 확인 (Windows 10 버전 2004 이상 필요)
2. Windows 업데이트 실행
3. WSL 수동 설치 (방법 B 참조)

### 문제 2: "가상화를 사용할 수 없습니다"

**원인**: BIOS에서 가상화 기능이 비활성화됨

**해결 방법**:
1. BIOS 진입 (재부팅 시 F2, F10, Del 키 등)
2. Virtualization Technology (VT-x/AMD-V) 활성화
3. 저장 후 재부팅

### 문제 3: Ubuntu 실행 시 오류

**원인**: 배포판이 제대로 설치되지 않음

**해결 방법**:
```powershell
# Ubuntu 제거
wsl --unregister Ubuntu

# 재설치
wsl --install -d Ubuntu
```

### 문제 4: "sudo apt update" 실패

**원인**: 인터넷 연결 문제 또는 DNS 설정

**해결 방법**:
```bash
# DNS 설정 변경
sudo bash -c 'echo "nameserver 8.8.8.8" > /etc/resolv.conf'

# 다시 시도
sudo apt update
```

### 문제 5: gcc 컴파일 시 "Permission denied"

**원인**: Windows 파일 시스템 권한 문제

**해결 방법**:
```bash
# 파일에 실행 권한 부여
chmod +x exercise1

# 또는 WSL 홈 디렉토리에서 작업
cd ~
cp -r /mnt/c/Users/dclab/Desktop/.../practice ./
cd practice
gcc -o exercise1 exercise1_hello_server.c
```

---

## 📝 다음 단계

환경 설정이 완료되었습니다! 이제:

### ✅ 완료된 것들
- [x] WSL2 설치
- [x] Ubuntu 설치
- [x] 개발 도구 설치 (gcc, make, sqlite3, ffmpeg)
- [x] 첫 번째 예제 컴파일 및 실행

### 📚 다음에 할 일

1. **Exercise 1 복습**
   - 코드 읽어보기
   - 수정하고 다시 컴파일해보기

2. **Exercise 2 실행**
   ```bash
   # 테스트 파일 생성
   echo -e "Line 1\nLine 2\nLine 3\nLine 4\nLine 5" > test.txt

   # 컴파일 및 실행
   gcc -o exercise2 exercise2_file_server.c
   ./exercise2

   # 새 터미널에서 테스트
   curl http://localhost:8080/test.txt
   ```

3. **EXERCISES.md 읽기**
   - Exercise 3, 4 코드 복사하기
   - 각 예제의 목적 이해하기

---

## 🎯 빠른 참조 카드

### 자주 사용하는 명령어

```bash
# 디렉토리 이동
cd /mnt/c/Users/dclab/Desktop/...

# 파일 목록
ls -la

# 컴파일
gcc -o 프로그램이름 소스파일.c

# 실행
./프로그램이름

# 파일 내용 보기
cat 파일명

# 텍스트 파일 편집
nano 파일명
# 또는
vim 파일명

# 권한 부여
chmod +x 파일명
```

### WSL ↔ Windows 파일 접근

```bash
# WSL에서 Windows 파일 열기
cd /mnt/c/Users/dclab/Desktop

# Windows 탐색기에서 현재 WSL 디렉토리 열기
explorer.exe .

# Windows에서 WSL 파일 경로:
# \\wsl$\Ubuntu\home\dclab\
```

---

## 🎉 축하합니다!

WSL2 개발 환경 설정을 완료했습니다!

이제 본격적으로 OTT 스트리밍 서버 개발을 시작할 준비가 되었습니다.

**다음 단계**: Exercise 2, 3, 4를 차례대로 진행하세요!

궁금한 점이 있으면 언제든지 물어보세요! 😊
