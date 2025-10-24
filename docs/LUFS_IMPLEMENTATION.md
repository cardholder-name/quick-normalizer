# LUFS Implementation Summary

## Overview
Successfully implemented ITU-R BS.1770-4 compliant LUFS (Loudness Units relative to Full Scale) normalization for the normalize audio tool, with advanced percentile gating support.

**Note**: Implementation was corrected in October 2025 to fix biquad filter state handling and proper stereo channel processing.

## Recent Fixes (October 2025)

### 1. Biquad Filter State Correction
**Problem**: Filter states were storing input samples instead of intermediate values, causing incorrect K-weighting.

**Fix**: Changed to Direct Form II Transposed implementation:
```c
// Correct implementation
out = b0 * sample + z1;
z1 = b1 * sample - a1 * out + z2;
z2 = b2 * sample - a2 * out;
```

### 2. Proper Stereo Channel Handling
**Problem**: Stereo files were processed as a single stream, not per ITU-R BS.1770-4 spec.

**Fix**: Now processes each channel independently:
- Separate K-weighting filters for left and right channels
- Per-channel mean square calculation
- Proper averaging across channels: `(L_ms + R_ms) / 2`

### 3. Known Limitations
- **No true peak detection**: Uses simple sample peak instead of 4x oversampled true peak
  - Impact: May allow clipping on some DACs (~0.5dB error possible)
  - Mitigation: Use `-m 98` or `-m 99` for safety margin
- **Stereo only**: No support for 5.1/7.1 surround sound

## Files Modified

### normalize.c
- **Added ~500 lines** of new code
- **New Data Structures**:
  - `biquad_filter` - State and coefficients for IIR filtering
  - `k_weighting` - Pair of biquad filters for K-weighting
  
- **New Global Variables**:
  - `lufs_mode` - Flag to enable LUFS normalization
  - `target_lufs` - Target loudness level (default -16 LUFS)
  - `gate_percentile` - Percentile for smart gating (default 100%)

- **New Functions**:
  - `compare_double()` - qsort comparison for double values
  - `init_k_weighting()` - Calculate ITU-R BS.1770-4 filter coefficients
  - `apply_k_weighting()` - Process samples through K-weighting filters
  - `calculate_lufs8()` - LUFS calculation for 8-bit audio
  - `calculate_lufs16()` - LUFS calculation for 16-bit audio

- **Modified Functions**:
  - `main()` - Added `-L` and `-g` command line parsing
  - `process_file()` - Added LUFS calculation path with optional peak limiting
  - `usage()` - Added documentation for LUFS options

### New Files Created
- **build.bat** - Build script for easy compilation
- **README.md** - Comprehensive user documentation
- **.github/copilot-instructions.md** - Updated with LUFS architecture

## Technical Implementation Details

### K-Weighting Filters (ITU-R BS.1770-4)
Two cascaded biquad IIR filters:

1. **High-Shelf Pre-Filter**
   - Center frequency: 1681.974 Hz
   - Gain: +4.0 dB
   - Q factor: 0.7071
   - Purpose: Simulates head diffraction effect

2. **High-Pass RLB Filter**
   - Cutoff frequency: 38.13547 Hz
   - Q factor: 0.5
   - Purpose: Remove subsonic content

### Block-Based Analysis
- **Block size**: 400ms (sample_rate * 0.4)
- **Hop size**: 100ms (75% overlap)
- **Algorithm**: Sliding window with circular buffer
- **Memory efficient**: Reuses I/O buffer, allocates only for window and block loudness

### Gating Strategy
Three-stage gating process:

1. **Percentile Gate** (optional, `-g` flag)
   - Applied first if `gate_percentile < 100`
   - Sorts blocks by loudness
   - Keeps only quieter X% of blocks
   - Purpose: Ignore loud transients/peaks (similar to SmartPeak)

2. **Absolute Gate**
   - Threshold: -70 LUFS
   - Filters out blocks below threshold
   - Purpose: Ignore silence

3. **Relative Gate**
   - Threshold: Average loudness - 10 LU
   - Calculated from absolute-gated blocks
   - Filters blocks below relative threshold
   - Purpose: Ignore quiet passages per ITU-R standard

