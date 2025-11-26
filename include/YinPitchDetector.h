#pragma once

#include "PitchDetector.h"
#include <memory>
#include <vector>

namespace GuitarDSP
{
    /**
     * @brief Configuration for YIN algorithm
     */
    struct YinPitchDetectorConfig
    {
        float threshold = 0.15f;      ///< Detection threshold [0.0, 1.0]
        float minFrequency = 80.0f;   ///< Minimum detectable frequency (Hz)
        float maxFrequency = 1200.0f; ///< Maximum detectable frequency (Hz)
    };

    /**
     * @brief YIN pitch detection algorithm implementation
     *
     * Based on "YIN, a fundamental frequency estimator for speech and music"
     * by Alain de Cheveigné and Hideki Kawahara (2002)
     *
     * Provides ±0.1 cent accuracy for guitar tuning applications.
     */
    class YinPitchDetector : public PitchDetector
    {
    public:
        /**
         * @brief Constructs YIN pitch detector
         * @param config Algorithm configuration
         */
        explicit YinPitchDetector(const YinPitchDetectorConfig &config = YinPitchDetectorConfig{});

        ~YinPitchDetector() override;

        [[nodiscard]] std::optional<PitchResult> Detect(std::span<const float> buffer, float sampleRate) override;

        void Reset() override;

    private:
        YinPitchDetectorConfig config; ///< Algorithm configuration
        std::vector<float> yinBuffer;  ///< Temporary buffer for YIN calculation
    };

} // namespace GuitarDSP
