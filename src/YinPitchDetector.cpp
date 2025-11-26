#include "YinPitchDetector.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace GuitarDSP
{
    YinPitchDetector::YinPitchDetector(const YinPitchDetectorConfig &config) : config(config), yinBuffer({})
    {
    }

    YinPitchDetector::~YinPitchDetector() = default;

    std::optional<PitchResult> YinPitchDetector::Detect(std::span<const float> buffer, float sampleRate)
    {
        if (buffer.empty() || sampleRate <= 0.0f)
        {
            return std::nullopt;
        }

        const size_t bufferSize = buffer.size();

        // Calculate tau range from frequency range
        const auto minTau = static_cast<size_t>(sampleRate / config.maxFrequency);
        const auto maxTau = static_cast<size_t>(sampleRate / config.minFrequency);
        const size_t halfBufferSize = bufferSize / 2;

        if (maxTau >= halfBufferSize)
        {
            return std::nullopt; // Buffer too small
        }

        // Resize YIN buffer if needed
        if (yinBuffer.size() < halfBufferSize)
        {
            yinBuffer.resize(halfBufferSize, 0.0f);
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
            yinBuffer[tau] = sum;
        }

        // Step 2: Calculate cumulative mean normalized difference function
        yinBuffer[0] = 1.0f;
        float runningSum = 0.0f;

        for (size_t tau = 1; tau < halfBufferSize; ++tau)
        {
            runningSum += yinBuffer[tau];
            if (runningSum != 0.0f)
            {
                yinBuffer[tau] *= static_cast<float>(tau) / runningSum;
            }
            else
            {
                yinBuffer[tau] = 1.0f;
            }
        }

        // Step 3: Absolute threshold
        size_t tau = minTau;
        while (tau < maxTau)
        {
            if (yinBuffer[tau] < config.threshold)
            {
                // Step 4: Parabolic interpolation for sub-sample accuracy
                float betterTau = static_cast<float>(tau);

                if (tau > 0 && tau < halfBufferSize - 1)
                {
                    const float s0 = yinBuffer[tau - 1];
                    const float s1 = yinBuffer[tau];
                    const float s2 = yinBuffer[tau + 1];

                    const float adjustment = (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));
                    betterTau += adjustment;
                }

                const float frequency = sampleRate / betterTau;
                const float confidence = 1.0f - yinBuffer[tau];

                return PitchResult{ frequency, confidence };
            }
            ++tau;
        }

        return std::nullopt; // No pitch detected
    }

    void YinPitchDetector::Reset()
    {
        std::fill(yinBuffer.begin(), yinBuffer.end(), 0.0f);
    }

} // namespace GuitarDSP
