#include "FFTProcessor.h"

#include <pffft.h>

#include <algorithm>
#include <cmath>

namespace GuitarDSP
{

    float FFTSpectrum::GetMagnitudeAtBin(size_t bin) const
    {
        if (bin >= fftSize / 2)
        {
            return 0.0f;
        }

        size_t binIndex = bin * 2;
        if (binIndex + 1 < data.size())
        {
            float real = data[binIndex];
            float imag = data[binIndex + 1];
            return std::sqrt(real * real + imag * imag);
        }

        return 0.0f;
    }

    float FFTSpectrum::GetMagnitudeAtFrequency(float frequency) const
    {
        if (sampleRate <= 0.0f)
        {
            return 0.0f;
        }

        float binWidth = sampleRate / static_cast<float>(fftSize);
        size_t bin = static_cast<size_t>(frequency / binWidth);

        return GetMagnitudeAtBin(bin);
    }

    float FFTSpectrum::ExtractBandEnergy(float minFreq, float maxFreq) const
    {
        if (sampleRate <= 0.0f)
        {
            return 0.0f;
        }

        float binWidth = sampleRate / static_cast<float>(fftSize);
        size_t minBin = static_cast<size_t>(minFreq / binWidth);
        size_t maxBin = static_cast<size_t>(maxFreq / binWidth);

        maxBin = std::min(maxBin, fftSize / 2);

        float bandEnergy = 0.0f;
        for (size_t i = minBin; i <= maxBin; ++i)
        {
            size_t binIndex = i * 2;
            if (binIndex + 1 < data.size())
            {
                float real = data[binIndex];
                float imag = data[binIndex + 1];
                bandEnergy += real * real + imag * imag;
            }
        }

        return bandEnergy;
    }

    float FFTSpectrum::CalculateSpectralCentroid() const
    {
        float numerator = 0.0f;
        float denominator = 0.0f;

        if (sampleRate <= 0.0f)
        {
            return 0.0f;
        }

        float binWidth = sampleRate / static_cast<float>(fftSize);

        for (size_t i = 0; i < fftSize / 2; ++i)
        {
            size_t binIndex = i * 2;
            if (binIndex + 1 < data.size())
            {
                float real = data[binIndex];
                float imag = data[binIndex + 1];
                float magnitude = std::sqrt(real * real + imag * imag);

                float frequency = static_cast<float>(i) * binWidth;
                numerator += frequency * magnitude;
                denominator += magnitude;
            }
        }

        if (denominator < 1e-6f)
        {
            return 0.0f;
        }

        return numerator / denominator;
    }

    FFTProcessor::FFTProcessor(size_t fftSize, float sampleRate)
        : fftSetup(nullptr)
        , inputBuffer(fftSize, 0.0f)
        , workBuffer(fftSize, 0.0f)
        , spectrum()
    {
        spectrum.fftSize = fftSize;
        spectrum.sampleRate = sampleRate;
        spectrum.data.resize(fftSize, 0.0f);

        fftSetup = pffft_new_setup(static_cast<int>(fftSize), PFFFT_REAL);
    }

    FFTProcessor::~FFTProcessor()
    {
        if (fftSetup)
        {
            pffft_destroy_setup(static_cast<PFFFT_Setup *>(fftSetup));
            fftSetup = nullptr;
        }
    }

    void FFTProcessor::ComputeSpectrum(std::span<const float> audioData)
    {
        size_t copySize = std::min(audioData.size(), inputBuffer.size());
        std::copy_n(audioData.begin(), copySize, inputBuffer.begin());

        if (copySize < inputBuffer.size())
        {
            std::fill(inputBuffer.begin() + copySize, inputBuffer.end(), 0.0f);
        }

        pffft_transform_ordered(static_cast<PFFFT_Setup *>(fftSetup), inputBuffer.data(), spectrum.data.data(),
                                workBuffer.data(), PFFFT_FORWARD);
    }

    const FFTSpectrum &FFTProcessor::GetSpectrum() const
    {
        return spectrum;
    }

} // namespace GuitarDSP