# Contributing to Normalize

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## 🚀 Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/normalize-src.git`
3. Create a branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Test thoroughly
6. Commit with clear messages
7. Push and create a Pull Request

## 🏗️ Development Setup

### Prerequisites
- Windows 10/11
- Visual Studio 2022 Build Tools
- Git for Windows

### Building
```batch
build-msvc.bat
```

### Testing
Test your changes with various WAV files:
- Different bit depths (8-bit, 16-bit)
- Different sample rates (44.1kHz, 48kHz)
- Various content types (music, speech, silence)

## 📝 Code Style

### C Code Conventions
- Follow existing code style (K&R style with tabs)
- Keep functions focused and reasonably sized
- Comment complex algorithms
- Use descriptive variable names
- Maintain the dual 8-bit/16-bit architecture pattern

### Example:
```c
// Good
int calculate_lufs16(void) {
    double measured_lufs;
    // Clear implementation
    return measured_lufs;
}

// Avoid
int f(void){int x;return x;}  // Too compressed
```

## 🧪 Testing Guidelines

### Before Submitting
- [ ] Code compiles without errors
- [ ] No new compiler warnings
- [ ] Tested with 8-bit and 16-bit WAV files
- [ ] Tested with various sample rates
- [ ] Memory leaks checked (use Visual Studio analyzer)
- [ ] Edge cases handled (silence, clipping, etc.)

### Test Files
Create test cases for:
1. Normal audio
2. Silent files
3. Clipped audio
4. Very quiet audio
5. Dynamic content (classical music)
6. Compressed content (pop/rock)

## 🎯 Areas for Contribution

### High Priority
- [ ] True peak limiting (4x oversampling)
- [ ] Cross-platform support (Linux, macOS)
- [ ] Unit tests
- [ ] CI/CD pipeline

### Medium Priority
- [ ] Multi-channel support (5.1 surround)
- [ ] Short-term/momentary LUFS
- [ ] Loudness range (LRA) calculation
- [ ] Progress bar improvements
- [ ] JSON output format

### Nice to Have
- [ ] GUI version
- [ ] Undo functionality
- [ ] Batch processing optimizations
- [ ] Plugin system
- [ ] More audio format support (FLAC, MP3, etc.)

## 📋 Pull Request Process

1. **Update Documentation**
   - Update README.md if adding features
   - Add to CHANGELOG.md
   - Update relevant docs in `/docs`

2. **Commit Messages**
   - Use clear, descriptive messages
   - Reference issues if applicable
   - Example: `Add true peak limiting support (#42)`

3. **PR Description**
   - Describe what you changed and why
   - List any breaking changes
   - Include test results
   - Screenshots/audio samples if applicable

4. **Code Review**
   - Be open to feedback
   - Respond to review comments
   - Make requested changes promptly

## 🐛 Bug Reports

### Before Reporting
- Check existing issues
- Test with latest version
- Gather debug information

### Bug Report Template
```markdown
**Description**
Clear description of the bug

**To Reproduce**
1. Run command: `normalize -L -14 file.wav`
2. Observe behavior

**Expected Behavior**
What you expected to happen

**Actual Behavior**
What actually happened

**Environment**
- OS: Windows 11
- Version: [from normalize -h]
- WAV file: 16-bit, 44.1kHz, stereo

**Additional Context**
Any other relevant information
```

## 💡 Feature Requests

We welcome feature suggestions! Please:
- Check if it's already requested
- Explain the use case
- Describe expected behavior
- Consider implementation complexity

## 🔍 Code Review Criteria

Pull requests will be evaluated on:
- **Correctness**: Does it work as intended?
- **Performance**: No significant slowdowns
- **Compatibility**: Windows API conventions maintained
- **Code Quality**: Readable, maintainable
- **Testing**: Adequately tested
- **Documentation**: Changes documented

## 📜 License

By contributing, you agree that your contributions will be licensed under the GNU GPL v2 License.

## ❓ Questions?

- Open an issue with the `question` label
- Check existing documentation in `/docs`
- Review closed issues for similar questions

## 🙏 Thank You!

Every contribution helps make this tool better for everyone. We appreciate your time and effort!

---

**Maintainer Notes:**
- Be respectful and constructive
- Help new contributors
- Keep discussions on-topic
- Maintain project goals and scope
