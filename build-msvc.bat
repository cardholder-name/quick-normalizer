@echo off
REM Build script for normalize.exe using MSVC

REM Setup Visual Studio environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

echo Building normalize.exe with MSVC...
echo.

cl /W3 /O2 /Fenormalize.exe normalize.c PCMWAV.C kernel32.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo Build successful! normalize.exe created.
    echo ===================================
    echo.
    echo Try these commands:
    echo   normalize -h
    echo   normalize -L -14 your_audio_file.wav
    echo.
) else (
    echo.
    echo Build failed!
)
