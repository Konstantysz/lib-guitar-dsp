#include "MpmPitchDetector.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace GuitarDSP
{

    MpmPitchDetector::MpmPitchDetector(const Config &config)
        : config(config), nsdfBuffer({}), acfBuffer({}), rBuffer({})
    {
    }

    MpmPitchDetector::~MpmPitchDetector() = default;

    void MpmPitchDetector::Reset()
    {
        nsdfBuffer.clear();
        acfBuffer.clear();
        rBuffer.clear();
    }

    std::optional<PitchResult> MpmPitchDetector::Detect(std::span<const float> buffer, float sampleRate)
    {
        if (buffer.empty() || sampleRate <= 0.0f)
        {
            return std::nullopt;
        }

        const size_t bufferSize = buffer.size();

        // Calculate tau range from frequency range
        const auto maxTau = static_cast<size_t>(sampleRate / config.minFrequency);

        if (maxTau >= bufferSize / 2)
        {
            return std::nullopt; // Buffer too small
        }

        // Resize buffers if needed
        const size_t halfSize = bufferSize / 2;
        if (nsdfBuffer.size() != halfSize)
        {
            nsdfBuffer.resize(halfSize);
            acfBuffer.resize(halfSize);
            rBuffer.resize(halfSize);
        }

        // Compute NSDF
        ComputeNSDF(buffer);

        // Find peaks in NSDF
        auto peaks = FindPeaks();

        if (peaks.empty())
        {
            return std::nullopt;
        }

        // Get highest peak
        int maxTauPeak = peaks[0];
        float maxValue = nsdfBuffer[maxTauPeak];

        for (int peak : peaks)
        {
            if (nsdfBuffer[peak] > maxValue)
            {
                maxValue = nsdfBuffer[peak];
                maxTauPeak = peak;
            }
        }

        // Refine with parabolic interpolation
        float refinedTau = ParabolicInterpolation(maxTauPeak);

        if (refinedTau <= 0.0f)
        {
            return std::nullopt;
        }

        const float frequency = sampleRate / refinedTau;

        // Confidence is simply the NSDF value at the peak
        const float confidence = maxValue;

        return PitchResult{ frequency, confidence };
    }

    void MpmPitchDetector::ComputeNSDF(std::span<const float> buffer)
    {
        const size_t bufferSize = buffer.size();
        const size_t halfSize = bufferSize / 2;

        // Compute autocorrelation (ACF) using time-domain method
        for (size_t tau = 0; tau < halfSize; ++tau)
        {
            float sum = 0.0f;
            for (size_t j = 0; j < halfSize; ++j)
            {
                sum += buffer[j] * buffer[j + tau];
            }
            acfBuffer[tau] = sum;
        }

        // Compute r(tau) = sum of squares for normalization
        for (size_t tau = 0; tau < halfSize; ++tau)
        {
            float sum1 = 0.0f;
            float sum2 = 0.0f;

            for (size_t j = 0; j < halfSize; ++j)
            {
                sum1 += buffer[j] * buffer[j];
                sum2 += buffer[j + tau] * buffer[j + tau];
            }

            rBuffer[tau] = sum1 + sum2;
        }

        // Compute NSDF = 2 * ACF(tau) / r(tau)
        for (size_t tau = 0; tau < halfSize; ++tau)
        {
            if (rBuffer[tau] > 0.0f)
            {
                nsdfBuffer[tau] = (2.0f * acfBuffer[tau]) / rBuffer[tau];
            }
            else
            {
                nsdfBuffer[tau] = 0.0f;
            }
        }
    }

    std::vector<int> MpmPitchDetector::FindPeaks()
    {
        std::vector<int> peaks;
        const size_t halfSize = nsdfBuffer.size();

        // Find positive zero crossings
        std::vector<int> zeroCrossings;
        for (size_t i = 1; i < halfSize; ++i)
        {
            if (nsdfBuffer[i - 1] <= 0.0f && nsdfBuffer[i] > 0.0f)
            {
                zeroCrossings.push_back(static_cast<int>(i));
            }
        }

        // Find peaks between zero crossings
        for (size_t i = 0; i < zeroCrossings.size() - 1; ++i)
        {
            int start = zeroCrossings[i];
            int end = zeroCrossings[i + 1];

            // Find maximum in this region
            int maxIdx = start;
            float maxVal = nsdfBuffer[start];

            for (int j = start + 1; j < end; ++j)
            {
                if (nsdfBuffer[j] > maxVal)
                {
                    maxVal = nsdfBuffer[j];
                    maxIdx = j;
                }
            }

            // Only keep peaks above threshold
            if (maxVal >= config.threshold)
            {
                peaks.push_back(maxIdx);
            }
        }

        return peaks;
    }

    float MpmPitchDetector::ParabolicInterpolation(int tau)
    {
        const size_t halfSize = nsdfBuffer.size();

        if (tau <= 0 || tau >= static_cast<int>(halfSize) - 1)
        {
            return static_cast<float>(tau);
        }

        const float s0 = nsdfBuffer[tau - 1];
        const float s1 = nsdfBuffer[tau];
        const float s2 = nsdfBuffer[tau + 1];

        const float adjustment = (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));

        return static_cast<float>(tau) + adjustment;
    }

} // namespace GuitarDSP
