#pragma once

#include "MpmPitchDetector.h"
#include "YinPitchDetector.h"
#include <memory>

namespace GuitarDSP
{
    /**
     * @brief Configuration for hybrid detector
     */
    struct HybridPitchDetectorConfig
    {
        float yinConfidenceThreshold = 0.8f; ///< Use MPM if YIN confidence below this
        bool enableHarmonicRejection = true; ///< Enable harmonic rejection
        float harmonicTolerance = 0.05f;     ///< Tolerance for harmonic detection (5%)
        YinPitchDetectorConfig yinConfig;    ///< YIN configuration
        MpmPitchDetectorConfig mpmConfig;    ///< MPM configuration
    };

    /**
     * @brief Hybrid pitch detector combining YIN and MPM with harmonic rejection
     *
     * Strategy:
     * - Primary: YIN (faster, accurate for stable tones)
     * - Fallback: MPM when YIN confidence < threshold
     * - Harmonic rejection: Detect and correct octave errors (2x, 3x, 4x harmonics)
     *
     * This provides robust detection for guitar tuning, handling both
     * stable tones and strings with vibrato.
     */
    class HybridPitchDetector : public PitchDetector
    {
    public:
        /**
         * @brief Constructs hybrid pitch detector
         * @param config Hybrid configuration
         */
        explicit HybridPitchDetector(const HybridPitchDetectorConfig &config = HybridPitchDetectorConfig{});

        ~HybridPitchDetector() override;

        [[nodiscard]] std::optional<PitchResult> Detect(std::span<const float> buffer, float sampleRate) override;

        void Reset() override;

    private:
        /**
         * @brief Detects if frequency is a harmonic of a fundamental
         * @return Fundamental frequency if harmonic detected, otherwise the original frequency
         */
        float ApplyHarmonicRejection(float frequency);

        /**
         * @brief Checks if freq1 is approximately N times freq2
         */
        bool IsHarmonic(float freq1, float freq2, int harmonicNumber);

        HybridPitchDetectorConfig config;              ///< Detector configuration
        std::unique_ptr<YinPitchDetector> yinDetector; ///< YIN detector instance
        std::unique_ptr<MpmPitchDetector> mpmDetector; ///< MPM detector instance

        mutable size_t yinUsedCount; ///< Counter for YIN algorithm usage
        mutable size_t mpmUsedCount; ///< Counter for MPM algorithm usage
    };

} // namespace GuitarDSP
