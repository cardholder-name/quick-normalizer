@echo off
REM Build script for normalize.exe with LUFS support
REM Requires Microsoft Visual C++ compiler (cl.exe) in PATH

echo Building normalize.exe...
cl /W3 /O2 /Fenormalize.exe normalize.c PCMWAV.C kernel32.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful! normalize.exe created.
    echo.
    echo Try: normalize -h
) else (
    echo.
    echo Build failed! Make sure Visual C++ compiler is available.
    echo You may need to run this from "Developer Command Prompt for VS"
)

pause
