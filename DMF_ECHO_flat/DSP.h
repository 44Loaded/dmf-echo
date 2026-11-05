#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <cmath>

namespace dmf
{
struct MusicalScale
{
    static constexpr std::array<int, 12> major{{0,2,4,5,7,9,11,0,0,0,0,0}};
    static constexpr std::array<int, 12> minor{{0,2,3,5,7,8,10,0,0,0,0,0}};
    static constexpr std::array<int, 12> harmonicMinor{{0,2,3,5,7,8,11,0,0,0,0,0}};
    static constexpr std::array<int, 12> melodicMinor{{0,2,3,5,7,9,11,0,0,0,0,0}};

    static int noteForDegree (int key, int scale, int degree)
    {
        static constexpr int intervals[11][7] = {
            {0,2,4,5,7,9,11}, {0,2,3,5,7,8,10}, {0,2,3,5,7,8,11},
            {0,2,3,5,7,9,11}, {0,2,3,5,7,9,10}, {0,1,3,5,7,8,10},
            {0,2,4,6,7,9,11}, {0,2,4,5,7,9,10}, {0,1,3,5,6,8,10},
            {0,2,4,7,9,0,0}, {0,3,5,7,10,0,0}
        };
        static constexpr int counts[11] = {7,7,7,7,7,7,7,7,7,5,5};
        const int s = juce::jlimit (0, 10, scale), n = counts[s];
        const int octave = (int) std::floor ((double) degree / n);
        const int index = ((degree % n) + n) % n;
        return key + intervals[s][index] + octave * 12;
    }

