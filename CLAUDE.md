# lib-guitar-dsp

This file provides guidance when working with the **lib-guitar-dsp** library.

## Overview

**lib-guitar-dsp** is a DSP (Digital Signal Processing) library specialized for guitar audio applications. It provides pitch detection algorithms and note conversion utilities optimized for accuracy and performance.

**Key Features:**

- YIN pitch detection algorithm (±0.1 cent accuracy target)
- Note/frequency conversion utilities
- PFFFT integration (fast, GPL-free FFT)
- Real-time safe processing
- Optimized for guitar frequency range (80-1200 Hz)

## Repository Structure

```
lib-guitar-dsp/
├── include/
│   ├── PitchDetector.h        # Abstract pitch detector interface
│   ├── YinPitchDetector.h     # YIN algorithm implementation
│   └── NoteConverter.h        # Frequency ↔ note conversion
├── src/
│   ├── YinPitchDetector.cpp   # YIN implementation
│   └── NoteConverter.cpp      # Note utilities
├── external/
│   └── pffft/                 # PFFFT library (BSD license)
├── CMakeLists.txt
└── CLAUDE.md
```

## Coding Standards

**Naming Conventions:**

- **PascalCase** for classes, structs, and functions
  - Examples: `YinPitchDetector`, `Detect()`, `FrequencyToNote()`
- **camelCase** for ALL variables (including class member variables)
  - **NO trailing underscores** on member variables
  - Examples: `config`, `yinBuffer`, `threshold`, `minFrequency`

**Code Formatting:**

- **Line limit:** 120 characters
- **Indentation:** 4 spaces (no tabs)
- **Brace style:** Allman (opening braces on new lines)
- **Pointer alignment:** Right-aligned (`Type *ptr`)

**Formatting Tools:**

- `.clang-format` - Enforces code style
- `.clang-tidy` - Static analysis
- `.pre-commit-config.yaml` - Automated checks

## Architecture

### Pitch Detection Interface

The library uses an abstract interface for pitch detection algorithms:

```cpp
namespace GuitarDSP
{
    struct PitchResult
    {
        float frequency;   // Detected frequency in Hz
        float confidence;  // Detection confidence [0.0, 1.0]
    };

    class PitchDetector
    {
    public:
        virtual ~PitchDetector() = default;

        virtual std::optional<PitchResult> Detect(
            const float *buffer,
            size_t bufferSize,
            float sampleRate
        ) = 0;

        virtual void Reset() = 0;
    };
}
```

This allows swapping algorithms (YIN, MPM, etc.) without changing client code.

### YIN Algorithm

The YIN algorithm is the primary pitch detector, offering ±0.1 cent accuracy:

**Algorithm Steps:**

1. **Difference function:** Calculate autocorrelation-like function
2. **Cumulative mean normalized difference:** Normalize the difference function
3. **Absolute threshold:** Find first minimum below threshold
4. **Parabolic interpolation:** Achieve sub-sample accuracy

**Configuration:**

```cpp
GuitarDSP::YinPitchDetector::Config config{
    .threshold = 0.15f,        // Detection sensitivity [0.0, 1.0]
    .minFrequency = 80.0f,     // E2 (low E string)
    .maxFrequency = 1200.0f    // D6 (high frets)
};
```

### Note Conversion

Utilities for converting between frequencies and musical notes:

```cpp
namespace GuitarDSP
{
    struct NoteInfo
    {
        std::string name;   // "E", "F#", "C", etc.
        int octave;         // 2, 3, 4, etc.
        float cents;        // Deviation from target pitch [-50, +50]
    };

    class NoteConverter
    {
    public:
        static NoteInfo FrequencyToNote(float frequency, float a4Reference = 440.0f);
        static float NoteToFrequency(const std::string &note, int octave, float a4Reference = 440.0f);
    };
}
```

## Usage Example

### Basic Pitch Detection

```cpp
#include <YinPitchDetector.h>

// Configure YIN detector
GuitarDSP::YinPitchDetector::Config config{
    .threshold = 0.15f,
    .minFrequency = 80.0f,
    .maxFrequency = 1200.0f
};

GuitarDSP::YinPitchDetector detector(config);

// Process audio buffer
const float *audioBuffer = ...;  // From audio device
size_t bufferSize = 2048;
float sampleRate = 48000.0f;

auto result = detector.Detect(audioBuffer, bufferSize, sampleRate);

if (result.has_value())
{
    std::cout << "Frequency: " << result->frequency << " Hz" << std::endl;
    std::cout << "Confidence: " << (result->confidence * 100.0f) << "%" << std::endl;
}
else
{
    std::cout << "No pitch detected" << std::endl;
}
```

### Note Conversion

```cpp
#include <NoteConverter.h>

// Convert frequency to note
float frequency = 329.63f;  // E4
auto note = GuitarDSP::NoteConverter::FrequencyToNote(frequency);

std::cout << "Note: " << note.name << note.octave << std::endl;
std::cout << "Cents: " << note.cents << std::endl;

// Convert note to frequency
float e4 = GuitarDSP::NoteConverter::NoteToFrequency("E", 4);
std::cout << "E4 frequency: " << e4 << " Hz" << std::endl;
```

## Real-Time Considerations

### Buffer Size Selection

**Accuracy vs Latency Trade-off:**

- **Smaller buffers (512-1024):** Lower latency, less accurate
- **Larger buffers (2048-4096):** Higher accuracy, more latency
- **Recommended:** 2048 samples @ 48kHz = ~43ms latency