### Peak Limiting Integration
When `-m <percent>` is used with `-L`:
1. Calculate LUFS-based gain
2. Perform additional peak analysis pass
3. Calculate maximum safe gain to reach target peak level
4. Use minimum of LUFS gain and peak limit
5. Prevents clipping while achieving target loudness

## Command Line Interface

### New Options
```
-L <lufs>    Normalize to target LUFS loudness
-g <percent> Gate percentile: ignore loudest blocks (50-100%)
```

### Usage Examples
```batch
# Basic LUFS normalization
normalize -L -14 song.wav

# With percentile gating (ignore loud transients)
normalize -L -14 -g 95 podcast.wav

# With peak limiting (prevent clipping)
normalize -L -14 -m 98 music.wav

# Full control
normalize -L -16 -g 90 -m 99 *.wav
```

## Standards Compliance

### ITU-R BS.1770-4
✓ K-weighting filters with exact coefficients
✓ 400ms block size with 75% overlap
✓ Absolute gate at -70 LUFS
✓ Relative gate at -10 LU below average
✓ Mean-square power calculation

### Streaming Platform Targets
- Spotify: -14 LUFS
- YouTube: -13 to -16 LUFS
- Apple Music: -16 LUFS
- EBU R128 (Broadcast): -23 LUFS
- ATSC A/85 (US TV): -24 LUFS

## Performance Characteristics

### Memory Usage
- K-weighting state: ~96 bytes per channel
- Sliding window: sample_rate * 0.4 * 8 bytes
  - 44.1kHz: ~141 KB
  - 48kHz: ~154 KB
- Block loudness array: estimated_blocks * 8 bytes
  - For 3-minute file at 44.1kHz: ~14 KB

### Processing Speed
- Single pass through audio for K-weighting and block calculation
- Optional second pass for peak limiting
- Uses Windows VirtualAlloc for efficient large allocations
- Reuses existing I/O buffer infrastructure

## Key Innovations

### Percentile Gating
Unlike standard LUFS which only has absolute/relative gates:
- **SmartPeak for blocks**: Applies percentile logic to loudness blocks
- **Use case**: Podcasts with occasional shouting, music with loud intros
- **Example**: `-g 95` ignores the loudest 5% of blocks
- **Prevents**: Over-normalization from brief loud sections

### Backward Compatibility
- All original functionality preserved
- Peak and LUFS modes are mutually exclusive (validated in arg parsing)
- Can combine LUFS with `-m` (peak limiting)
- Can use `-g` with or without `-L` (intended for LUFS but doesn't break peak mode)

## Testing Recommendations

### Test Cases
1. **Silence handling**: File with silence at start/end
2. **Dynamic range**: Classical music with wide dynamic range
3. **Transients**: Drum hits, percussive content
4. **Sustained loudness**: Compressed pop/rock music
5. **Speech**: Podcasts with varying speakers
6. **Bit depths**: Test both 8-bit and 16-bit files
7. **Sample rates**: 44.1kHz, 48kHz, 96kHz

### Validation Tools
- Compare results with: ffmpeg loudnorm filter, Audacity LUFS meter
- Check for clipping: Visual inspection and peak analysis
- Verify LUFS accuracy: ±0.5 LU tolerance is acceptable

## Known Limitations

1. **Mono/Stereo only**: No support for 5.1 surround (would need channel weighting)
2. **No True Peak**: Uses sample peak, not intersample peak (could add 4x oversampling)
3. **Single precision**: Uses double for calculations, could be more precise
4. **Fixed blocks**: 400ms blocks are standard but could be configurable
5. **Memory**: Large files with long duration need significant memory for block array

## Future Enhancements

### Possible Additions
1. **True Peak limiting**: Oversample to detect intersample peaks
2. **Short-term LUFS**: Add `-S` flag for momentary/short-term loudness
3. **Loudness range**: Calculate and report LRA (EBU R128)
4. **Multi-channel**: Support 5.1 with proper channel weighting
5. **Progress bars**: More detailed progress for LUFS calculation
6. **Dry run**: `-n` flag to report levels without modifying files
7. **JSON output**: Machine-readable loudness data

## Compilation Verification

The code compiles without errors or warnings when built with:
```batch
cl /W3 /O2 /Fenormalize.exe normalize.c PCMWAV.C kernel32.lib
```

All functions are properly declared and implemented. The code maintains the original Windows-specific patterns and error handling conventions.
