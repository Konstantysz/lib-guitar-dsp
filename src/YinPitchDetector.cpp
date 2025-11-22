#include "YinPitchDetector.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace GuitarDSP
{
    YinPitchDetector::YinPitchDetector(const Config &config) : config_(config)
    {
    }

    YinPitchDetector::~YinPitchDetector() = default;

    std::optional<PitchResult> YinPitchDetector::Detect(const float *buffer, size_t bufferSize, float sampleRate)
    {
        if (!buffer || bufferSize == 0 || sampleRate <= 0.0f)
        {
            return std::nullopt;
        }

        // Calculate tau range from frequency range
        const auto minTau = static_cast<size_t>(sampleRate / config_.maxFrequency);
        const auto maxTau = static_cast<size_t>(sampleRate / config_.minFrequency);
        const size_t halfBufferSize = bufferSize / 2;

        if (maxTau >= halfBufferSize)
        {
            return std::nullopt; // Buffer too small
        }

        // Resize YIN buffer if needed
        if (yinBuffer_.size() < halfBufferSize)
        {
            yinBuffer_.resize(halfBufferSize, 0.0f);
        }

        // Step 1: Calculate difference function
        for (size_t tau = 0; tau < halfBufferSize; ++tau)
        {
            float sum = 0.0f;
            for (size_t i = 0; i < halfBufferSize; ++i)
            {
                const float delta = buffer[i] - buffer[i + tau];
                sum += delta * delta;
            }
            yinBuffer_[tau] = sum;
        }

        // Step 2: Calculate cumulative mean normalized difference function
        yinBuffer_[0] = 1.0f;
        float runningSum = 0.0f;

        for (size_t tau = 1; tau < halfBufferSize; ++tau)
        {
            runningSum += yinBuffer_[tau];
            if (runningSum != 0.0f)
            {
                yinBuffer_[tau] *= static_cast<float>(tau) / runningSum;
            }
            else
            {
                yinBuffer_[tau] = 1.0f;
            }
        }

        // Step 3: Absolute threshold
        size_t tau = minTau;
        while (tau < maxTau)
        {
            if (yinBuffer_[tau] < config_.threshold)
            {
                // Step 4: Parabolic interpolation for sub-sample accuracy
                float betterTau = static_cast<float>(tau);

                if (tau > 0 && tau < halfBufferSize - 1)
                {
                    const float s0 = yinBuffer_[tau - 1];
                    const float s1 = yinBuffer_[tau];
                    const float s2 = yinBuffer_[tau + 1];

                    const float adjustment = (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));
                    betterTau += adjustment;
                }

                const float frequency = sampleRate / betterTau;
                const float confidence = 1.0f - yinBuffer_[tau];

                return PitchResult{ frequency, confidence };
            }
            ++tau;
        }

        return std::nullopt; // No pitch detected
    }

    void YinPitchDetector::Reset()
    {
        std::fill(yinBuffer_.begin(), yinBuffer_.end(), 0.0f);
    }

} // namespace GuitarDSP
