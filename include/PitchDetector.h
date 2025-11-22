#pragma once

#include <cstddef>
#include <optional>

namespace GuitarDSP
{
    /**
     * @brief Pitch detection result
     */
    struct PitchResult
    {
        float frequency;   ///< Detected frequency in Hz
        float confidence;  ///< Confidence level [0.0, 1.0]
    };

    /**
     * @brief Abstract base class for pitch detection algorithms
     */
    class PitchDetector
    {
    public:
        virtual ~PitchDetector() = default;

        /**
         * @brief Detects pitch from audio buffer
         * @param buffer Input audio buffer (mono)
         * @param bufferSize Number of samples in buffer
         * @param sampleRate Sample rate in Hz
         * @return Pitch result if detected, nullopt otherwise
         */
        [[nodiscard]] virtual std::optional<PitchResult> Detect(
            const float* buffer,
            size_t bufferSize,
            float sampleRate
        ) = 0;

        /**
         * @brief Resets internal state
         */
        virtual void Reset() = 0;
    };

} // namespace GuitarDSP
