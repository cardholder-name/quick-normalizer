# Watch Mode - Automated Folder Monitoring

## Overview

Watch mode allows `normalize` to continuously monitor a folder for new WAV files and automatically process them using your specified settings. Perfect for automated workflows, dropbox scenarios, and unattended processing.

## Basic Usage

```batch
normalize [normalization-options] -w <input-folder> -O <output-folder>
```

**Example:**
```batch
normalize -L -14 -m 99 -w C:\incoming -O C:\processed
```

This will:
1. Watch `C:\incoming` for new `.wav` files
2. Normalize each file to -14 LUFS with 99% peak limiting
3. Move processed files to `C:\processed`
4. Continue running until you press Ctrl+C

## How It Works

### File Detection
- Monitors folder using Windows `ReadDirectoryChangesW` API
- Detects new files and file modifications
- Only processes `.wav` files (case-insensitive)

### File Readiness Check
- Waits for files to be completely written
- Handles locked files gracefully
- Up to 5-second timeout for file availability
- Extra 500ms delay after file is unlocked

### Processing
- Applies all normalization settings specified on command line
- Same processing as batch mode
- Errors don't stop the watch loop

### Output Handling
- Moves successfully processed files to output folder
- Creates output folder if it doesn't exist
- Handles filename conflicts automatically (appends `_1`, `_2`, etc.)
- Failed files remain in watch folder

## Command Examples

### LUFS Normalization

**Spotify Standard:**
```batch
normalize -L -14 -m 99 -w "Drop Here" -O "Spotify Ready"
```

**Podcast Production:**
```batch
normalize -L -16 -g 95 -w incoming -O ready
```

**YouTube Content:**
```batch
normalize -L -13 -m 98 -w raw -O youtube
```

**Broadcast:**
```batch
normalize -L -23 -w source -O broadcast
```

### Peak Normalization

**Classic 95% Peak:**
```batch
normalize -m 95 -w input -O output
```

**SmartPeak (ignore top 10%):**
```batch
normalize -s 90 -m 100 -w input -O output
```

**Fixed Amplification:**
```batch
normalize -a 6.0 -w input -O output
```

### Quiet Mode

Run silently in the background:
```batch
normalize -L -14 -m 99 -w input -O output -q
```

## Workflow Integration

### 1. Recording Studio Workflow
```batch
# Terminal 1: Recording software saves to 'raw' folder
# Terminal 2: Auto-normalize to Spotify standard
normalize -L -14 -m 99 -w C:\recordings\raw -O C:\recordings\mastered
```

### 2. Podcast Production
```batch
# Process uploads with smart gating for dynamic speech
normalize -L -16 -g 95 -w "C:\Dropbox\Podcast Uploads" -O "C:\Dropbox\Ready to Publish"
```

### 3. Batch Service
```batch
# Set up as a service for continuous processing
normalize -L -14 -m 99 -w \\server\incoming -O \\server\processed -q
```

### 4. Music Library Preparation
```batch
# Normalize entire collections being copied in
normalize -L -14 -m 99 -w "E:\New Music" -O "E:\Library"
```

## Advanced Features

### Filename Conflict Resolution
If a file with the same name exists in output folder:
- Original: `song.wav` → `song.wav`
- Duplicate: `song.wav` → `song_1.wav`
- Next: `song.wav` → `song_2.wav`

### Error Handling
Files that fail to process:
- Remain in the watch folder
- Error message displayed (unless `-q`)
- Watch mode continues running
- Can be re-processed by moving them out and back

### File Locking
- Gracefully handles files being written
- Waits for exclusive file access
- 5-second timeout prevents infinite waits
- Safe for network folders

## Stopping Watch Mode

Press **Ctrl+C** to stop monitoring and exit gracefully.

## Tips & Best Practices

### 1. Test Your Settings First
Before running watch mode, test your normalization settings on sample files:
```batch
# Test
normalize -L -14 -m 99 test.wav

# If good, use in watch mode
normalize -L -14 -m 99 -w input -O output
```