### Pre-allocation

YIN detector pre-allocates internal buffers to avoid allocations during `Detect()`:

```cpp
// Internal buffer resized only when needed
if (yinBuffer.size() < halfBufferSize)
{
    yinBuffer.resize(halfBufferSize, 0.0f);  // Only grows, never shrinks
}
```

**Best practice:** Process consistent buffer sizes to avoid reallocation.

### Thread Safety

The `Detect()` method is **not thread-safe** if called on the same instance. Use one detector per thread or add synchronization:

```cpp
// Thread-safe pattern
class AudioProcessor
{
    GuitarDSP::YinPitchDetector detector;
    std::mutex detectorMutex;

public:
    std::optional<PitchResult> DetectPitch(const float *buffer, size_t size, float rate)
    {
        std::lock_guard<std::mutex> lock(detectorMutex);
        return detector.Detect(buffer, size, rate);
    }
};
```

Or use separate detectors per thread (no synchronization needed).

## Accuracy Optimization

### Achieving ±0.1 Cent Precision

**Requirements:**

1. **High sample rate:** 48kHz or higher
2. **Large buffer size:** 2048+ samples
3. **Parabolic interpolation:** Enabled by default in YIN
4. **Stable input:** Clean signal, minimal noise

**Accuracy Formula:**

```
Frequency resolution = sampleRate / bufferSize
At 48kHz, 2048 samples:
  Resolution = 48000 / 2048 ≈ 23.4 Hz

With parabolic interpolation:
  Sub-sample accuracy ≈ 0.01 Hz

At A4 (440 Hz):
  1 cent = 0.254 Hz
  0.1 cent = 0.0254 Hz

Achievable accuracy: ~0.05-0.1 cents
```

### Confidence Thresholding

Filter unreliable detections by confidence:

```cpp
auto result = detector.Detect(buffer, size, rate);

if (result.has_value() && result->confidence > 0.7f)
{
    // High confidence detection
    ProcessPitch(result->frequency);
}
else
{
    // Ignore low-confidence or no detection
}
```

**Typical confidence values:**

- **>0.9:** Excellent (clean single note)
- **0.7-0.9:** Good (usable for tuning)
- **0.5-0.7:** Fair (noisy or multiple frequencies)
- **<0.5:** Poor (ignore or use fallback)

## Common Development Tasks

### Adding a New Pitch Detection Algorithm

1. Extend the `PitchDetector` interface:

```cpp
// include/MyPitchDetector.h
namespace GuitarDSP
{
    class MyPitchDetector : public PitchDetector
    {
    public:
        struct Config { /* ... */ };

        explicit MyPitchDetector(const Config &config);

        std::optional<PitchResult> Detect(
            const float *buffer,
            size_t bufferSize,
            float sampleRate
        ) override;

        void Reset() override;

    private:
        Config config;
        // ... algorithm-specific state ...
    };
}
```

2. Implement in `src/MyPitchDetector.cpp`
3. Add tests
4. Update CMakeLists.txt

### Testing Pitch Detection

```cpp
#include <gtest/gtest.h>
#include <YinPitchDetector.h>
#include <cmath>

TEST(YinPitchDetectorTest, DetectsA440)
{
    constexpr int sampleRate = 48000;
    constexpr int bufferSize = 2048;
    float buffer[bufferSize];

    // Generate pure 440 Hz sine wave
    for (int i = 0; i < bufferSize; ++i)
    {
        buffer[i] = std::sin(2.0f * M_PI * 440.0f * i / sampleRate);
    }

    GuitarDSP::YinPitchDetector detector;
    auto result = detector.Detect(buffer, bufferSize, sampleRate);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->frequency, 440.0f, 0.5f);  // Within 0.5 Hz
    EXPECT_GT(result->confidence, 0.9f);  // High confidence
}
```

## Troubleshooting

**Problem:** Inaccurate pitch detection

- Increase buffer size (2048 → 4096)
- Check input signal quality (use clean DI signal)
- Verify sample rate is accurate
- Adjust threshold (try 0.10-0.20 range)

**Problem:** No pitch detected

- Check frequency range covers input (default: 80-1200 Hz)
- Increase threshold (try 0.20-0.25)
- Verify buffer contains non-zero audio data
- Check signal amplitude (may be too quiet)

**Problem:** Multiple frequencies detected (jumpy output)

- Increase confidence threshold (>0.7)
- Use low-pass filter on input
- Implement smoothing on output
- Check for harmonic interference

## Performance Optimization

### YIN Algorithm Complexity

- **Time complexity:** O(N²) where N = bufferSize
- **Typical processing time:** ~0.5-2ms for 2048 samples (modern CPU)
- **CPU usage:** <1% on modern hardware

### Optimization Tips

1. **Use consistent buffer sizes** to avoid reallocation
2. **Pre-allocate detectors** at initialization, not in audio callback
3. **Reuse detector instances** instead of creating new ones
4. **Profile before optimizing** - algorithm is already fast

## Build System

This library is designed to be used as a git submodule:

```cmake
# Parent project CMakeLists.txt
add_subdirectory(external/lib-guitar-dsp)

target_link_libraries(your-app PRIVATE guitar-dsp)
```

The library automatically links PFFFT and handles dependencies.

## License

BSD License - See LICENSE file for details.

Compatible with commercial projects. PFFFT is BSD-licensed, YIN algorithm is public domain.
