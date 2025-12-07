#pragma once

#include <span>
#include <vector>

namespace GuitarDSP
{

    struct FFTSpectrum
    {
        std::vector<float> data;
        size_t fftSize;
        float sampleRate;

        float GetMagnitudeAtBin(size_t bin) const;
        float GetMagnitudeAtFrequency(float frequency) const;
        float ExtractBandEnergy(float minFreq, float maxFreq) const;
        float CalculateSpectralCentroid() const;
    };

    class FFTProcessor
    {
    public:
        FFTProcessor(size_t fftSize, float sampleRate);
        ~FFTProcessor();

        FFTProcessor(const FFTProcessor &) = delete;
        FFTProcessor &operator=(const FFTProcessor &) = delete;
        FFTProcessor(FFTProcessor &&) = delete;
        FFTProcessor &operator=(FFTProcessor &&) = delete;

        void ComputeSpectrum(std::span<const float> audioData);
        const FFTSpectrum &GetSpectrum() const;

    private:
        void *fftSetup;
        std::vector<float> inputBuffer;
        std::vector<float> workBuffer;
        FFTSpectrum spectrum;
    };

} // namespace GuitarDSP