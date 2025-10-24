# 🎵 Normalize - Audio Normalization Tool

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![Platform](https://img.shields.io/badge/platform-Windows-blue)]()
[![Language: C](https://img.shields.io/badge/language-C-brightgreen.svg)]()

A high-performance Windows audio normalization tool with modern **LUFS (Loudness Units relative to Full Scale)** support, perfect for preparing audio for streaming platforms, podcasts, and broadcast.

## ✨ Features

### 🎯 LUFS Normalization (NEW!)
- **ITU-R BS.1770-4 compliant** perceptual loudness measurement
- **K-weighting filters** for accurate human hearing simulation
- **Smart gating** with percentile-based block filtering
- **Peak limiting** to prevent clipping while achieving target loudness
- Ready for **Spotify** (-14 LUFS), **YouTube** (-13 LUFS), **Broadcast** (-23 LUFS)

### 📊 Peak Normalization (Classic)
- Traditional amplitude-based normalization
- **SmartPeak** algorithm ignores transient spikes using statistical analysis
- Configurable normalization percentage
- Direct amplification in dB or linear gain

### 🔄 Watch Mode (NEW!)
- **Automated folder monitoring** for unattended processing
- Drop files into watch folder, get normalized output automatically
- Perfect for recording studios, podcasts, and batch workflows
- Handles file locking and conflict resolution gracefully

### ⚡ Performance
- 💨 Lightweight executable (~50KB)
- 🚀 Zero runtime dependencies
- ⚙️ Optimized DSP with lookup tables
- 📦 Batch processing with wildcard support

## 📁 Format Compatibility

### Supported Formats
- **PCM WAV files** (uncompressed audio)
- **8-bit PCM**: Unsigned samples (0-255)
- **16-bit PCM**: Signed samples (-32768 to 32767)
- **Mono and Stereo**: Both channel configurations supported
- **Any sample rate**: 8kHz, 16kHz, 44.1kHz, 48kHz, 96kHz, etc.

### Not Supported
- ❌ Compressed formats (MP3, AAC, FLAC, OGG, etc.)
- ❌ 24-bit or 32-bit PCM WAV
- ❌ Multi-channel audio (5.1, 7.1 surround)

## 🚀 Quick Start

### Installation

1. **Install Build Tools** (one-time):
   ```powershell
   winget install Microsoft.VisualStudio.2022.BuildTools
   ```

2. **Build the tool**:
   ```batch
   build-msvc.bat
   ```

3. **Done!** You now have `normalize.exe`

### Usage Examples

```batch
# Spotify standard (-14 LUFS with peak limiting)
normalize -L -14 -m 99 *.wav

# YouTube standard
normalize -L -13 -m 98 video_audio.wav

# Podcast (ignore loud laughter/shouting with 95% gating)
normalize -L -16 -g 95 podcast.wav

# Broadcast standard (EBU R128)
normalize -L -23 broadcast.wav

# Classic peak normalization
normalize -m 95 *.wav

# SmartPeak (ignore top 10% of peaks)
normalize -s 90 -m 100 music.wav

# Watch folder for automatic processing
normalize -L -14 -m 99 -w C:\incoming -O C:\processed
```

## 🎯 LUFS Standards Reference

### LUFS Normalization
```
-L <lufs>      Target loudness in LUFS (e.g., -14 for Spotify)
-g <percent>   Gate percentile: ignore loudest X% of blocks (50-100)
-m <percent>   Peak limiting: don't exceed X% of maximum
```

### Peak Normalization
```
-m <percent>   Normalize to X% of maximum (default 100)
-s <percent>   SmartPeak: use percentile for peak detection (50-100)
-a <level>     Amplify by exact dB amount
-l <ratio>     Linear gain multiplication
```

### Other Options
```
-w <folder>    Watch folder mode: process files automatically
-O <folder>    Output folder for watch mode (required with -w)
-b <size>      I/O buffer size in KB (16-16384, default 64)
-o <file>      Output to file instead of overwriting
-p             Prompt before normalization
-q             Quiet mode (no output)
-d             Don't abort batch on skip
-x <level>     Skip if gain is less than X dB
-h             Show help
```

## 🎯 LUFS Standards Reference

| Platform/Use | Target LUFS | Command Example |
|--------------|-------------|-----------------|
| **Spotify** | -14 LUFS | `normalize -L -14 -m 99 *.wav` |
| **YouTube** | -13 LUFS | `normalize -L -13 -m 98 *.wav` |
| **Apple Music** | -16 LUFS | `normalize -L -16 -m 99 *.wav` |
| **Amazon Music** | -14 LUFS | `normalize -L -14 -m 99 *.wav` |
| **Podcast** | -16 to -19 | `normalize -L -16 -g 95 *.wav` |
| **EBU R128 (TV)** | -23 LUFS | `normalize -L -23 *.wav` |
| **ATSC (US TV)** | -24 LUFS | `normalize -L -24 *.wav` |

## 🎚️ Command Reference

### LUFS Normalization
```
-L <lufs>      Target loudness in LUFS (e.g., -14 for Spotify)
-g <percent>   Gate percentile: ignore loudest X% of blocks (50-100)
-m <percent>   Peak limiting: don't exceed X% of maximum
```

### Peak Normalization
```
-m <percent>   Normalize to X% of maximum (default 100)
-s <percent>   SmartPeak: use percentile for peak detection (50-100)
-a <level>     Amplify by exact dB amount
-l <ratio>     Linear gain multiplication
```

### Other Options
```
-w <folder>    Watch folder mode: process files automatically
-O <folder>    Output folder for watch mode (required with -w)
-b <size>      I/O buffer size in KB (16-16384, default 64)
-o <file>      Output to file instead of overwriting
-p             Prompt before normalization
-q             Quiet mode (no output)
-d             Don't abort batch on skip
-x <level>     Skip if gain is less than X dB
-h             Show help
```

## 🏗️ Building from Source

### Prerequisites
- Windows 10/11
- Visual Studio 2019/2022 Build Tools or full Visual Studio

### Build Commands

**Using batch file:**
```batch
build-msvc.bat
```

**Manual:**
```batch
cl /W3 /O2 /Fenormalize.exe normalize.c PCMWAV.C kernel32.lib
```

**Alternative (build.bat):**
```batch
build.bat
```

## 🤝 Contributing

Contributions welcome! Areas for improvement:
- True peak limiting (4x oversampling)
- Multi-channel support (5.1 surround)
- Short-term/momentary LUFS
- Loudness range (LRA) calculation
- Cross-platform support (Linux, macOS)

## 📜 License

GNU General Public License v2 (GPL-2.0)

**Original normalize v0.253**  
Copyright © 2000-2004 Manuel Kasper <mk@neon1.net>  
SmartPeak code by Lapo Luchini <lapo@lapo.it>

**LUFS Normalization & Watch Mode (2025)**  
Cam St Clair with assistance from Claude (Anthropic)
- ITU-R BS.1770-4 compliant LUFS implementation
- Automated folder monitoring (watch mode)
- Enhanced file handling and processing

See [COPYING.txt](COPYING.txt) for full license text.

## 🙏 Acknowledgments

- **Manuel Kasper** - Original normalize tool (2000-2004)
- **Lapo Luchini** - SmartPeak algorithm
- **Cam St Clair** - LUFS normalization and watch mode (2025)
- **Claude (Anthropic)** - AI assistance for implementation, I'm always learning
- **ITU-R BS.1770-4** - LUFS specification

---

**Made with ❤️ for audio professionals, podcasters, and music enthusiasts**
