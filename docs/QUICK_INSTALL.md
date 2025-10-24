# Quick Install Guide - Get Compiling in 5 Minutes!

## You Have winget! ✓

The easiest way to get a compiler is using Windows Package Manager (winget).

## Fastest Option: Install LLVM (Clang)

Run this in PowerShell (as Administrator):

```powershell
winget install LLVM.LLVM
```

Then **restart your PowerShell** and compile with:

```powershell
clang -O2 -o normalize.exe normalize.c PCMWAV.C
```

## Alternative: Install MinGW (GCC)

```powershell
winget install msys2.msys2
```

After installation, open MSYS2 and run:
```bash
pacman -S mingw-w64-x86_64-gcc
```

Then add to PATH: `C:\msys64\mingw64\bin` and compile with:
```powershell
gcc -O2 -o normalize.exe normalize.c PCMWAV.C -lkernel32
```

## Best for Windows: Visual Studio Build Tools

```powershell
winget install Microsoft.VisualStudio.2022.BuildTools
```

After installation, open **"Developer Command Prompt for VS 2022"** from Start menu and run:
```batch
.\build.bat
```

---

## What I Recommend Right Now

**Use LLVM/Clang** - it's the fastest to install and works great:

1. Open PowerShell as Administrator
2. Run: `winget install LLVM.LLVM`
3. Restart PowerShell
4. Run: `clang -O2 -o normalize.exe normalize.c PCMWAV.C`
5. Done! You now have `normalize.exe`

---

## After Compilation

Test your new tool:
```powershell
.\normalize.exe -h
```

Try LUFS normalization:
```powershell
.\normalize.exe -L -14 your_audio_file.wav
```

---

## Don't Have Admin Rights?

You can use an online C compiler for testing:
- https://godbolt.org/ (Compiler Explorer)
- https://replit.com/ (Full IDE in browser)

Or ask your IT department to install one of the compilers above.
