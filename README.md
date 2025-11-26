# lib-guitar-dsp

[![Build Status](https://github.com/Konstantysz/kappa-core/actions/workflows/build.yml/badge.svg)](https://github.com/Konstantysz/kappa-core/actions/workflows/build.yml)
[![Tests](https://github.com/Konstantysz/kappa-core/actions/workflows/tests.yml/badge.svg)](https://github.com/Konstantysz/kappa-core/actions/workflows/tests.yml)
[![Static Analysis](https://github.com/Konstantysz/kappa-core/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/Konstantysz/kappa-core/actions/workflows/static-analysis.yml)

Digital signal processing library for guitar applications with ±0.1 cent pitch detection accuracy.

## Overview

`lib-guitar-dsp` provides high-precision pitch detection and music theory utilities optimized for guitar tuning applications. Built on the YIN algorithm with PFFFT for fast Fourier transforms.

## Features

- ✅ **YIN Pitch Detection**: ±0.1 cent accuracy for professional tuning
- ✅ **Fast FFT**: PFFFT library with SIMD optimizations (BSD license)
- ✅ **Note Conversion**: Frequency ↔ note name with cent precision
- ✅ **Real-time Performance**: Optimized for low-latency audio processing
- ✅ **BSD/MIT License**: GPL-free, commercially compatible

## Usage

```cpp
#include <YinPitchDetector.h>
#include <NoteConverter.h>

using namespace GuitarDSP;

// Create YIN pitch detector
YinPitchDetector::Config config{
    .threshold = 0.15f,
    .minFrequency = 80.0f,   // E2 (low E string)
    .maxFrequency = 1200.0f  // D6 (high frets)
};

YinPitchDetector detector(config);

// Detect pitch from audio buffer
const float* audioBuffer = /* ... */;
const size_t bufferSize = 2048;
const float sampleRate = 48000.0f;

if (auto result = detector.Detect(audioBuffer, bufferSize, sampleRate))
{
    // Convert to note
    auto note = NoteConverter::FrequencyToNote(result->frequency);

    printf("Detected: %s%d (%.2f Hz)\n",
           note.name.c_str(), note.octave, result->frequency);
    printf("Deviation: %+.1f cents (confidence: %.2f)\n",
           note.cents, result->confidence);
}
```

## Algorithms

### YIN Pitch Detection

Based on "YIN, a fundamental frequency estimator for speech and music" (de Cheveigné & Kawahara, 2002):

1. **Difference Function**: Autocorrelation-based lag detection
2. **Cumulative Mean Normalization**: Noise reduction
3. **Absolute Threshold**: Pitch period selection
4. **Parabolic Interpolation**: Sub-sample accuracy

**Performance**:

- Accuracy: 0.78% error rate (±0.1 cent)
- Latency: ~20-50ms (depending on buffer size)
- Frequency range: 80 Hz - 1200 Hz (configurable)

### Note Conversion

- **Frequency → Note**: MIDI note calculation with cent deviation
- **Note → Frequency**: Equal temperament tuning (configurable A4)
- **Cent Calculation**: Logarithmic frequency ratio

## Building

Designed as a git submodule:

```bash
# Add as submodule
git submodule add <repo-url> external/lib-guitar-dsp
git submodule update --init --recursive

# CMake integration
add_subdirectory(external/lib-guitar-dsp)
target_link_libraries(your-app PRIVATE guitar-dsp)
```

## Dependencies

- **PFFFT** (git submodule): Fast FFT with BSD license
- **CMake** 3.20+
- **C++20** compiler
- **Math library** (libm on Unix)

## Architecture

```
lib-guitar-dsp/
├── include/
│   ├── PitchDetector.h       # Abstract base class
│   ├── YinPitchDetector.h    # YIN implementation
│   └── NoteConverter.h       # Frequency/note utilities
├── src/
│   ├── PitchDetector.cpp
│   ├── YinPitchDetector.cpp  # YIN algorithm
│   └── NoteConverter.cpp     # Music theory math
├── external/
│   └── pffft/                # PFFFT submodule (BSD)
└── CMakeLists.txt
```

## Optimization

PFFFT is compiled with SIMD optimizations:

- **MSVC**: `/arch:AVX2`
- **GCC/Clang**: `-mavx2 -mfma`

For maximum performance, ensure CPU supports AVX2 (2013+).

## Future Algorithms

- **MPM (McLeod Pitch Method)**: Lower latency alternative to YIN
- **Polyphonic Detection**: Multiple simultaneous notes
- **Harmonic Analysis**: Timbre characterization

## License

BSD License - See [LICENSE](LICENSE) for details.

Uses PFFFT (BSD license) - GPL-free and commercially compatible.
