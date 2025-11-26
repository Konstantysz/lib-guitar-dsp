#include "PitchStabilizer.h"
#include <algorithm>
#include <cmath>

namespace GuitarDSP
{
    ExponentialMovingAverage::ExponentialMovingAverage(const EMAConfig &config)
        : config(config), stabilized{ 0.0f, 0.0f }, initialized(false)
    {
    }

    void ExponentialMovingAverage::Update(const PitchResult &result)
    {
        if (!initialized)
        {
            // First sample: initialize with input
            stabilized = result;
            initialized = true;
        }
        else
        {
            // EMA formula: smoothed = alpha * new + (1-alpha) * previous
            stabilized.frequency = config.alpha * result.frequency + (1.0f - config.alpha) * stabilized.frequency;
            stabilized.confidence = config.alpha * result.confidence + (1.0f - config.alpha) * stabilized.confidence;
        }
    }

    PitchResult ExponentialMovingAverage::GetStabilized() const
    {
        return stabilized;
    }

    void ExponentialMovingAverage::Reset()
    {
        stabilized = { 0.0f, 0.0f };
        initialized = false;
    }

    MedianFilter::MedianFilter(const MedianFilterConfig &config) : config(config), writeIndex(0), sampleCount(0)
    {
        // Pre-allocate window buffer (real-time safe)
        window.resize(config.windowSize, { 0.0f, 0.0f });
    }

    void MedianFilter::Update(const PitchResult &result)
    {
        // Add to circular buffer
        window[writeIndex] = result;
        writeIndex = (writeIndex + 1) % config.windowSize;

        if (sampleCount < config.windowSize)
        {
            sampleCount++;
        }
    }

    PitchResult MedianFilter::GetStabilized() const
    {
        return ComputeMedian();
    }

    void MedianFilter::Reset()
    {
        writeIndex = 0;
        sampleCount = 0;
        std::fill(window.begin(), window.end(), PitchResult{ 0.0f, 0.0f });
    }

    PitchResult MedianFilter::ComputeMedian() const
    {
        if (sampleCount == 0)
        {
            return { 0.0f, 0.0f };
        }

        // Create sorted copies for median calculation (not real-time safe, but small arrays)
        std::vector<float> frequencies;
        std::vector<float> confidences;

        frequencies.reserve(sampleCount);
        confidences.reserve(sampleCount);

        // Copy active samples
        for (uint32_t i = 0; i < sampleCount; ++i)
        {
            frequencies.push_back(window[i].frequency);
            confidences.push_back(window[i].confidence);
        }

        // Sort and find median
        std::sort(frequencies.begin(), frequencies.end());
        std::sort(confidences.begin(), confidences.end());

        uint32_t midIndex = sampleCount / 2;

        PitchResult median;
        if (sampleCount % 2 == 0)
        {
            // Even count: average of two middle values
            median.frequency = (frequencies[midIndex - 1] + frequencies[midIndex]) * 0.5f;
            median.confidence = (confidences[midIndex - 1] + confidences[midIndex]) * 0.5f;
        }
        else
        {
            // Odd count: middle value
            median.frequency = frequencies[midIndex];
            median.confidence = confidences[midIndex];
        }

        return median;
    }

    HybridStabilizer::HybridStabilizer(const HybridStabilizerConfig &config)
        : config(config), medianFilter(MedianFilterConfig{ config.windowSize }), emaResult{ 0.0f, 0.0f },
          initialized(false)
    {
    }

    void HybridStabilizer::Update(const PitchResult &result)
    {
        // Stage 1: Median filter for spike rejection
        medianFilter.Update(result);
        PitchResult medianFiltered = medianFilter.GetStabilized();

        // Stage 2: Confidence-weighted EMA
        if (!initialized)
        {
            emaResult = medianFiltered;
            initialized = true;
        }
        else
        {
            // Adaptive alpha based on confidence
            float adaptiveAlpha = ComputeAdaptiveAlpha(medianFiltered.confidence);

            // Apply EMA with adaptive smoothing
            emaResult.frequency =
                adaptiveAlpha * medianFiltered.frequency + (1.0f - adaptiveAlpha) * emaResult.frequency;
            emaResult.confidence =
                adaptiveAlpha * medianFiltered.confidence + (1.0f - adaptiveAlpha) * emaResult.confidence;
        }
    }

    PitchResult HybridStabilizer::GetStabilized() const
    {
        return emaResult;
    }

    void HybridStabilizer::Reset()
    {
        medianFilter.Reset();
        emaResult = { 0.0f, 0.0f };
        initialized = false;
    }

    float HybridStabilizer::ComputeAdaptiveAlpha(float confidence) const
    {
        // High confidence → higher alpha → faster response
        // Low confidence → lower alpha → more smoothing
        // Formula: alpha = baseAlpha * (1.0 + confidence)
        // Confidence range [0.0, 1.0] → alpha range [baseAlpha, 2*baseAlpha]

        float adaptiveAlpha = config.baseAlpha * (1.0f + confidence);

        // Clamp to valid range [0.0, 1.0]
        return std::clamp(adaptiveAlpha, 0.0f, 1.0f);
    }

} // namespace GuitarDSP
