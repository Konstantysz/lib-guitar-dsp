# Changelog

All notable changes to lib-guitar-dsp will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.1] - 2025-12-07

### Added

- FFTProcessor class

### Fixed

- PFFFT CMake generation

## [0.1.0] - 2025-12-06

### Added

- YIN pitch detection algorithm with sub-sample interpolation
- MPM (McLeod Pitch Method) pitch detection
- PFFFT library integration (BSD-licensed, GPL-free)
- HybridPitchDetector combining YIN and MPM with automatic fallback
- Harmonic rejection for 2x, 3x, 4x overtones
- std::span-based interfaces for type-safe buffer handling

### Fixed

- Harmonic rejection being too aggressive (was incorrectly halving frequencies in 160-800 Hz range)
- Now only applies harmonic rejection above 1200 Hz (normal guitar playing range)

### Technical

- C++20 codebase with concepts and ranges
- Thread-safe pitch detection suitable for real-time audio
- Configurable detection parameters (thresholds, frequency ranges)
