/*
  ==============================================================================

    Filter.cpp
    Created: 6 Dec 2021 1:34:20pm
    Author:  Oddur Kristjansson

  ==============================================================================
*/

#include "Filter.h"
Filter::Filter() {}


Filter::~Filter() {}

void Filter::reset(float sampleRate) {
    b0 = 0.0f;
    b1 = 0.0f;
    a0 = 0.0f;
    a1 = 0.0f;
    prevInput = 0.0f;
    prevOutput = 0.0f;
    this->Fs = sampleRate;
}

void Filter::updateLowShelf(float t60, float fT, float delay, float sampleRate) {

    float gdB = -60/(t60*sampleRate);
   
    float gLin = pow(10,gdB/20);
    
    float del = floor(delay);
    float g = pow(gLin,del);

    float wc = 2.0 * PI * fT / Fs;
    float tc = std::tan(wc * 0.5f);

    b0 = g*tc + sqrt(g);
    b1 = g*tc - sqrt(g);
    a0 = tc + sqrt(g);
    a1 = tc - sqrt(g);
    
    float a0inv = 1/a0;
    b0 *= a0inv;
    b1 *= a0inv;
    a1 *= a0inv;

}

void Filter::updateHighShelf(float t60, float fT, float delay, float sampleRate) {

    float gdB = -60/(t60*sampleRate);
   
    float gLin = pow(10,gdB/20);
    
    float del = floor(delay);
    float g = pow(gLin,del);

    float wc = 2.0 * PI * fT / Fs;
    float tc = std::tan(wc * 0.5f);
    
    b0 = sqrt(g)*tc + g;
    b1 = sqrt(g)*tc - g;
    a0 = sqrt(g)*tc + 1;
    a1 = sqrt(g)*tc - 1;
    
    float a0inv = 1/a0;
    b0 *= a0inv;
    b1 *= a0inv;
    a1 *= a0inv;

}

float Filter::processSample(float input) {
    float output = b0 * input + b1 * prevInput - a1 * prevOutput;
    
    prevInput = input;
    prevOutput = output;
    
    return  output;
}
