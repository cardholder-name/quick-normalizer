# Normalize Audio Tool - Copilot Instructions

## Project Overview

This is a Windows C application for normalizing PCM WAV audio files.

**Original normalize v0.253** (2000-2004)  
- Created by Manuel Kasper <mk@neon1.net>
- SmartPeak algorithm by Lapo Luchini <lapo@lapo.it>

**2025 Enhancements**  
- LUFS normalization and watch mode by Cam St Clair with Claude (Anthropic)
- ITU-R BS.1770-4 compliant loudness measurement
- Automated folder monitoring for unattended processing

The tool can analyze audio using peak detection or perceptual loudness (LUFS), apply appropriate amplification, and monitor folders for automated processing workflows.

## Architecture & Key Components

### Core Files
- **`normalize.c`** (1,680+ lines): Main application logic, command line parsing, file processing pipeline, watch mode
- **`PCMWAV.H`/`PCMWAV.C`**: Custom WAV file I/O library with Windows-specific file handling (original code by Manuel Kasper)
- **`COPYING.txt`**: GPL v2 license

### Data Flow Pipeline
1. **File Discovery**: 
   - **Batch Mode**: Uses Windows `_findfirst`/`_findnext` for wildcard file processing
   - **Watch Mode**: Uses `ReadDirectoryChangesW` for real-time folder monitoring
2. **WAV Parsing**: Custom RIFF/WAVE parser that validates PCM format and extracts metadata
3. **Analysis Pass**: 
   - **Peak Mode**: Two-pass algorithm - first pass finds peaks, second pass applies amplification
   - **LUFS Mode**: Calculates perceptual loudness using ITU-R BS.1770-4 K-weighting filters with 400ms blocks
4. **Amplification**: Uses lookup tables for performance (8-bit and 16-bit variants)
5. **Watch Mode Output**: Automatically moves processed files to output folder with conflict resolution

## Critical Implementation Patterns

### Bit Depth Handling
The codebase has parallel implementations for 8-bit and 16-bit audio:
- `getpeaks8()` vs `getpeaks16()` - peak detection algorithms
- `amplify8()` vs `amplify16()` - amplification with lookup tables
- `make_table8()` vs `make_table16()` - pre-computed amplification tables

### Memory Management
Uses Windows-specific `VirtualAlloc`/`VirtualFree` for large buffers:
```c
// For smartpeak statistics (16-bit needs 65536 * sizeof(unsigned long))
stats = (unsigned long*)VirtualAlloc(NULL, sizeof(unsigned long) * 65536, MEM_COMMIT, PAGE_READWRITE);
```

### SmartPeak Algorithm
When `-s <percent>` is specified, uses statistical analysis instead of absolute peaks:
- Builds histogram of sample values during first pass
- Calculates percentile-based peaks to ignore transient spikes
- Modifies `normpercent *= peakpercent / 100.0` for correct amplification

### LUFS Algorithm (ITU-R BS.1770-4)
When `-L <target_lufs>` is specified, uses perceptual loudness normalization:
- **K-Weighting Filters**: Two-stage biquad filters (high-shelf at 1682Hz, high-pass at 38Hz)
- **Block Processing**: 400ms blocks with 75% overlap (100ms hop)
- **Gating**: Absolute gate at -70 LUFS, relative gate at -10 LU below average
- **Percentile Gating**: Optional `-g <percent>` ignores loudest blocks (like SmartPeak for transients)
- **Peak Limiting**: Use `-m <percent>` with `-L` to prevent clipping

### Watch Mode (Automated Folder Monitoring)
When `-w <folder> -O <output>` is specified, enables continuous folder monitoring:
- **File Detection**: Uses Windows `ReadDirectoryChangesW` API for real-time monitoring
- **File Readiness**: Waits up to 5 seconds for files to be completely written (handles locked files)
- **Processing**: Applies all normalization settings specified on command line
- **Auto-Move**: Moves successfully processed files to output folder
- **Conflict Resolution**: Appends `_1`, `_2`, etc. for duplicate filenames
- **Error Handling**: Failed files remain in watch folder, processing continues
- **Continuous Operation**: Runs until Ctrl+C, processes files sequentially

### Error Handling Convention
Specific error codes with semantic meaning:
- 0 = success, 1 = I/O error, 2 = parameter error  
- 3 = no amplification needed, 4 = out of memory, 5 = user abort

## Development Workflow

### Building
Use the provided `build.bat` or compile manually with MSVC:
```bash
cl /W3 /O2 /Fenormalize.exe normalize.c PCMWAV.C kernel32.lib
```
Links against Windows APIs (kernel32.lib for file I/O).

### Key Command Line Patterns
```bash
# Peak normalization (classic mode)
normalize -m 95 *.wav          # Normalize to 95% of max
normalize -s 90 -m 100 *.wav   # Use 90th percentile peaks  
normalize -a 6.0 file.wav      # Amplify by 6dB
normalize -l 1.5 file.wav      # Linear amplification by 1.5x

# LUFS normalization (perceptual loudness)
normalize -L -14 *.wav         # Spotify standard (-14 LUFS)
normalize -L -23 *.wav         # Broadcast standard (EBU R128)
normalize -L -16 -g 95 *.wav   # Ignore loudest 5% of blocks
normalize -L -14 -m 98 *.wav   # LUFS with peak limiting at 98%

# Watch mode (automated folder monitoring)
normalize -L -14 -m 99 -w C:\incoming -O C:\processed    # LUFS watch mode
normalize -m 95 -w input -O output                       # Peak watch mode
normalize -L -16 -g 95 -w raw -O ready -q                # Silent watch mode
```

## Code Conventions

### Buffer Management
- Global buffer `buf` cast to `buf8`/`buf16` for type-specific access
- Configurable I/O buffer size via `-b` flag (16KB to 16MB)
- Always check `pcmwav_read`/`pcmwav_write` return values

### Progress Reporting  
Consistent pattern for long operations:
```c
npercent = (int)(100.0 * ((double)ndone / (double)total));
if (npercent > lastn) {
    fprintf(stderr, "\r%d%%", npercent);
    fflush(stderr);
    lastn = npercent;
}
```

### Windows-Specific Code
- Uses `#include <windows.h>` and Win32 APIs throughout
- File paths use Windows conventions (`_MAX_PATH`, `_splitpath`)
- Handle invalid file handles with `INVALID_HANDLE_VALUE`

When modifying this code, maintain the dual 8-bit/16-bit architecture, preserve Windows API usage patterns, and ensure error codes remain consistent with the established convention.