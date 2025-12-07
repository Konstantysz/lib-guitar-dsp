#pragma once

#include <span>
#include <vector>

namespace GuitarDSP
{
    /**
     * @brief FFT spectrum data and analysis methods
     *
     * Stores frequency-domain representation of audio signal and provides
     * methods for spectral analysis.
     */
    struct FFTSpectrum
    {
        std::vector<float> data; ///< Interleaved complex FFT data [real0, imag0, real1, imag1, ...]
        size_t fftSize;          ///< FFT size (number of bins)
        float sampleRate;        ///< Sample rate (Hz)

        /**
         * @brief Get magnitude at specific FFT bin
         * @param bin FFT bin index [0, fftSize/2)
         * @return Magnitude at bin
         */
        float GetMagnitudeAtBin(size_t bin) const;

        /**
         * @brief Get magnitude at specific frequency
         * @param frequency Target frequency (Hz)
         * @return Magnitude at nearest FFT bin to frequency
         */
        float GetMagnitudeAtFrequency(float frequency) const;

        /**
         * @brief Extract total energy in frequency band
         * @param minFreq Minimum frequency (Hz)
         * @param maxFreq Maximum frequency (Hz)
         * @return Sum of magnitudes in frequency band
         */
        float ExtractBandEnergy(float minFreq, float maxFreq) const;

        /**
         * @brief Calculate spectral centroid (brightness measure)
         * @return Spectral centroid (Hz)
         */
        float CalculateSpectralCentroid() const;
    };

    /**
     * @brief Fast Fourier Transform processor using PFFFT
     *
     * Encapsulates PFFFT library for real-time audio analysis.
     * Provides efficient FFT computation with SIMD optimization.
     *
     * Thread-safe: Use separate instances per thread.
     * Real-time safe: Pre-allocates all buffers in constructor.
     *
     * @note FFT size must be power of 2 (PFFFT requirement)
     */
    class FFTProcessor
    {
    public:
        /**
         * @brief Constructs FFT processor
         * @param fftSize FFT size (must be power of 2, typically 2048)
         * @param sampleRate Sample rate (Hz, typically 48000.0f)
         */
        FFTProcessor(size_t fftSize, float sampleRate);

        ~FFTProcessor();

        FFTProcessor(const FFTProcessor &) = delete;
        FFTProcessor &operator=(const FFTProcessor &) = delete;
        FFTProcessor(FFTProcessor &&) = delete;
        FFTProcessor &operator=(FFTProcessor &&) = delete;

        /**
         * @brief Compute FFT spectrum from audio data
         * @param audioData Input audio samples (must be >= fftSize)
         *
         * Real-time safe: No allocations, uses pre-allocated buffers.
         */
        void ComputeSpectrum(std::span<const float> audioData);

        /**
         * @brief Get computed spectrum
         * @return Reference to most recent spectrum
         */
        const FFTSpectrum &GetSpectrum() const;

    private:
        void *fftSetup;                 ///< PFFFT setup handle
        std::vector<float> inputBuffer; ///< Pre-allocated input buffer
        std::vector<float> workBuffer;  ///< Pre-allocated work buffer for PFFFT
        FFTSpectrum spectrum;           ///< Current spectrum data
    };

} // namespace GuitarDSP
