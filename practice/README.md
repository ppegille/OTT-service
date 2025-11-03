# Practice Exercises - Quick Start Guide

Progressive hands-on exercises to learn HTTP Range Request streaming.

## 🚀 Quick Start

```bash
# 1. Build all exercises
make all

# 2. Create test files
make test_files

# 3. Run exercise 1
./exercise1

# In another terminal:
curl http://localhost:8080/
```

## 📚 Exercise Progression

| Exercise | Focus | File | Time |
|----------|-------|------|------|
| 1 | Basic HTTP Server | exercise1_hello_server.c | 20-30 min |
| 2 | Serve Static Files | exercise2_file_server.c | 20-30 min |
| 3 | Range Requests (Text) | exercise3_range_text.c | 45-60 min |
| 4 | Video Streaming | exercise4_video_server.c | 45-60 min |

## 📖 Detailed Instructions

See **EXERCISES.md** for:
- Complete code explanations
- Step-by-step testing instructions
- Expected outputs
- Troubleshooting tips
- Learning objectives

## ✅ Quick Test Commands

### Exercise 1: Basic HTTP Server
```bash
./exercise1
# Test: curl http://localhost:8080/
```

### Exercise 2: File Server
```bash
./exercise2
# Test: curl http://localhost:8080/test.txt
```

### Exercise 3: Range Requests (Text)
```bash
./exercise3
# Test: curl -r 0-99 http://localhost:8080/bigfile.txt
```

### Exercise 4: Video Streaming
```bash
./exercise4
# Test: Open http://localhost:8080/ in browser
```

## 🎯 Success Criteria

After completing all exercises, you should be able to:
- ✅ Create TCP server and handle connections
- ✅ Parse HTTP requests
- ✅ Serve files from disk
- ✅ Implement Range Request support
- ✅ Stream video with seeking

## 📝 Notes

- All servers use port 8080 by default
- Use Ctrl+C to stop servers
- Run one server at a time (port conflict)
- For exercise 4, provide your own test_video.mp4

## 🔧 Troubleshooting

**"Address already in use"**
```bash
lsof -ti:8080 | xargs kill -9
```

**Compilation errors**
```bash
# Make sure you have gcc installed
gcc --version

# On Windows, use WSL or MinGW
```

**Need help?**
```bash
make help
```

## ➡️ Next Steps

After completing exercises:
1. Review EXERCISES.md for detailed learning points
2. Read PREPARATION_GUIDE.md for deeper concepts
3. Start MVP Phase 1 implementation

**Ready? Say: "I'm ready for MVP Phase 1"**
