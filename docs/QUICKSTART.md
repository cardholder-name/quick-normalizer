# Quick Start Guide - LUFS Normalization

## Building the Tool

From a Visual Studio Developer Command Prompt:
```batch
cd c:\Users\camer\Documents\normalize-src
build.bat
```

Or manually:
```batch
cl /W3 /O2 /Fenormalize.exe normalize.c PCMWAV.C kernel32.lib
```

## Common Use Cases

### Music for Spotify
```batch
normalize -L -14 -m 99 *.wav
```
- Target: -14 LUFS (Spotify standard)
- Peak limit: 99% (prevents clipping)

### Podcast Normalization
```batch
normalize -L -16 -g 95 *.wav
```
- Target: -16 LUFS (common podcast standard)
- Gate: Ignore loudest 5% of blocks (handles shouting/laughter)

### YouTube Video Audio
```batch
normalize -L -13 -m 98 *.wav
```
- Target: -13 LUFS (YouTube preference)
- Peak limit: 98%

### Broadcast TV (EBU R128)
```batch
normalize -L -23 *.wav
```
- Target: -23 LUFS (EBU R128 standard)
- No peak limiting (assumes subsequent processing)

### Dynamic Music (Classical)
```batch
normalize -L -18 -g 90 *.wav
```
- Target: -18 LUFS (quieter for dynamic content)
- Gate: Ignore loudest 10% (preserve dynamics of quiet passages)

## Comparing Old vs New Behavior

### Old: Peak Normalization
```batch
normalize -m 95 music.wav
```
**Problem**: Files with brief loud peaks get under-normalized
**Result**: Perceived loudness varies between files

### New: LUFS Normalization
```batch
normalize -L -14 -g 95 music.wav
```
**Solution**: Measures perceptual loudness, ignores brief peaks
**Result**: Consistent perceived loudness across files

## Testing Your Implementation

### 1. Check Help Output
```batch
normalize -h
```
Should show new `-L` and `-g` options in the usage text.

### 2. Test Basic LUFS
```batch
normalize -L -16 test.wav
```
Should output:
```
Pass 1: Calculating LUFS loudness...
Measured loudness: -20.5 LUFS
Target loudness: -16.0 LUFS
Performing amplification of 4.50 dB
...
```

### 3. Test Percentile Gating
```batch
normalize -L -14 -g 90 test.wav
```
Should use only 90% of blocks (quietest blocks).

### 4. Test Peak Limiting
```batch
normalize -L -14 -m 98 test.wav
```
Should run two passes:
- Pass 1: Calculate LUFS
- Pass 1b: Find peaks for limiting

### 5. Verify Peak Mode Still Works
```batch
normalize -m 95 test.wav
normalize -s 90 -m 100 test.wav
```
Should work exactly as before (backward compatibility).

## Understanding the Output

### Example Output
```
Processing file C:\audio\song.wav
Pass 1: Calculating LUFS loudness...
100%
Measured loudness: -18.3 LUFS
Target loudness: -14.0 LUFS
Pass 1b: Finding peaks for limiting...
100%
Limiting gain to prevent clipping (0.8 dB reduction)
Performing amplification of 3.50 dB

Pass 2: Amplifying...
100%
Processed 10582974 bytes in 2.3 seconds.
```

**Explanation**:
1. LUFS measured: -18.3 LUFS
2. Target: -14.0 LUFS
3. Needed gain: +4.3 dB
4. Peak limiting detected clipping risk
5. Reduced gain to +3.5 dB (safe)
6. Final loudness: ~-14.8 LUFS (close to target, no clipping)

## Troubleshooting

### "Can't understand flag -L"
- You're running the old version
- Rebuild with the new source code

### LUFS result seems wrong
- Check sample rate (uncommon rates may have filter issues)
- Verify file is mono or stereo (multichannel not supported)
- Compare with reference tool (ffmpeg loudnorm)

### File sounds distorted after normalization
- Try adding peak limiting: `-m 98`
- Or reduce target LUFS: `-L -16` instead of `-L -14`
- Check if input file was already clipped

### "Out of memory" error
- Very long files need lots of memory for block array
- Try smaller files or increase virtual memory
- Typical: 3-minute file = ~14KB, 30-minute file = ~140KB

## Command Reference Card

```
LUFS NORMALIZATION
-L <lufs>     Set target loudness (-30 to 0)
-g <percent>  Ignore loudest X% blocks (50-100)
-m <percent>  Limit peaks to X% (with -L)

COMMON TARGETS
Spotify:      -14 LUFS
YouTube:      -13 to -16 LUFS
Podcasts:     -16 to -19 LUFS
Broadcast:    -23 LUFS (EBU R128)

TYPICAL COMMANDS
Basic:        normalize -L -14 file.wav
Safe:         normalize -L -14 -m 99 file.wav
Transients:   normalize -L -14 -g 95 file.wav
Full control: normalize -L -14 -g 95 -m 98 file.wav
```

## Batch Processing Tips

### Process entire music library
```batch
for /R C:\Music %%f in (*.wav) do normalize -L -14 -m 99 "%%f"
```

### Process with backup
```batch
for %%f in (*.wav) do (
    copy "%%f" "%%~nf_backup.wav"
    normalize -L -14 -m 99 "%%f"
)
```

### Create normalized versions
```batch
for %%f in (*.wav) do (
    normalize -L -14 -m 99 -o "%%~nf_normalized.wav" "%%f"
)
```

## Next Steps

1. Build the tool using `build.bat`
2. Test with a sample WAV file
3. Compare results with a reference LUFS meter
4. Process your audio library
5. Enjoy consistent loudness across all files!

For detailed technical information, see `LUFS_IMPLEMENTATION.md`.
