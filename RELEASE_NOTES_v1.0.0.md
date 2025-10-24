# Release v1.0.0 - LUFS Normalization and Watch Mode

**First major release with modern audio normalization features!** 🎉

## 🎯 Major New Features

### LUFS Normalization
- **ITU-R BS.1770-4 compliant** perceptual loudness measurement
- K-weighting filters for accurate human hearing simulation
- Smart gating with percentile-based block filtering
- Peak limiting to prevent clipping
- Ready for streaming platforms:
  - **Spotify**: `-L -14` LUFS
  - **YouTube**: `-L -13` LUFS  
  - **Broadcast (EBU R128)**: `-L -23` LUFS

### Watch Mode
- **Automated folder monitoring** for unattended processing
- Drop files into watch folder, get normalized output automatically
- Perfect for recording studios, podcasts, and batch workflows
- Graceful handling of locked files and conflict resolution
- Continuous operation until stopped with Ctrl+C

### Example Commands

```batch
# LUFS normalization
normalize -L -14 *.wav              # Spotify standard
normalize -L -23 *.wav              # Broadcast standard
normalize -L -16 -g 95 *.wav        # Ignore loudest 5% of blocks
normalize -L -14 -m 98 *.wav        # LUFS with peak limiting

# Watch mode (automated processing)
normalize -L -14 -m 99 -w C:\incoming -O C:\processed
normalize -m 95 -w "Drop Files Here" -O "Done"
normalize -L -16 -g 95 -w input -O output -q
```

## 📥 Installation

**Option 1: Use Pre-Built Binary** (Easiest)
1. Download `normalize.exe` from the Assets below
2. Place in a folder (e.g., `C:\Tools\`)
3. Add to PATH or run directly

**Option 2: Build from Source**
1. Install Visual Studio Build Tools: `winget install Microsoft.VisualStudio.2022.BuildTools`
2. Run `build-msvc.bat`

## 🐛 Bug Fixes
- Fixed issue where only one file was processed when multiple files added simultaneously in watch mode
- Enhanced file readiness detection for network drives and locked files

## 📝 Documentation
- Comprehensive watch mode guide in `docs/WATCH_MODE.md`
- LUFS implementation details in `docs/LUFS_IMPLEMENTATION.md`
- Quick start guide in `docs/QUICKSTART.md`

## 🙏 Credits

**Original normalize v0.253** (2000-2004)
- Created by Manuel Kasper <mk@neon1.net>
- SmartPeak algorithm by Lapo Luchini <lapo@lapo.it>

**2025 Enhancements**
- LUFS normalization and watch mode by Cam St Clair with Claude (Anthropic)

## 📄 License
GPL v2 - See LICENSE file

---

**Full Changelog**: https://github.com/cardholder-name/quick-normalizer/blob/main/CHANGELOG.md
