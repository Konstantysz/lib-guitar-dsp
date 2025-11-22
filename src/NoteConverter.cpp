#include "NoteConverter.h"
#include <array>
#include <cmath>
#include <stdexcept>

namespace GuitarDSP
{
    namespace
    {
        // Note names in chromatic scale
        constexpr std::array<const char *, 12>
            NOTE_NAMES = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

        // A4 MIDI note number
        constexpr int32_t A4_MIDI = 69;

        // Semitones per octave
        constexpr float SEMITONES_PER_OCTAVE = 12.0f;

        // Cents per semitone
        constexpr float CENTS_PER_SEMITONE = 100.0f;
    } // namespace

    NoteInfo NoteConverter::FrequencyToNote(float frequency, float a4Frequency)
    {
        if (frequency <= 0.0f || a4Frequency <= 0.0f)
        {
            return NoteInfo{ "", 0, 0.0f, 0.0f };
        }

        // Calculate semitones from A4
        const float semitonesFromA4 = SEMITONES_PER_OCTAVE * std::log2(frequency / a4Frequency);

        // Calculate cent deviation
        const int32_t nearestNote = static_cast<int32_t>(std::round(semitonesFromA4)) + A4_MIDI;
        const float nearestFrequency =
            a4Frequency * std::pow(2.0f, static_cast<float>(nearestNote - A4_MIDI) / SEMITONES_PER_OCTAVE);
        const float cents = FrequencyToCents(frequency, nearestFrequency);

        // Extract note name and octave
        const int32_t noteIndex = nearestNote % 12;
        const int32_t octave = (nearestNote / 12) - 1;

        return NoteInfo{ NOTE_NAMES[noteIndex], octave, cents, nearestFrequency };
    }

    float NoteConverter::NoteToFrequency(const std::string &noteName, int32_t octave, float a4Frequency)
    {
        const int32_t midiNote = NoteNameToMidi(noteName, octave);
        const float semitonesFromA4 = static_cast<float>(midiNote - A4_MIDI);
        return a4Frequency * std::pow(2.0f, semitonesFromA4 / SEMITONES_PER_OCTAVE);
    }

    float NoteConverter::FrequencyToCents(float frequency1, float frequency2)
    {
        if (frequency1 <= 0.0f || frequency2 <= 0.0f)
        {
            return 0.0f;
        }

        return SEMITONES_PER_OCTAVE * CENTS_PER_SEMITONE * std::log2(frequency1 / frequency2);
    }

    std::string NoteConverter::MidiNoteToName(int32_t midiNote)
    {
        if (midiNote < 0 || midiNote > 127)
        {
            return "";
        }

        return NOTE_NAMES[midiNote % 12];
    }

    int32_t NoteConverter::NoteNameToMidi(const std::string &noteName, int32_t octave)
    {
        // Find note index
        int32_t noteIndex = -1;
        for (size_t i = 0; i < NOTE_NAMES.size(); ++i)
        {
            if (noteName == NOTE_NAMES[i])
            {
                noteIndex = static_cast<int32_t>(i);
                break;
            }
        }

        // Handle flat notation (e.g., "Bb" -> "A#")
        if (noteIndex == -1 && noteName.length() == 2 && noteName[1] == 'b')
        {
            for (size_t i = 0; i < NOTE_NAMES.size(); ++i)
            {
                if (i > 0 && noteName[0] == NOTE_NAMES[i][0])
                {
                    noteIndex = static_cast<int32_t>(i - 1);
                    break;
                }
            }
        }

        if (noteIndex == -1)
        {
            throw std::invalid_argument("Invalid note name: " + noteName);
        }

        return (octave + 1) * 12 + noteIndex;
    }

} // namespace GuitarDSP
