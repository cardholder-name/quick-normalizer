# Setting Up for GitHub

This guide will help you get your project ready for GitHub.

## Step 1: Install Git

### Option A: Using winget (Easiest)
```powershell
winget install Git.Git
```

### Option B: Manual Download
Download from: https://git-scm.com/download/win

After installation, **restart PowerShell**.

## Step 2: Configure Git

```powershell
# Set your name and email
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"

# Optional: Set default branch name to main
git config --global init.defaultBranch main
```

## Step 3: Initialize Repository

```powershell
cd C:\Users\camer\Documents\normalize-src

# Initialize git repository
git init

# Add all files (respects .gitignore)
git add .

# Create initial commit
git commit -m "Initial commit: LUFS-enabled audio normalizer"
```

## Step 4: Create GitHub Repository

1. Go to https://github.com/new
2. Repository name: `normalize-audio` (or your preferred name)
3. Description: `Professional audio normalization tool with LUFS support for Windows`
4. Choose: **Public** (or Private)
5. **Do NOT** initialize with README (we already have one)
6. Click **Create repository**

## Step 5: Push to GitHub

After creating the repository, GitHub will show you commands. Use these:

```powershell
# Add the remote repository
git remote add origin https://github.com/YOUR_USERNAME/normalize-audio.git

# Push your code
git branch -M main
git push -u origin main
```

## Step 6: Add GitHub Topics (Optional)

On your repository page:
1. Click the ⚙️ icon next to "About"
2. Add topics: `audio`, `normalization`, `lufs`, `windows`, `c`, `dsp`, `audio-processing`

## Quick Reference

### Common Git Commands

```powershell
# Check status
git status

# Stage changes
git add .

# Commit changes
git commit -m "Description of changes"

# Push to GitHub
git push

# Pull latest changes
git pull

# Create a new branch
git checkout -b feature/new-feature

# Switch branches
git checkout main

# View commit history
git log --oneline
```

## Recommended Repository Settings

### Branch Protection (for main branch)
1. Go to Settings → Branches
2. Add rule for `main`
3. Enable: "Require pull request reviews"

### GitHub Actions (Optional)
Create `.github/workflows/build.yml` for automated builds:

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Add MSBuild to PATH
        uses: microsoft/setup-msbuild@v1
      - name: Build
        run: build-msvc.bat
```

## Creating Releases

When you're ready to release:

```powershell
# Tag a version
git tag -a v1.0.0 -m "Release v1.0.0 - LUFS support"
git push origin v1.0.0
```

Then on GitHub:
1. Go to "Releases"
2. Click "Draft a new release"
3. Select your tag
4. Add release notes
5. Upload `normalize.exe` as a binary

## .gitignore Already Configured

Your `.gitignore` file is already set up to ignore:
- Compiled binaries (*.exe, *.obj)
- Build artifacts
- Temporary files
- IDE files

## Repository Structure (Clean)

```
normalize-src/
├── .github/
│   └── copilot-instructions.md
├── docs/
│   ├── COMPILATION_OPTIONS.md
│   ├── LANGUAGE_CHOICE.md
│   ├── LUFS_IMPLEMENTATION.md
│   ├── ORIGINAL_README.md
│   ├── QUICKSTART.md
│   └── QUICK_INSTALL.md
├── .gitignore
├── build-msvc.bat
├── build.bat
├── CHANGELOG.md
├── CONTRIBUTING.md
├── COPYING.txt
├── LICENSE
├── normalize.c
├── PCMWAV.C
├── PCMWAV.H
└── README.md
```

## Next Steps After Pushing

1. **Add Shields/Badges** to README (already included)
2. **Enable GitHub Pages** (optional, for documentation)
3. **Set up Issues** for bug tracking
4. **Create Project Board** for task management
5. **Add Wiki** for extended documentation

## Collaborating

To accept contributions:
1. Contributors fork your repository
2. They create a branch and make changes
3. They submit a Pull Request
4. You review and merge

See `CONTRIBUTING.md` for detailed guidelines.

## Troubleshooting

### Authentication Issues
GitHub requires a Personal Access Token (PAT) instead of password:
1. Go to Settings → Developer settings → Personal access tokens
2. Generate new token with `repo` scope
3. Use token instead of password when pushing

Or use **GitHub CLI**:
```powershell
winget install GitHub.cli
gh auth login
```

### Large Files
If you accidentally commit large test WAV files:
```powershell
git rm --cached large_file.wav
git commit -m "Remove large file"
```

## Resources

- [GitHub Docs](https://docs.github.com)
- [Git Basics](https://git-scm.com/book/en/v2/Getting-Started-Git-Basics)
- [GitHub Flow](https://guides.github.com/introduction/flow/)

---

**Ready to share your work with the world!** 🚀
