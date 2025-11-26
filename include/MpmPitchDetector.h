#pragma once

#include "PitchDetector.h"
#include <memory>
#include <vector>

namespace GuitarDSP
{
    /**
     * @brief MPM (McLeod Pitch Method) pitch detection algorithm implementation
     *
     * Based on "A Smarter Way to Find Pitch" by Philip McLeod (2005)
     * Uses NSDF (Normalized Square Difference Function) for robust pitch detection,
     * particularly effective for signals with vibrato or changing pitch.
     */
    class MpmPitchDetector : public PitchDetector
    {
    public:
        /**
         * @brief Configuration for MPM algorithm
         */
        struct Config
        {
            float threshold = 0.93f;      ///< NSDF threshold [0.0, 1.0] (higher = more selective)
            float minFrequency = 80.0f;   ///< Minimum detectable frequency (Hz)
            float maxFrequency = 1200.0f; ///< Maximum detectable frequency (Hz)
            float cutoff = 0.97f;         ///< Cutoff for peak detection
            float smallCutoff = 0.5f;     ///< Small cutoff for initial peak search
        };

        /**
         * @brief Constructs MPM pitch detector
         * @param config Algorithm configuration
         */
        explicit MpmPitchDetector(const Config &config = Config{});

        ~MpmPitchDetector() override;

        [[nodiscard]] std::optional<PitchResult> Detect(std::span<const float> buffer, float sampleRate) override;

        void Reset() override;

    private:
        /**
         * @brief Computes Normalized Square Difference Function (NSDF)
         */
        void ComputeNSDF(std::span<const float> buffer);

        /**
         * @brief Finds peaks in NSDF above threshold
         */
        std::vector<int> FindPeaks();

        /**
         * @brief Uses parabolic interpolation to refine peak position
         */
        float ParabolicInterpolation(int tau);

        Config config;                 ///< Algorithm configuration
        std::vector<float> nsdfBuffer; ///< NSDF values
        std::vector<float> acfBuffer;  ///< Autocorrelation buffer
        std::vector<float> rBuffer;    ///< Temp buffer for ACF calculation
    };

} // namespace GuitarDSP
