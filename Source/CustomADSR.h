/*
  ==============================================================================

    CustomADSR.h
    Created: 27 Apr 2023 2:28:57pm
    Author:  Andrew Orals

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class CustomADSR
{
public:
  typedef std::function<float(float)> Envelope;

  struct Parameters : public juce::ADSR::Parameters
  {
    template <int num, int den>
    static inline Envelope EXP_GRO_ENV = [](float x)
    {
        return (static_cast<float>(den) / num) * std::log((std::exp(static_cast<float>(num) / den) - 1.0f) * x + 1.0f);
    };

    template <int num, int den>
    static inline Envelope EXP_DEC_ENV = [](float x)
    {
        return (-static_cast<float>(den) / num) * std::log((std::exp(static_cast<float>(num) / den) - 1) * x + 1.0f) + 1.0f;
    };

    template <int num, int den>
    static inline Envelope POLY_ENV = [](float x)
    {
        return std::powf(x, static_cast<float>(num) / den);
    };

    Parameters() = default;

    Parameters(float a, float d, float s, float r, int env_res=2, float max_amp=1.0f)
      : juce::ADSR::Parameters(a, d, s, r),
        env_resolution(env_res),
        maxAmp(max_amp)
    {
        jassert(env_res >= 2 && env_res <= 4096);
        jassert(max_amp >= 0.0f && max_amp <= 1.0f);
    }

    Parameters(float attackTimeSeconds,
               float decayTimeSeconds,
               float sustainTimeSeconds,
               float releaseTimeSeconds,
               float maxAmplitude,
               const Envelope& attackEnvFunc,
               const Envelope& decayEnvFunc,
               const Envelope& releaseEnvFunc,
               size_t env_resolution=256)
      : juce::ADSR::Parameters(attackTimeSeconds,
                               decayTimeSeconds,
                               sustainTimeSeconds,
                               releaseTimeSeconds),
        env_resolution(env_resolution),
        maxAmp(maxAmplitude),
        attackEnv(attackEnvFunc),
        decayEnv(decayEnvFunc),
        releaseEnv(releaseEnvFunc)
    {
      jassert(env_resolution >= 2 && env_resolution <= 4096 /* arbitrary */);
        jassert(maxAmplitude >= 0.0f && maxAmplitude <= 1.0f);
    }

    size_t env_resolution = 2; // number of tabulations for each envelope
    float maxAmp = 1.0f;
    Envelope attackEnv = [](float x) { return x; },
             decayEnv = [](float x) { return -x; },
             releaseEnv = [](float x) { return -x; };
  };

  CustomADSR(const Parameters& newParameters);

  void setParameters (const Parameters& newParameters);

  void setParameters (const juce::ADSR::Parameters& newParameters);

  const Parameters& getParameters() const noexcept;

  bool isActive() const noexcept;

  void setSampleRate (double newSampleRate) noexcept;

  void reset() noexcept;

  void noteOn() noexcept;

  void noteOff() noexcept;

  float getNextSample() noexcept;

  template <typename FloatType>
  void applyEnvelopeToBuffer(juce::AudioBuffer<FloatType>& buffer,
                             int startSample,
                             int numSamples);

private:
  typedef enum
  {
    Idle,
    Attack,
    Decay,
    Sustain,
    Release
  } State;

  void gotoState(State s) noexcept;

  void calcRate(State s) noexcept;

  void recalculateRates() noexcept;

  void tabulateEnvelopes();

  State adsr_state_ = Idle;
  Parameters parameters_;
  double sample_rate_ = 44100.0;
  float curr_amplitude_ = 0.0f;

  std::vector<float> attack_env_table_, // tabulations of the envelope
                                        // functions
                     decay_env_table_,
                     release_env_table_;

  int attack_table_rate_ = 1, // after how many samples to move to
                              // the next rate
      decay_table_rate_ = 1,
      release_table_rate_ = 1;

  int attack_samples_ = 0, // number of samples elapsed for each
                           // state; reset on state change
      decay_samples_ = 0,
      release_samples_ = 0;

  unsigned int attack_idx_ = 0, // index into the respective tables
               decay_idx_ = 0,
               release_idx_ = 0;

  float curr_attack_rate_ = 0.0f,
        curr_decay_rate_ = 0.0f,
        curr_release_rate_ = 0.0f;

  float release_start_amp_ = 1.0f; // the amplitude at which release starts
};

