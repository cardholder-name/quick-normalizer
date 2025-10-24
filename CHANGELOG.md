# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [1.0.1] - 2025-10-24

### Fixed - LUFS Critical Bugs
- **Fixed biquad filter state handling**: Filter states were incorrectly storing input samples instead of intermediate values, causing inaccurate K-weighting
- **Fixed stereo channel processing**: Now properly separates and processes left/right channels independently per ITU-R BS.1770-4 specification
- Each channel now has its own K-weighting filter instance
- Mean squares are calculated per-channel and properly averaged
- LUFS measurements are now accurate for both mono and stereo files (~3dB more accurate for stereo)

### Changed
- Implemented Direct Form II Transposed biquad filter structure for correct state handling
- Updated `docs/LUFS_IMPLEMENTATION.md` with fix details and known limitations

## [1.0.0] - 2025-10-24

### Added - Watch Mode (2025)
- Automated folder monitoring for unattended processing
- Windows ReadDirectoryChangesW API integration
- File readiness detection with 5-second timeout
- Automatic file moving with conflict resolution
- Command line flags: `-w <folder>` (watch), `-O <folder>` (output)
- Comprehensive watch mode documentation in `docs/WATCH_MODE.md`
- Support for continuous background operation
- Graceful handling of locked files and network drives
- Folder scanning after each event batch to catch missed files

### Fixed - Watch Mode (2025)
- Fixed issue where only one file was processed when multiple files added simultaneously
- Added `process_existing_files()` to scan folder after each event batch
- Ensures all files are processed regardless of Windows event batching behavior

### Added - LUFS Support (2025)
- ITU-R BS.1770-4 compliant LUFS normalization
- K-weighting filters (high-shelf at 1682Hz, high-pass at 38Hz)
- 400ms block processing with 75% overlap
- Absolute gate (-70 LUFS) and relative gate (-10 LU)
- Percentile gating option (`-g`) to ignore loudest blocks
- Peak limiting integration with LUFS mode (`-m` with `-L`)
- Support for streaming platform standards (Spotify, YouTube, etc.)
- Comprehensive documentation in `/docs` folder
- Build scripts for MSVC (`build-msvc.bat`)

### Changed
- Updated usage text with LUFS and watch mode examples
- Enhanced error handling for LUFS calculations
- Improved memory management for block processing

### Technical
- Added watch_folder_mode(), is_file_ready(), move_file_to_output() functions
- Windows file change notification implementation (~180 lines)
- Added biquad filter structures for K-weighting
- Implemented circular buffer for efficient block overlap
- Added qsort-based block sorting for percentile gating
- ~700 lines of new code (500 LUFS + 200 watch mode)

## [0.253] - 2004

### Original Features
- Peak-based normalization
- SmartPeak statistical algorithm
- 8-bit and 16-bit PCM WAV support
- Batch processing with wildcards
- Configurable I/O buffer sizes
- Multiple amplification modes (linear, dB)
- Custom output file support

### Credits
- **Original Author**: Manuel Kasper <mk@neon1.net>
- **SmartPeak**: Lapo Luchini <lapo@lapo.it>
- **LUFS & Watch Mode (2025)**: Cam St Clair with Claude (Anthropic)

---

**Legend:**
- `Added` - New features
- `Changed` - Changes to existing functionality
- `Fixed` - Bug fixes
- `Deprecated` - Soon-to-be removed features
- `Removed` - Removed features
- `Fixed` - Bug fixes
- `Security` - Security fixes
