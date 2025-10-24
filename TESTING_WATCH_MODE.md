# Testing Watch Mode

This guide will help you test the watch mode feature.

## Quick Test

1. **Build normalize** (if you haven't already):
   ```batch
   build-msvc.bat
   ```

2. **Generate test WAV files**:
   ```batch
   build-test-wav.bat
   test-generate-wav.exe
   ```
   This creates three test files:
   - `test-quiet.wav` - 10% amplitude (very quiet)
   - `test-medium.wav` - 50% amplitude (medium)
   - `test-loud.wav` - 90% amplitude (loud)

3. **Run the demo script**:
   ```batch
   demo-watch-mode.bat
   ```

4. **Test the watch mode**:
   - Open another command prompt
   - Copy one of the test files into `test-watch-input\`:
     ```batch
     copy test-quiet.wav test-watch-input\
     ```
   - Watch the first terminal for processing output
   - Check `test-watch-output\` for the normalized file

5. **Stop watching**:
   - Press **Ctrl+C** in the watch mode terminal

## Manual Testing

### Basic Watch Mode
```batch
normalize -L -14 -m 99 -w test-watch-input -O test-watch-output
```

### With Quiet Mode
```batch
normalize -L -14 -m 99 -w test-watch-input -O test-watch-output -q
```

### Peak Normalization
```batch
normalize -m 95 -w test-watch-input -O test-watch-output
```

### SmartPeak
```batch
normalize -s 90 -m 100 -w test-watch-input -O test-watch-output
```

## What to Observe

### Successful Processing
You should see output like:
```
Watching folder: test-watch-input
Output folder: test-watch-output
Press Ctrl+C to stop...

New file detected: test-quiet.wav
Processing: test-quiet.wav
Measured loudness: -32.5 LUFS
Target loudness: -14.0 LUFS
Performing amplification of 18.50 dB
Moved to output folder: test-quiet.wav
```

### File Conflict Resolution
If you copy the same file twice:
```
New file detected: test-quiet.wav
Processing: test-quiet.wav
File already exists in output, moving as: test-quiet_1.wav
Moved to output folder: test-quiet_1.wav
```

### Error Handling
If a file can't be processed (e.g., not a valid WAV):
```
New file detected: bad-file.wav
Error: Cannot open file: bad-file.wav
```

## Testing Scenarios

### 1. Single File Processing
- Copy `test-quiet.wav` to watch folder
- Verify it's processed and moved
- Check output file plays correctly

### 2. Multiple Files
- Copy all three test files at once
- Verify they're processed sequentially
- Check all output files exist

### 3. File Name Conflicts
- Copy `test-quiet.wav` twice (with different names)
- Rename both to same name and copy
- Verify `_1`, `_2` suffixes are added

### 4. Different Normalization Modes
Test with different settings:
- `-L -14` (LUFS)
- `-L -16 -g 95` (LUFS with gating)
- `-m 95` (peak)
- `-s 90 -m 100` (SmartPeak)

### 5. Quiet Mode
- Run with `-q` flag
- Verify no output except errors
- Check files are still processed

### 6. Large Files
- Create longer WAV files (use audio software)
- Verify watch mode handles them correctly
- Check file readiness detection works

## Cleanup

After testing:
```batch
del test-watch-input\*.wav
del test-watch-output\*.wav
```

Or delete the folders entirely:
```batch
rmdir /s test-watch-input
rmdir /s test-watch-output
```

## Troubleshooting

### "Cannot open watch folder"
- Verify `test-watch-input` exists
- Check you have read permissions
- Try with quotes: `-w "test-watch-input"`

### Files Not Detected
- Ensure files have `.wav` extension
- Try copying instead of moving files
- Check if files are locked by another program

### Files Process But Aren't Moved
- Verify you specified `-O <folder>`
- Check output folder has write permissions
- Look for error messages in output

### Watch Mode Doesn't Start
- Ensure normalize.exe was built successfully
- Try running without watch mode first
- Check command line syntax

## Expected Performance

- **Detection**: Immediate (within 100ms)
- **File Readiness**: Up to 5 seconds wait
- **Processing**: Depends on file size (typically <1 second for test files)
- **Moving**: Instant (same drive) or fast (different drive)

## Notes

- Watch mode processes one file at a time
- Files remain in input folder if processing fails
- Output folder is created automatically if it doesn't exist
- Watch mode runs indefinitely until Ctrl+C
- File system events may trigger multiple times (handled gracefully)

## Next Steps

Once testing is successful:
- Use with your real audio files
- Set up automated workflows
- Create batch scripts for common tasks
- Consider running as background service

See `docs/WATCH_MODE.md` for production usage examples.
