# Watch Mode Demo - Step by Step

## What You'll See

This demo will show watch mode processing files automatically.

## Steps

### 1. Start Watch Mode

Open a terminal and run:
```batch
demo-watch-mode.bat
```

You should see:
```
========================================
 NORMALIZE WATCH MODE DEMO
========================================

Folders prepared:
  Input:  test-watch-input\
  Output: test-watch-output\

Starting normalize in watch mode...
Target: -14 LUFS (Spotify standard) with 99% peak limiting

Press Ctrl+C to stop watching

========================================
 DROP WAV FILES INTO test-watch-input\
========================================


normalize v0.253 (c) 2000-2004 Manuel Kasper <mk@neon1.net>.
All rights reserved.
smartpeak code by Lapo Luchini <lapo@lapo.it>.
LUFS support added 2025.

Watching folder: test-watch-input
Output folder: test-watch-output
Press Ctrl+C to stop...
```

### 2. In Another Terminal

Copy a test file into the watch folder:
```batch
cd C:\Users\camer\Documents\normalize-src
copy test-quiet.wav test-watch-input\
```

### 3. Watch the Magic

In the first terminal, you'll see:
```
New file detected: test-quiet.wav
Processing: test-quiet.wav
Pass 1: Calculating LUFS...
100%
Measured loudness: -32.5 LUFS
Target loudness: -14.0 LUFS
Performing amplification of 18.50 dB

Amplifying...
100%

Done.
Moved to output folder: test-quiet.wav
Time taken: 0.1 sec.
```

### 4. Check the Output

```batch
dir test-watch-output
```

You should see `test-quiet.wav` has been processed and moved!

### 5. Try More Files

```batch
copy test-medium.wav test-watch-input\
copy test-loud.wav test-watch-input\
```

Watch them get processed automatically!

### 6. Test Conflict Resolution

```batch
REM Copy the same file again (it was already moved to output)
copy test-watch-output\test-quiet.wav test-watch-input\
```

You'll see:
```
New file detected: test-quiet.wav
Processing: test-quiet.wav
...
File already exists in output, moving as: test-quiet_1.wav
Moved to output folder: test-quiet_1.wav
```

### 7. Stop Watch Mode

Press **Ctrl+C** in the watch mode terminal.

## What Just Happened?

✅ Normalize monitored the `test-watch-input` folder  
✅ Detected each new `.wav` file immediately  
✅ Applied LUFS normalization (-14 LUFS, Spotify standard)  
✅ Applied peak limiting (99% max)  
✅ Moved processed files to `test-watch-output`  
✅ Handled filename conflicts automatically  

## Real-World Application

This same process works for any workflow:

**Recording Studio:**
```batch
normalize -L -14 -m 99 -w "C:\Recordings\Raw" -O "C:\Recordings\Mastered"
```
Drop your recordings in `Raw`, get mastered files in `Mastered`!

**Podcast Production:**
```batch
normalize -L -16 -g 95 -w "C:\Podcast\Uploads" -O "C:\Podcast\Ready"
```
Guest uploads their file, it's automatically normalized!

**Batch Service:**
```batch
normalize -L -14 -m 99 -w "\\server\incoming" -O "\\server\processed" -q
```
Silent background processing for network folders!

## Cleanup

When done testing:
```batch
del test-watch-input\*.wav
del test-watch-output\*.wav
```

Or delete entirely:
```batch
rmdir /s test-watch-input
rmdir /s test-watch-output
```

## Next Steps

- Try with your own audio files
- Experiment with different settings
- Create custom batch scripts for your workflows
- Read `docs/WATCH_MODE.md` for advanced usage

**Enjoy automated audio processing!** 🎵
