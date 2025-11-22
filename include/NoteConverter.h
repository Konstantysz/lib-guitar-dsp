#pragma once

#include <string>
#include <cstdint>

namespace GuitarDSP
{
    /**
     * @brief Note information for display
     */
    struct NoteInfo
    {
        std::string name;      ///< Note name (e.g., "A", "C#")
        int32_t octave;        ///< Octave number
        float cents;           ///< Deviation in cents [-50.0, 50.0]
        float frequency;       ///< Target frequency in Hz
    };

    /**
     * @brief Frequency/note conversion utilities
     *
     * Provides conversions between frequency (Hz) and musical notes
     * with cent-precision for accurate tuning feedback.
     */
    class NoteConverter
    {
    public:
        /**
         * @brief Converts frequency to nearest note with cent deviation
         * @param frequency Frequency in Hz
         * @param a4Frequency Reference A4 frequency (default 440.0 Hz)
         * @return Note information
         */
        [[nodiscard]] static NoteInfo FrequencyToNote(
            float frequency,
            float a4Frequency = 440.0f
        );

        /**
         * @brief Converts note to frequency
         * @param noteName Note name (e.g., "A", "C#", "Bb")
         * @param octave Octave number
         * @param a4Frequency Reference A4 frequency (default 440.0 Hz)
         * @return Frequency in Hz
         */
        [[nodiscard]] static float NoteToFrequency(
            const std::string& noteName,
            int32_t octave,
            float a4Frequency = 440.0f
        );

        /**
         * @brief Calculates cent difference between two frequencies
         * @param frequency1 First frequency in Hz
         * @param frequency2 Second frequency in Hz
         * @return Cent difference (positive if frequency1 > frequency2)
         */
        [[nodiscard]] static float FrequencyToCents(
            float frequency1,
            float frequency2
        );

        /**
         * @brief Returns note name from MIDI note number
         * @param midiNote MIDI note number (0-127)
         * @return Note name (e.g., "C", "C#")
         */
        [[nodiscard]] static std::string MidiNoteToName(int32_t midiNote);

        /**
         * @brief Returns MIDI note number from note name and octave
         * @param noteName Note name (e.g., "A", "C#", "Bb")
         * @param octave Octave number
         * @return MIDI note number (0-127)
         */
        [[nodiscard]] static int32_t NoteNameToMidi(
            const std::string& noteName,
            int32_t octave
        );
    };

} // namespace GuitarDSP
