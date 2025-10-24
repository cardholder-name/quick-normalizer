# Normalize - PCM WAV Audio Normalizer with LUFS Support

A Windows C application for normalizing PCM WAV audio files with support for both traditional peak normalization and modern LUFS (Loudness Units relative to Full Scale) normalization.

## Features

- **Peak Normalization**: Classic amplitude-based normalization
- **SmartPeak**: Statistical peak detection ignoring transient spikes
- **LUFS Normalization**: Perceptual loudness normalization (ITU-R BS.1770-4)
- **Percentile Gating**: For LUFS mode, ignore loudest blocks to handle transients
- **Peak Limiting**: Combine LUFS normalization with peak limiting to prevent clipping
- **Batch Processing**: Wildcard support for processing multiple files
- **8-bit and 16-bit**: Support for both PCM bit depths

## Building

### Using the Build Script
```batch
build.bat
```

### Manual Compilation
Requires Microsoft Visual C++ compiler:
```batch
cl /W3 /O2 /Fenormalize.exe normalize.c PCMWAV.C kernel32.lib
```

## Usage Examples

### Peak Normalization (Classic)
```batch
normalize -m 95 *.wav                # Normalize to 95% of maximum
normalize -s 90 -m 100 *.wav         # Use 90th percentile for peaks
normalize -a 6.0 music.wav           # Amplify by exactly 6dB
normalize -l 1.5 audio.wav           # Linear gain of 1.5x
```

### LUFS Normalization (Perceptual Loudness)
```batch
normalize -L -14 *.wav               # Normalize to -14 LUFS (Spotify)
normalize -L -23 *.wav               # Normalize to -23 LUFS (Broadcast EBU R128)
normalize -L -16 *.wav               # Normalize to -16 LUFS (YouTube)
normalize -L -14 -g 95 *.wav         # LUFS, ignore loudest 5% of blocks
normalize -L -14 -m 98 *.wav         # LUFS with peak limiting at 98%
normalize -L -18 -g 90 -m 99 *.wav   # Full control: LUFS + gating + limiting
```

## Command Line Options

```
-L <lufs>    Normalize to target LUFS loudness (e.g., -14 for Spotify)
-g <percent> Gate percentile for LUFS: ignore loudest blocks (50-100%)
-m <percent> Normalize to <percent> % of maximum (default 100)
-s <percent> SmartPeak: use percentile for peak detection (50-100%)
-a <level>   Amplify by exact dB level
-l <ratio>   Multiply samples by linear ratio
-x <level>   Abort if gain change is smaller than <level> dB
-b <size>    I/O buffer size in KB (16-16384, default 64)
-o <file>    Write output to <file> instead of overwriting
-p           Prompt before normalization
-q           Quiet mode (no screen output)
-d           Don't abort batch on user skip
-h           Display help
```

## LUFS Standards Reference

| Platform/Standard | Target LUFS | Use Case |
|------------------|-------------|----------|
| Spotify          | -14 LUFS    | Music streaming |
| YouTube          | -13 to -16  | Video content |
| Apple Music      | -16 LUFS    | Music streaming |
| Amazon Music     | -14 LUFS    | Music streaming |
| EBU R128         | -23 LUFS    | Broadcast television |
| ATSC A/85        | -24 LUFS    | US broadcast |
| Podcast          | -16 to -19  | Voice content |

## Technical Details

### LUFS Algorithm
- **K-Weighting Filters**: ITU-R BS.1770-4 compliant
  - High-shelf pre-filter at 1682 Hz (+4dB)
  - High-pass RLB filter at 38 Hz
- **Block Processing**: 400ms blocks with 75% overlap (100ms hop)
- **Absolute Gate**: -70 LUFS (ignores silence)
- **Relative Gate**: -10 LU below average (ignores quiet passages)

### SmartPeak Algorithm
- Builds histogram of all sample values
- Calculates specified percentile as effective peak
- Prevents over-normalization from brief transients

## License

GNU General Public License v2 (GPL-2.0)

Original normalize v0.253 (c) 2000-2004 Manuel Kasper  
SmartPeak code by Lapo Luchini  
LUFS implementation added 2025

## Error Codes

- 0 = Success
- 1 = I/O error
- 2 = Parameter error
- 3 = No amplification required
- 4 = Out of memory
- 5 = User abort
