# Get Started with Practice Exercises

## 🎯 You Have Everything You Need!

Your practice environment is ready. Here's what to do:

### 📁 What's in This Directory

```
practice/
├── EXERCISES.md              ← Complete guide with ALL code
├── README.md                 ← Quick reference
├── GET_STARTED.md           ← This file
├── Makefile                  ← Build automation
├── exercise1_hello_server.c  ← Exercise 1 (ready)
├── exercise2_file_server.c   ← Exercise 2 (ready)
└── (exercises 3 & 4 code is in EXERCISES.md - copy from there)
```

### ⚡ Quick Start (3 Steps)

#### Step 1: Extract Exercise Code

The complete working code for exercises 3 and 4 is in **EXERCISES.md**.

**Option A: Copy from EXERCISES.md manually**
- Open EXERCISES.md
- Find exercise 3 code section (search for "exercise3_range_text.c")
- Copy the code block
- Create file `exercise3_range_text.c` and paste
- Repeat for exercise 4 and player.html

**Option B: I can create them for you**
- Just ask: "Create exercise 3 and 4 files"
- I'll extract and create all remaining files

#### Step 2: Build Exercise 1

```bash
cd practice

# Compile exercise 1
gcc -o exercise1 exercise1_hello_server.c

# Or use Makefile
make exercise1
```

#### Step 3: Run and Test

```bash
# Run exercise 1
./exercise1

# In another terminal, test it:
curl http://localhost:8080/
# Expected: HTML with "Hello, World!"
```

### 📚 Complete Exercise Path

Follow this order for best learning:

#### **Exercise 1: Basic HTTP Server** (20-30 min)
```bash
gcc -o exercise1 exercise1_hello_server.c
./exercise1

# Test:
curl http://localhost:8080/
```
**Goal**: Learn socket programming basics

---

#### **Exercise 2: File Server** (20-30 min)
```bash
# Create test file first
echo -e "Line 1\nLine 2\nLine 3\nLine 4\nLine 5" > test.txt

gcc -o exercise2 exercise2_file_server.c
./exercise2

# Test:
curl http://localhost:8080/test.txt
```
**Goal**: Learn file I/O and HTTP responses

---

#### **Exercise 3: Range Requests (Text)** (45-60 min)
```bash
# Create larger test file
for i in {1..100}; do echo "Line $i: ABCDEFGHIJKLMNOPQRSTUVWXYZ"; done > bigfile.txt

# Get code from EXERCISES.md, create exercise3_range_text.c
gcc -o exercise3 exercise3_range_text.c
./exercise3

# Test Range Requests:
curl -r 0-99 http://localhost:8080/bigfile.txt
curl -v -r 0-99 http://localhost:8080/bigfile.txt | grep "206"
```
**Goal**: Master Range Request implementation

---

#### **Exercise 4: Video Streaming** (45-60 min)
```bash
# Get a test video (any MP4 file, rename to test_video.mp4)
# Or download sample:
# curl -L "sample-url" -o test_video.mp4

# Get code from EXERCISES.md, create exercise4_video_server.c and player.html
gcc -o exercise4 exercise4_video_server.c
./exercise4

# Test: Open browser to http://localhost:8080/
```
**Goal**: Apply Range Requests to video streaming

### 🔧 Using the Makefile

```bash
# Build all exercises at once
make all

# Build specific exercise
make exercise1
make exercise2
make exercise3
make exercise4

# Create test files
make test_files

# Clean up
make clean

# Get help
make help
```

### ✅ Success Checklist

After each exercise, verify:

**Exercise 1:**
- [ ] Server starts without errors
- [ ] Browser shows "Hello, World!"
- [ ] curl gets 200 OK response
- [ ] Can handle multiple requests

**Exercise 2:**
- [ ] test.txt is served correctly
- [ ] Content-Length header matches file size
- [ ] 404 returned for missing files
- [ ] Full file content delivered

**Exercise 3:**
- [ ] Full file request returns 200 OK
- [ ] Range request returns 206 Partial Content
- [ ] Content-Range header present and correct
- [ ] curl -r 0-99 gets exactly 100 bytes
- [ ] Invalid range returns 416

**Exercise 4:**
- [ ] Video loads in browser
- [ ] Video plays smoothly
- [ ] Seeking works (can jump to different positions)
- [ ] Browser DevTools shows 206 responses
- [ ] Multiple seeks work correctly

### 📖 Where to Find Help

1. **Complete Code**: EXERCISES.md (all exercise code is there)
2. **Quick Reference**: README.md
3. **Concepts**: ../PREPARATION_GUIDE.md
4. **Build Help**: `make help`

### 🐛 Common Issues

**"Address already in use"**
```bash
# Kill process on port 8080
lsof -ti:8080 | xargs kill -9
```

**"gcc: command not found"**
```bash
# Install gcc (Ubuntu/Debian)
sudo apt install build-essential

# Or use WSL on Windows
wsl --install
```

**Can't find files**
```bash
# Make sure you're in the practice directory
pwd
# Should show: .../기말프로젝트/practice

# List files
ls -la
```

### 🎓 Learning Tips

1. **Read code before running**: Understand what each line does
2. **Modify and experiment**: Change buffer sizes, add printf statements
3. **Break things intentionally**: Remove error checking, see what happens
4. **Use debugger**: `gdb ./exercise1` to step through code
5. **Check server output**: Watch console for helpful messages

### 📊 Time Management

Estimated time per exercise (with learning):
- Exercise 1: 30 minutes
- Exercise 2: 30 minutes
- Exercise 3: 60 minutes (most important!)
- Exercise 4: 60 minutes

**Total: ~3 hours** (can spread across multiple sessions)

### ➡️ After Completing Exercises

When all 4 exercises work:

1. ✅ You understand HTTP Range Requests completely
2. ✅ You can implement 206 Partial Content responses
3. ✅ You're ready for MVP Phase 1

**Next step**: Say "I'm ready for MVP Phase 1"

---

## 🚀 Let's Start!

**Recommended first step:**

```bash
# Build and run exercise 1
gcc -o exercise1 exercise1_hello_server.c
./exercise1

# Test in another terminal:
curl http://localhost:8080/
```

**Expected output:**
```html
<html><body><h1>Hello, World!</h1><p>Exercise 1: Basic HTTP Server Working!</p></body></html>
```

**Got that working?** Move to Exercise 2!

**Need the remaining files created?** Just ask:
- "Create exercise 3 file"
- "Create exercise 4 and player files"
- "Create all remaining exercise files"

**Questions?** I'm here to help!
