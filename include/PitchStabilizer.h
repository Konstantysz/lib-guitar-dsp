#pragma once

#include "PitchDetector.h"
#include <cstddef>
#include <vector>

namespace GuitarDSP
{
    /**
     * @brief Abstract base class for pitch stabilization strategies
     *
     * Provides smoothing/filtering of pitch detection results to reduce jitter
     * while maintaining responsiveness for intentional pitch changes.
     * All implementations are real-time safe (no allocations after construction).
     */
    class PitchStabilizer
    {
    public:
        virtual ~PitchStabilizer() = default;

        /**
         * @brief Updates stabilizer with new pitch reading
         * @param result Latest pitch detection result
         */
        virtual void Update(const PitchResult &result) = 0;

        /**
         * @brief Gets the stabilized pitch result
         * @return Stabilized pitch result
         */
        [[nodiscard]] virtual PitchResult GetStabilized() const = 0;

        /**
         * @brief Resets internal state
         */
        virtual void Reset() = 0;
    };

    /**
     * @brief Exponential Moving Average pitch stabilizer
     *
     * Simple and efficient smoothing using weighted average:
     * smoothed = alpha * new + (1-alpha) * previous
     *
     * Lower alpha → more smoothing, slower response
     * Higher alpha → less smoothing, faster response
     */
    class ExponentialMovingAverage : public PitchStabilizer
    {
    public:
        /**
         * @brief Configuration for EMA stabilizer
         */
        struct Config
        {
            float alpha = 0.3f; ///< Smoothing factor [0.0, 1.0]
        };

        /**
         * @brief Constructs EMA stabilizer
         * @param config Algorithm configuration
         */
        explicit ExponentialMovingAverage(const Config &config = Config{});

        void Update(const PitchResult &result) override;
        [[nodiscard]] PitchResult GetStabilized() const override;
        void Reset() override;

    private:
        Config config;
        PitchResult stabilized;
        bool initialized;
    };

    /**
     * @brief Median filter pitch stabilizer
     *
     * Uses sliding window median filter to reject outliers and spikes.
     * Excellent for removing transient noise while preserving edges.
     *
     * Window size affects smoothing:
     * Smaller window → less smoothing, faster response
     * Larger window → more smoothing, better spike rejection
     */
    class MedianFilter : public PitchStabilizer
    {
    public:
        /**
         * @brief Configuration for median filter stabilizer
         */
        struct Config
        {
            uint32_t windowSize = 5; ///< Sliding window size (odd recommended)
        };

        /**
         * @brief Constructs median filter stabilizer
         * @param config Algorithm configuration
         */
        explicit MedianFilter(const Config &config = Config{});

        void Update(const PitchResult &result) override;
        [[nodiscard]] PitchResult GetStabilized() const override;
        void Reset() override;

    private:
        Config config;
        std::vector<PitchResult> window; ///< Circular buffer (pre-allocated)
        uint32_t writeIndex;
        uint32_t sampleCount;

        [[nodiscard]] PitchResult ComputeMedian() const;
    };

    /**
     * @brief Hybrid pitch stabilizer (recommended for guitar tuning)
     *
     * Combines median filter for spike rejection with confidence-weighted EMA.
     * Adapts smoothing based on pitch detection confidence:
     * - High confidence → faster convergence (alpha increases)
     * - Low confidence → more smoothing (alpha decreases)
     *
     * Two-stage processing:
     * 1. Median filter removes spikes/outliers
     * 2. Confidence-weighted EMA smooths remaining jitter
     */
    class HybridStabilizer : public PitchStabilizer
    {
    public:
        /**
         * @brief Configuration for hybrid stabilizer
         */
        struct Config
        {
            float baseAlpha = 0.3f;  ///< Base EMA alpha [0.0, 1.0]
            uint32_t windowSize = 5; ///< Median filter window size
        };

        /**
         * @brief Constructs hybrid stabilizer
         * @param config Algorithm configuration
         */
        explicit HybridStabilizer(const Config &config = Config{});

        void Update(const PitchResult &result) override;
        [[nodiscard]] PitchResult GetStabilized() const override;
        void Reset() override;

    private:
        Config config;
        MedianFilter medianFilter;
        PitchResult emaResult;
        bool initialized;

        [[nodiscard]] float ComputeAdaptiveAlpha(float confidence) const;
    };

} // namespace GuitarDSP
