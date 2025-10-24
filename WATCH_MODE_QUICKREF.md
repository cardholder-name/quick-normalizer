# Watch Mode - Quick Reference

## Start Watch Mode
```batch
normalize -L -14 -m 99 -w <input> -O <output>
```

## Common Commands

| Use Case | Command |
|----------|---------|
| **Spotify Standard** | `normalize -L -14 -m 99 -w input -O output` |
| **Podcast** | `normalize -L -16 -g 95 -w input -O output` |
| **YouTube** | `normalize -L -13 -m 98 -w input -O output` |
| **Peak (95%)** | `normalize -m 95 -w input -O output` |
| **SmartPeak** | `normalize -s 90 -m 100 -w input -O output` |
| **Silent Mode** | `normalize -L -14 -w input -O output -q` |
| **Background** | `start /min normalize -L -14 -w input -O output -q` |

## Quick Demo
```batch
demo-watch-mode.bat
```

## Stop Watching
Press **Ctrl+C**

## How It Works
1. Monitors input folder for `.wav` files
2. Processes with your specified settings
3. Moves to output folder automatically
4. Handles conflicts (_1, _2, etc.)
5. Continues until stopped

## Documentation
- Full Guide: `docs\WATCH_MODE.md`
- Testing: `TESTING_WATCH_MODE.md`
- Demo: `WATCH_MODE_DEMO.md`
- Technical: `WATCH_MODE_IMPLEMENTATION.md`
