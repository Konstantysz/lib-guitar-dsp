#pragma once

#include "PitchDetector.h"
#include <memory>
#include <vector>

namespace GuitarDSP
{
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
         * @brief Configuration for YIN algorithm
         */
        struct Config
        {
            float threshold = 0.15f;      ///< Detection threshold [0.0, 1.0]
            float minFrequency = 80.0f;   ///< Minimum detectable frequency (Hz)
            float maxFrequency = 1200.0f; ///< Maximum detectable frequency (Hz)
        };

        /**
         * @brief Constructs YIN pitch detector
         * @param config Algorithm configuration
         */
        explicit YinPitchDetector(const Config &config = Config{});

        ~YinPitchDetector() override;

        [[nodiscard]] std::optional<PitchResult>
            Detect(std::span<const float> buffer, float sampleRate) override;

        void Reset() override;

    private:
        Config config;
        std::vector<float> yinBuffer; ///< Temporary buffer for YIN calculation
    };

} // namespace GuitarDSP
