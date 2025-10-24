# Authors and Contributors

## Original normalize v0.253 (2000-2004)

**Manuel Kasper** <mk@neon1.net>
- Original author and primary developer
- Core normalization engine
- Peak detection algorithms
- WAV file I/O library (PCMWAV.C/PCMWAV.H)
- Batch processing support

**Lapo Luchini** <lapo@lapo.it>
- SmartPeak algorithm
- Statistical peak analysis
- Percentile-based normalization

## 2025 Enhancements

**Cam St Clair**
- Project lead for modernization
- LUFS normalization implementation
- Watch mode for automated folder monitoring
- Documentation and testing

**Claude (Anthropic)**
- AI assistant for code implementation
- Algorithm design and optimization
- Documentation and technical writing
- Code review and debugging assistance

## Technical Contributions

### LUFS Normalization (2025)
- Implementation of ITU-R BS.1770-4 specification
- K-weighting filters (biquad IIR)
- Gated loudness measurement
- Percentile-based block filtering
- Integration with existing codebase

**Contributors**: Cam St Clair with Claude (Anthropic)

### Watch Mode (2025)
- Automated folder monitoring with ReadDirectoryChangesW
- File readiness detection and handling
- Automatic file moving with conflict resolution
- Active folder scanning for reliability
- Error recovery and continuous operation

**Contributors**: Cam St Clair with Claude (Anthropic)

## License

All contributions are released under the GNU General Public License v2 (GPL-2.0), 
consistent with the original normalize license.

## Acknowledgments

### Technical References
- **ITU-R BS.1770-4**: Algorithms to measure audio programme loudness and true-peak audio level
- **EBU R128**: Loudness normalisation and permitted maximum level of audio signals
- **Microsoft Developer Network**: Windows API documentation

### Community
- Original normalize users who kept the project alive
- Audio engineering community for loudness standards advocacy
- Open source contributors worldwide

## Contact

### Current Maintainer (2025+)
**Cam St Clair**
- GitHub: [Create issue for questions/bugs]

### Original Author (Historical)
**Manuel Kasper** <mk@neon1.net>
- Website: http://neon1.net/
- Note: Original project circa 2000-2004, may not be actively maintained

## Contributing

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

When contributing, please:
1. Maintain attribution for original authors
2. Add your name to this file for substantial contributions
3. Follow existing code style and conventions
4. Include appropriate copyright notices in new files
5. Respect the GPL-2.0 license terms

---

**This software stands on the shoulders of giants. We honor all contributors, 
past and present, who have made this tool possible.**