    static int chordOffset (int chord, int tone)
    {
        static constexpr int intervals[9][4] = {
            {0,4,7,0}, {0,3,7,0}, {0,3,6,0}, {0,4,8,0}, {0,2,7,0},
            {0,5,7,0}, {0,4,7,11}, {0,3,7,10}, {0,4,7,10}
        };
        static constexpr int counts[9] = {3,3,3,3,3,3,4,4,4};
        const int c = juce::jlimit (0, 8, chord);
        return intervals[c][tone % counts[c]];
    }
};

// Two overlapping modulated read heads. This compact delay-line shifter is intended
// for musical echo feedback, where modest grain modulation is masked by repeats.
class PitchShifter
{
public:
    void prepare (int maximumBlock, double sr)
    {
        sampleRate = sr;
        const int size = 16384;
        buffer.assign ((size_t) size, 0.0f);
        write = 0;
        phase = 0.0;
    }
    void reset() { std::fill (buffer.begin(), buffer.end(), 0.0f); write = 0; phase = 0.0; }
    float process (float in, float ratio)
    {
        if (buffer.empty()) return in;
        constexpr double grain = 2048.0;
        const double r = juce::jlimit (0.5, 2.0, (double) ratio);
        buffer[(size_t) write] = in;
        const double p = phase;
        const double p2 = std::fmod (p + 0.5, 1.0);
        const double offset1 = 64.0 + p * grain * (1.0 - r);
        const double offset2 = 64.0 + p2 * grain * (1.0 - r);
        const float a = read (offset1);
        const float b = read (offset2);
        const float wa = (float) (0.5 - 0.5 * std::cos (juce::MathConstants<double>::twoPi * p));
        const float wb = (float) (0.5 - 0.5 * std::cos (juce::MathConstants<double>::twoPi * p2));
        const float out = (a * wa + b * wb) / juce::jmax (0.001f, wa + wb);
        phase += 1.0 / grain;
        phase -= std::floor (phase);
        write = (write + 1) % (int) buffer.size();
        return out;
    }
private:
    float read (double delay) const
    {
        const double pos = (double) write - delay;
        const int size = (int) buffer.size();
        double wrapped = std::fmod (pos, (double) size); if (wrapped < 0) wrapped += size;
        const int i0 = (int) wrapped, i1 = (i0 + 1) % size;
        const float frac = (float) (wrapped - i0);
        return buffer[(size_t) i0] + frac * (buffer[(size_t) i1] - buffer[(size_t) i0]);
    }
    std::vector<float> buffer;
    int write = 0;
    double phase = 0.0, sampleRate = 44100.0;
};

class DelayEngine
{
public:
    void prepare (double sr, int channels)
    {
        sampleRate = sr; const int n = (int) (sr * 8.0) + 8;
        for (auto& b : delay) b.assign ((size_t) n, 0.0f);
        for (auto& p : shifter) p.prepare (512, sr);
        write = 0; counter = 0;
    }
    void reset()
    {
        for (auto& b : delay) std::fill (b.begin(), b.end(), 0.0f);
        for (auto& p : shifter) p.reset();
        write = counter = 0; repeat = 0;
    }
    void process (juce::AudioBuffer<float>& audio, float delayMs, float feedback, float mix,
                  float mutate, float space, float dirt, bool sync, float bpm, int division,
                  int key, int scale, int motion, bool chordOn, int chord)
    {
        const int channels = audio.getNumChannels();
        const int total = delay[0].size();
        double ms = delayMs;
        if (sync) {
            static constexpr double beats[] = {4.0, 2.0, 1.0, 0.5, 0.25, 6.0, 3.0, 1.5, 0.75, 0.375, 2.6666667, 1.3333333, 0.6666667, 0.3333333};
            ms = 60000.0 / juce::jmax (30.0f, bpm) * beats[juce::jlimit (0, 13, division)];
        }
        const int delaySamples = juce::jlimit (1, total - 4, (int) (sampleRate * ms / 1000.0));
        const int eventPeriod = delaySamples;
        for (int i = 0; i < audio.getNumSamples(); ++i)
        {
            if (counter++ >= eventPeriod) { counter = 0; ++repeat; }
            int degree = 0;
            switch (motion) {
                case 1: degree = repeat; break;
                case 2: degree = -repeat; break;
                case 3: degree = (repeat % 2 == 0 ? repeat / 2 : -(repeat + 1) / 2); break;
                case 4: degree = (int) ((repeat * 7 + 3) % 9) - 4; break;
                default: degree = 0; break;
            }
            int semitones = 0;
            if (mutate > 0.001f && motion != 0) {
                const int scaledDegree = (int) std::round ((float) degree * mutate);
                if (chordOn) {
                    const int tone = std::abs (scaledDegree);
                    const int chordSize = (chord >= 6 ? 4 : 3);
                    const int magnitude = std::abs (scaledDegree);
                    const int chordInterval = MusicalScale::chordOffset (chord, tone);
                    const int octave = magnitude / chordSize;
                    semitones = chordInterval + octave * 12;
                    if (scaledDegree < 0) semitones = chordInterval - (octave + (chordInterval == 0 ? 0 : 1)) * 12;
                } else semitones = MusicalScale::noteForDegree (key, scale, scaledDegree) - key;
            }
            const float ratio = std::pow (2.0f, semitones / 12.0f);
            float inL = audio.getSample (0, i), inR = channels > 1 ? audio.getSample (1, i) : inL;
            const float dL = delay[0][(size_t) ((write - delaySamples + total) % total)];
            const float dR = delay[1][(size_t) ((write - delaySamples + total) % total)];
            float wetL = shifter[0].process (dL, ratio), wetR = shifter[1].process (dR, ratio);
            const float mid = (wetL + wetR) * 0.5f;
            const float side = (wetL - wetR) * 0.5f * (0.25f + space * 1.5f);
            wetL = mid + side; wetR = mid - side;
            const float drive = 1.0f + dirt * 8.0f;
            wetL = std::tanh (wetL * drive) / std::tanh (drive);
            wetR = std::tanh (wetR * drive) / std::tanh (drive);
            delay[0][(size_t) write] = juce::jlimit (-1.2f, 1.2f, inL + wetL * feedback * 0.92f);
            delay[1][(size_t) write] = juce::jlimit (-1.2f, 1.2f, inR + wetR * feedback * 0.92f);
            audio.setSample (0, i, inL * (1.0f - mix) + wetL * mix);
            if (channels > 1) audio.setSample (1, i, inR * (1.0f - mix) + wetR * mix);
            write = (write + 1) % total;
        }
    }
private:
    std::array<std::vector<float>, 2> delay;
    std::array<PitchShifter, 2> shifter;
    int write = 0, counter = 0; uint64_t repeat = 0;
    double sampleRate = 44100.0;
};
}
