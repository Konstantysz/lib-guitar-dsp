#include "HybridPitchDetector.h"
#include <cmath>

namespace GuitarDSP
{

    HybridPitchDetector::HybridPitchDetector(const HybridPitchDetectorConfig &config)
        : config(config), yinDetector(nullptr), mpmDetector(nullptr), yinUsedCount(0), mpmUsedCount(0)
    {
        // Fine-tune YIN for guitar frequencies
        auto yinCfg = config.yinConfig;
        yinCfg.threshold = 0.10f;      // Lower threshold for better low-E detection
        yinCfg.minFrequency = 80.0f;   // Low E2 is 82.4 Hz
        yinCfg.maxFrequency = 1200.0f; // Up to D6

        yinDetector = std::make_unique<YinPitchDetector>(yinCfg);
        mpmDetector = std::make_unique<MpmPitchDetector>(config.mpmConfig);
    }

    HybridPitchDetector::~HybridPitchDetector() = default;

    void HybridPitchDetector::Reset()
    {
        yinDetector->Reset();
        mpmDetector->Reset();
        yinUsedCount = 0;
        mpmUsedCount = 0;
    }

    std::optional<PitchResult> HybridPitchDetector::Detect(std::span<const float> buffer, float sampleRate)
    {
        if (buffer.empty() || sampleRate <= 0.0f)
        {
            return std::nullopt;
        }

        // Try YIN first (faster)
        auto yinResult = yinDetector->Detect(buffer, sampleRate);

        std::optional<PitchResult> finalResult = std::nullopt;

        if (yinResult.has_value() && yinResult->confidence >= config.yinConfidenceThreshold)
        {
            // YIN is confident, use it
            finalResult = yinResult;
            ++yinUsedCount;
        }
        else
        {
            // YIN not confident, try MPM
            auto mpmResult = mpmDetector->Detect(buffer, sampleRate);

            if (mpmResult.has_value())
            {
                // If both detected but YIN had low confidence, prefer MPM
                finalResult = mpmResult;
                ++mpmUsedCount;
            }
            else if (yinResult.has_value())
            {
                // MPM failed but YIN detected something, use YIN anyway
                finalResult = yinResult;
                ++yinUsedCount;
            }
        }

        // Apply harmonic rejection if enabled
        if (finalResult.has_value() && config.enableHarmonicRejection)
        {
            float correctedFreq = ApplyHarmonicRejection(finalResult->frequency);
            if (std::abs(correctedFreq - finalResult->frequency) > 0.1f)
            {
                // Frequency was corrected, update result
                finalResult->frequency = correctedFreq;
            }
        }

        return finalResult;
    }

    float HybridPitchDetector::ApplyHarmonicRejection(float frequency)
    {
        // Check if this might be a harmonic (2x, 3x, 4x) of the fundamental
        // For guitar strings, the fundamental is usually below 400 Hz

        // Check 2nd harmonic (octave)
        float fundamental2x = frequency / 2.0f;
        if (fundamental2x >= 80.0f && fundamental2x <= 400.0f)
        {
            // Check if it's a valid guitar frequency
            if (IsHarmonic(frequency, fundamental2x, 2))
            {
                return fundamental2x;
            }
        }

        // Check 3rd harmonic
        float fundamental3x = frequency / 3.0f;
        if (fundamental3x >= 80.0f && fundamental3x <= 400.0f)
        {
            if (IsHarmonic(frequency, fundamental3x, 3))
            {
                return fundamental3x;
            }
        }

        // Check 4th harmonic
        float fundamental4x = frequency / 4.0f;
        if (fundamental4x >= 80.0f && fundamental4x <= 400.0f)
        {
            if (IsHarmonic(frequency, fundamental4x, 4))
            {
                return fundamental4x;
            }
        }

        // No harmonic detected return original
        return frequency;
    }

    bool HybridPitchDetector::IsHarmonic(float freq1, float freq2, int harmonicNumber)
    {
        // Check if freq1 is approximately N times freq2
        float expectedHarmonic = freq2 * static_cast<float>(harmonicNumber);
        float diff = std::abs(freq1 - expectedHarmonic);
        float tolerance = expectedHarmonic * config.harmonicTolerance;

        return diff <= tolerance;
    }

} // namespace GuitarDSP
