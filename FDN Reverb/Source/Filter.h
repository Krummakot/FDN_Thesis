/*
  ==============================================================================

    Filter.h
    Created: 6 Dec 2021 1:34:20pm
    Author:  Oddur Kristjansson

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <random>

class Filter : public juce::Component {
    
public:
    
    // Constructor function (special function - no return type, name = Class name)
    Filter();
    
    // Destructor
    ~Filter();
    
    void reset(float sampleRate);
    void updateLowShelf(float t60, float fT, float delay, float sampleRate);
    void updateHighShelf(float t60, float fT, float delay, float sampleRate);
    float processSample(float input);
 
private:
    float b0, b1, a0, a1;
    float Fs;
    float prevInput, prevOutput;
    double PI = MathConstants<double>::pi;
};
