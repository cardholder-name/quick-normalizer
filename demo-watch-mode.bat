@echo off
REM Watch Mode Demo Script for Normalize
REM This demonstrates automated folder monitoring and processing

echo ========================================
echo  NORMALIZE WATCH MODE DEMO
echo ========================================
echo.

REM Create folders if they don't exist
if not exist "test-watch-input" mkdir test-watch-input
if not exist "test-watch-output" mkdir test-watch-output

echo Folders prepared:
echo   Input:  test-watch-input\
echo   Output: test-watch-output\
echo.

echo Starting normalize in watch mode...
echo Target: -14 LUFS (Spotify standard) with 99%% peak limiting
echo.
echo Press Ctrl+C to stop watching
echo.
echo ========================================
echo  DROP WAV FILES INTO test-watch-input\
echo ========================================
echo.

REM Start watching with LUFS normalization
normalize.exe -L -14 -m 99 -w test-watch-input -O test-watch-output

echo.
echo Watch mode stopped.
pause