### 2. Use Absolute Paths
For reliability, especially with network folders:
```batch
normalize -L -14 -w C:\full\path\to\input -O C:\full\path\to\output
```

### 3. Create Folders First
Output folder is created automatically, but watch folder must exist:
```batch
mkdir input
mkdir output
normalize -L -14 -w input -O output
```

### 4. Monitor the Process
Without `-q` flag, you can see each file being processed:
```batch
normalize -L -14 -m 99 -w input -O output
```

Output shows:
```
New file detected: song.wav
Processing: song.wav
Measured loudness: -20.3 LUFS
Target loudness: -14.0 LUFS
Performing amplification of 6.30 dB
Moved to output folder: song.wav
```

### 5. Background Processing
Use `-q` for silent operation:
```batch
start /min normalize -L -14 -w input -O output -q
```

### 6. Network Folders
Works with UNC paths:
```batch
normalize -L -14 -w \\server\share\input -O \\server\share\output
```

## Limitations

1. **Windows Only**: Uses Windows-specific file monitoring APIs
2. **WAV Files Only**: Only `.wav` files are processed
3. **Single Folder**: Doesn't monitor subfolders (non-recursive)
4. **File Modifications**: Triggers on both new files and modifications (may process twice)
5. **No Undo**: Processed files are moved immediately

## Troubleshooting

### "Cannot open watch folder"
- Check folder exists
- Verify you have read permissions
- Try with quotes: `normalize -w "My Folder" -O output`

### "Error: Watch mode requires -O"
- Must specify output folder with `-O`
- Output folder can be relative or absolute

### Files Not Being Detected
- Verify files have `.wav` extension
- Check files aren't locked by another program
- Try copying files instead of moving them into watch folder

### Files Process Twice
- File modification triggers re-processing
- Use copy operation instead of save-in-place
- Check your recording software settings

### Network Drive Issues
- Ensure stable network connection
- Use UNC paths: `\\server\share` not drive letters
- Some network drives don't support change notifications

## Performance Considerations

### CPU Usage
- Minimal when idle (waiting for changes)
- Spikes during processing (expected)
- One file processed at a time

### Memory Usage
- Similar to normal mode
- No accumulation over time
- Each file processed independently

### Disk I/O
- Two passes for peak mode (read twice)
- One pass for LUFS mode (read once)
- Efficient move operation (same drive)

## Use Cases

### ✅ Perfect For:
- Recording studio post-processing
- Podcast upload automation
- Music library organization
- Batch service deployments
- Dropbox/cloud folder processing
- Unattended workstations

### ❌ Not Ideal For:
- Interactive editing workflows
- Real-time processing
- Very high-frequency file arrivals
- Multi-format processing
- Complex conditional logic

## Future Enhancements

Potential additions (not yet implemented):
- [ ] Subfolder monitoring (recursive)
- [ ] Multiple folder support
- [ ] File filters beyond .wav
- [ ] Pre/post processing hooks
- [ ] Logging to file
- [ ] Windows service wrapper
- [ ] Web dashboard

## Examples by Industry

### Music Production
```batch
# Master tracks for streaming
normalize -L -14 -m 99 -w "Export" -O "Masters"
```

### Podcast Production
```batch
# Process uploads with voice optimization
normalize -L -16 -g 95 -w "Raw Episodes" -O "Ready to Publish"
```

### Broadcast
```batch
# EBU R128 compliance
normalize -L -23 -w "Ingest" -O "Broadcast"
```

### Archival
```batch
# Preserve dynamic range, gentle normalization
normalize -L -18 -g 90 -w "Digitized" -O "Archive"
```

## Summary

Watch mode transforms `normalize` from a command-line tool into an automated processing service. Perfect for any workflow where audio files need consistent, unattended normalization.

**Key Benefits:**
- 🔄 Continuous operation
- 🤖 Fully automated
- 🎯 Same quality as batch mode
- 🛡️ Robust error handling
- 📁 Clean file management

Start watching and let the tool do the work!
