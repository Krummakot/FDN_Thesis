/*
  ==============================================================================

    FDN.hpp
    Created: 8 Oct 2021 9:45:54am
    Author:  Oddur Kristjansson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <random>
#include "Filter.h"


class FDN : public juce::Component {
    
public:
    
    // Constructor function (special function - no return type, name = Class name)
    FDN();
    
    // Destructor
    ~FDN();

    void reset();
    
    void init(float sampleRate, int nrDel, float loDel, float higDel);
    
    void updateFDN(int nrDel, int matrixSelection);
    
    void prepare(const dsp::ProcessSpec& spec);
    
    void setBGains(std::vector<float> gains);
    
    void setSingleBGain(int index, float newGain);
    
    String getBGains();
    
    void setCGains(std::vector<float> gains);
    
    void setSingleCGain(int index, float newGain);
    
    String getCGains();
    
    void updateDryMix(float dryWet);
    
    void updateDelay(float newDelay);
    
    void setDelayOSCWhole(std::vector<float> newDelayVector);
    
    void setDelayOSCSingle(int index, int newDelay);
    
    String getDelayValues();
        
    void updateFilter(float gDC, float gPI, float l_fT, float h_fT);
    
    float processFDN(int channel, float input);
    
    void findNPrime(int LR, int UR, int N);
    
    void updateMatrixCoefficients(std::vector<float> newMatrixCoef, int matrixSelection);
    
    void updateMixingMatrix(float frac);
    
    void updateMatrixCoefficientsOSC(std::vector<float> newMatrixCoef, String singleWhole);
    
    void updateModulation(float newDepth, float newRate);
    
    void updateModDepthOSCSingle(int index, float newDepth);

    void updateModRateOSCSingle(int index, float newRate);
    void setModDepth(float newDepth);
    void setModRate(float newRate);
    
    String getMatrixValues();

    float randomFloat(float min, float max);

    std::vector<int> delayLength;
    
    std::vector<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd>> dspDelayLines;


    Filter* lowShelf;
    Filter* highShelf;
    Filter* endHighShelf;
    std::vector<SmoothedValue<float, ValueSmoothingTypes::Linear>> delayLineSmoother;
    
    dsp::Matrix<float>* mixingMatrix;
    dsp::Matrix<float>* delayLineInputMatrix;
    dsp::Matrix<float>* delayLineOutputMatrix;
    
    Array<float> coefs;
    
    int nrDelayLines;
    float Fs;
    
    float lowDelay;
    float highDelay;
    float d; // direct path
    std::vector<float> bGains;
    std::vector<float> cGains;
          
private:
    enum
    {
        maxDelaySamples = 64 * 8192,
        maxDelayLines = 32,
    };

    // filter coefficients
    float b0, b1, a0, a1;   //filter coefficients
    float gDC, gPI, gFT, wT;   //parameters input by user
    
//    float* matrixCoefficients;
    Array<float> matrixCoefficients;

    std::string bGainsString = "";
    std::string cGainsString = "";
    std::string delayValuesString = "";
    
    std::vector<dsp::Oscillator<float>> lfos;
   
    float PI = MathConstants<double>::pi;
    bool updatingFDNOrder = false;
    int delayUpdate = 0;
    std::vector<float> modDepth;
};
