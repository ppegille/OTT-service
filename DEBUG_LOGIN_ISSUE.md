# 로그인 문제 디버깅 가이드

**작성일**: 2025-11-03
**문제**: 로그인 화면에서 넘어가지 않음

---

## 🔍 추가된 디버깅 코드

### 수정된 파일
1. **server/src/main.c** - POST /login 처리 디버깅
2. **server/src/session.c** - handle_login 함수 디버깅

### 디버깅 메시지
```
[DEBUG] POST /login received
[DEBUG] Body found, calling handle_login
[DEBUG] POST body: 'user_id=testuser'
[DEBUG] Parsed user_id: 'testuser'
✓ Session created: ...
✓ Login successful: testuser → session ...
```

---

## 🚀 재컴파일 및 테스트

### 1단계: 서버 재컴파일

**WSL2에서**:
```bash
cd /mnt/c/Users/dclab/Desktop/2025_2학기/네트워크프로그래밍/기말프로젝트
cd server
make clean
make
```

### 2단계: 서버 실행
```bash
./ott_server
```

### 3단계: 브라우저에서 로그인 시도
1. Chrome → `http://localhost:8080/`
2. 로그인 페이지에서 ID 입력 (예: `testuser`)
3. 로그인 버튼 클릭

### 4단계: 서버 로그 확인

**정상 로그인 시 예상 출력**:
```
✓ Client connected: 127.0.0.1 (PID: 1234)
  [Child 1235] POST /login
  [DEBUG] POST /login received
  [DEBUG] Body found, calling handle_login
  [DEBUG] POST body: 'user_id=testuser'
  [DEBUG] Parsed user_id: 'testuser'
✓ Session created: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx for user 'testuser' (total: 1)
✓ Login successful: testuser → session xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  [Child 1235] Connection closed
```

---

## 🐛 가능한 문제 및 해결

### 문제 1: POST body가 비어있음
**로그**:
```
[DEBUG] POST body: ''
[DEBUG] Parsed user_id: '(null)'
❌ Login failed: empty user_id
```

**원인**: POST 요청 body가 제대로 전달되지 않음

**해결**:
1. login.html의 form method 확인
2. Content-Type 헤더 확인
3. 브라우저 개발자 도구 → Network 탭에서 POST 요청 확인

### 문제 2: Body는 있지만 user_id 파싱 실패
**로그**:
```
[DEBUG] POST body: 'something_else=value'
[DEBUG] Parsed user_id: '(null)'
```

**원인**: POST body에 `user_id=` 형식이 없음

**해결**:
1. login.html의 input name 확인 (name="user_id"여야 함)
2. form enctype 확인

### 문제 3: 302 Redirect가 작동하지 않음
**로그**:
```
✓ Login successful: testuser → session xxx...
  [Child 1235] Connection closed
```
(로그는 성공했지만 브라우저가 리다이렉트 안됨)

**원인**: 브라우저가 302 응답을 무시

**해결**:
1. Chrome DevTools → Network 탭 확인
2. Response Headers에 Location: / 확인
3. Set-Cookie 헤더 확인

### 문제 4: No body found
**로그**:
```
[DEBUG] POST /login received
[DEBUG] No body found, redirecting to login
```

**원인**: HTTP 요청에 `\r\n\r\n` (헤더/바디 구분자)가 없음

**해결**:
1. 브라우저가 올바른 HTTP 요청을 보내는지 확인
2. 서버가 전체 요청을 읽었는지 확인

---

## 🧪 수동 테스트 (curl)

### POST 요청 테스트
```bash
curl -v -X POST http://localhost:8080/login \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "user_id=testuser"
```

**예상 응답**:
```http
HTTP/1.1 302 Found
Location: /
Set-Cookie: session_id=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx; Path=/; Max-Age=1800; HttpOnly
Content-Length: 0
```

---

## 📊 Chrome DevTools로 확인

### 1. Network 탭
1. F12 → Network 탭
2. 로그인 버튼 클릭
3. POST /login 요청 확인

**확인 사항**:
- Request Method: POST
- Request Payload: `user_id=testuser`
- Response Status: 302 Found
- Response Headers: Location: /
- Response Headers: Set-Cookie: session_id=...

### 2. Console 탭
JavaScript 에러 확인

### 3. Application → Cookies
로그인 후 `session_id` 쿠키 확인

---

## 🔧 login.html 확인

올바른 form 구조:
```html
<form method="POST" action="/login">
    <input type="text" name="user_id" required>
    <button type="submit">로그인</button>
</form>
```

**중요**:
- method="POST" (대문자 또는 소문자)
- action="/login"
- input name="user_id" (정확히 일치)

---

## 📝 로그 해석 가이드

### 성공 케이스
```
1. [Child XXX] POST /login          ← POST 요청 받음
2. [DEBUG] POST /login received     ← 핸들러 진입
3. [DEBUG] Body found               ← body 파싱 성공
4. [DEBUG] POST body: 'user_id=...' ← body 내용 확인
5. [DEBUG] Parsed user_id: '...'    ← user_id 파싱 성공
6. ✓ Session created                ← 세션 생성 성공
7. ✓ Login successful               ← 로그인 성공
8. [Child XXX] Connection closed    ← 연결 종료
```

### 실패 케이스 1: Body 없음
```
1. [Child XXX] POST /login
2. [DEBUG] POST /login received
3. [DEBUG] No body found            ← 문제!
4. [Child XXX] Connection closed
```

### 실패 케이스 2: user_id 없음
```
1. [Child XXX] POST /login
2. [DEBUG] POST /login received
3. [DEBUG] Body found
4. [DEBUG] POST body: 'wrong_param=value'  ← 문제!
5. [DEBUG] Parsed user_id: '(null)'        ← 파싱 실패
6. ❌ Login failed: empty user_id
```

---

## 🎯 다음 단계

### 로그 확인 후 보고

테스트 후 다음 정보를 제공해주세요:

1. **서버 로그 전체** (로그인 시도 부분)
2. **Chrome DevTools Network 탭** 스크린샷 (POST /login 요청)
3. **발생한 문제** 설명

예시:
```
"서버 로그에 이렇게 나왔습니다:
[DEBUG] POST body: ''
[DEBUG] Parsed user_id: '(null)'
❌ Login failed: empty user_id

Chrome Network 탭에서는 POST 요청이 성공했다고 나옵니다."
```

---

## 💡 빠른 해결책 시도

### 방법 1: 브라우저 캐시 삭제
1. Ctrl + Shift + Delete
2. 캐시 삭제
3. 다시 시도

### 방법 2: 시크릿 창 사용
1. Ctrl + Shift + N (시크릿 창)
2. http://localhost:8080/ 접속
3. 로그인 시도

### 방법 3: curl로 먼저 테스트
```bash
# 세션 획득
SESSION_ID=$(curl -i -X POST http://localhost:8080/login \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "user_id=testuser" 2>&1 | \
  grep -i "set-cookie" | \
  sed 's/.*session_id=\([^;]*\).*/\1/')

echo "Session ID: $SESSION_ID"

# 세션으로 플레이어 접속
curl -i http://localhost:8080/ \
  -H "Cookie: session_id=$SESSION_ID"
```

---

**작성일**: 2025-11-03
**목적**: 로그인 문제 진단 및 해결
**다음 단계**: 로그 확인 후 문제 파악

**재컴파일 후 다시 테스트해주세요! 🔍**
