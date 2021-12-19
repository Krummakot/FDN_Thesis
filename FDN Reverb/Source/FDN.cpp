/*
  ==============================================================================

    FDN.cpp
    Created: 8 Oct 2021 9:45:54am
    Author:  Oddur Kristjansson

  ==============================================================================
*/

#include "FDN.hpp"

FDN::FDN() {}


FDN::~FDN() {

    delete mixingMatrix;
    delete delayLineInputMatrix;
    delete delayLineOutputMatrix;
    delete [] lowShelf;
    delete [] highShelf;
}

void FDN::reset() {
    for(int i = 0; i < nrDelayLines; ++i) {
        dspDelayLines[i].reset();
    }
}

void FDN::init(float sampleRate, int nrDel, float loDel, float highDel) {
    Fs = sampleRate;
    nrDelayLines = nrDel;

    lowDelay = loDel;
    highDelay = highDel;
    bGains.resize(maxDelayLines);
    cGains.resize(maxDelayLines);

    delayLength.resize(maxDelayLines);
    findNPrime((int)(lowDelay * Fs/1000.0), (int)(highDelay * Fs/1000.0), nrDelayLines);
        
    dspDelayLines.resize(maxDelayLines);
    
    lowShelf = new Filter[maxDelayLines];
    highShelf = new Filter[maxDelayLines];
    endHighShelf = new Filter[maxDelayLines];

    lfos.resize(maxDelayLines);
    delayLineSmoother.resize(maxDelayLines);

    int size = nrDel*nrDel;
    coefs.resize(size);

    for (int i = 0; i < maxDelayLines; ++i) {
        dspDelayLines[i] = dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Lagrange3rd>(maxDelaySamples);
        dspDelayLines[i].setDelay(delayLength[i]);
        lfos[i] = dsp::Oscillator<float>{[](float x) {return std::sin(x);}};
        bGains[i] = randomFloat(-1.f, 1.f);
        cGains[i] = randomFloat(-1.f, 1.f);
        lfos[i].setFrequency(randomFloat(0.0f, 2.f));
        delayLineSmoother[i].reset(Fs, 0.05f);
        delayLineSmoother[i].setCurrentAndTargetValue(delayLength[i]);
        lowShelf[i].reset(Fs);
        highShelf[i].reset(Fs);
        endHighShelf[i].reset(Fs);
    }

    matrixCoefficients.resize(nrDelayLines*nrDelayLines);
    mixingMatrix = new dsp::Matrix<float>(dsp::Matrix<float>::identity(nrDelayLines));
    delayLineInputMatrix = new dsp::Matrix<float>(nrDelayLines, 1); // all zeros
    delayLineOutputMatrix = new dsp::Matrix<float>(nrDelayLines, 1); // all zeros
    
}


void FDN::updateFDN(int nrDel, int matrixSelection) {
    updatingFDNOrder = true;
    nrDelayLines = nrDel;
    reset();
    for(int i = 0; i < nrDelayLines; ++i) {
        bGains[i] = randomFloat(-1.f, 1.f);
        cGains[i] = randomFloat(-1.f, 1.f);
    }
    findNPrime((int)(lowDelay * Fs/1000.0), (int)(highDelay * Fs/1000.0), nrDelayLines);
    
    for (int i = 0; i < nrDel; ++i) {
        dspDelayLines[i].setDelay(delayLength[i]);
        delayLineSmoother[i].setCurrentAndTargetValue(delayLength[i]);
    }

    matrixCoefficients.resize(nrDelayLines*nrDelayLines);
    if(matrixSelection == 1) {
        mixingMatrix = new dsp::Matrix<float>(dsp::Matrix<float>::identity(nrDelayLines));
    }
    if(matrixSelection == 2) {
        mixingMatrix = new dsp::Matrix<float>(nrDelayLines, nrDelayLines, matrixCoefficients.getRawDataPointer());
    }
    delayLineInputMatrix = new dsp::Matrix<float>(nrDelayLines, 1); // all zeros
    delayLineOutputMatrix = new dsp::Matrix<float>(nrDelayLines, 1); // all zeros
    updatingFDNOrder = false;
}

void FDN::prepare(const dsp::ProcessSpec& spec) {
    for(int i = 0; i < nrDelayLines; ++i) {
        dspDelayLines[i].prepare(spec);
        lfos[i].prepare(spec);
    }
}

void FDN::setBGains(std::vector<float> gains) {
    for(int i = 0; i < nrDelayLines; ++i) {
        bGains[i] = gains[i];
    }
}

void FDN::setSingleBGain(int index, float newGain) {
    bGains[index] = newGain;
}
    
void FDN::setCGains(std::vector<float> gains) {
    for(int i = 0; i < nrDelayLines; ++i) {
        cGains[i] = gains[i];
    }
}

void FDN::setSingleCGain(int index, float newGain) {
    cGains[index] = newGain;
}

void FDN::updateDryMix(float dryWet) {
    d = dryWet;
}

void FDN::updateFilter(float g_DC, float g_PI, float l_fT, float h_fT) {
    for (int i = 0; i < nrDelayLines; ++i) {
        lowShelf[i].updateLowShelf(g_DC, l_fT, delayLength[i], Fs);
        highShelf[i].updateHighShelf(g_PI, h_fT, delayLength[i], Fs);
        endHighShelf[i].updateHighShelf(g_PI, 20200.f, delayLength[i], Fs);
        }
}

void FDN::updateDelay(float newDelay) {
    lowDelay = newDelay * 0.6;
    highDelay = newDelay;
    findNPrime((int)(lowDelay * Fs/1000.0), (int)(highDelay * Fs/1000.0), nrDelayLines);
    
    for(int i = 0; i < nrDelayLines; ++i) {
        delayLineSmoother[i].setTargetValue(delayLength[i]);
        while (delayLength[i] != delayLineSmoother[i].getNextValue()) {
            dspDelayLines[i].setDelay(delayLineSmoother[i].getNextValue());
        }
    }
}

void FDN::setDelayOSCWhole(std::vector<float> newDelayVector) {
    
    for(int i = 0; i < nrDelayLines; ++i) {
        dspDelayLines[i].reset();
        delayLength[i] = newDelayVector[i] * Fs/1000.0f;
        dspDelayLines[i].setDelay(delayLength[i]);
    }
}

void FDN::setDelayOSCSingle(int index, int newDelay) {
    delayLength[index] = newDelay * Fs/1000.0f;
    dspDelayLines[index].setDelay(delayLength[index]);
}

void FDN::updateModulation(float newDepth, float newRate) {
    modDepth = newDepth;
    for(int i = 0; i < nrDelayLines; ++i) {
        lfos[i].setFrequency(newRate);
        float delay = delayLength[i];
        float lfoOut = modDepth * lfos[i].processSample(0.0f);

        float newDelay = delay + lfoOut;
        
        if(newDelay != 0) {
            dspDelayLines[i].setDelay(newDelay);
        }
    }
}
    
float FDN::processFDN(int channel, float input) {
    float out = 0.0f;
  
    
//    updateModulation();
    if (!updatingFDNOrder) {
    
    for (int i = 0; i < nrDelayLines; ++i) {
        dspDelayLines[i].pushSample(channel, bGains[i] * input + delayLineInputMatrix->operator()(i, 0));
        delayLineOutputMatrix->operator()(i, 0) = endHighShelf[i].processSample( highShelf[i].processSample(lowShelf[i].processSample(dspDelayLines[i].popSample(channel))));
        
        out += cGains[i] * delayLineOutputMatrix->operator()(i, 0);
    }
    
    out += d * input;
    *delayLineInputMatrix = mixingMatrix->operator*(*delayLineOutputMatrix);

    
    return out;
    } else {
        return 0.f;
    }
}

void FDN::findNPrime(int LR, int UR, int N){
    int count = 0;
    bool prime;
    
    for(int i = LR; i <= UR; i++){
        if (count >= N)
            return;
        else {
            prime= true;
            for(int k = 2; k < i/2; k++) {
                if (i % k == 0) {
                    prime = false;
                    break;
                }
            }
            if (prime == true)
                delayLength[count++] = i;
         
        }
    }
}

float FDN::randomFloat(float min, float max) {
    float random = ((float) rand())/(float)RAND_MAX;
    float diff = max - min;
    float r = random * diff;
    return min + r;
}

void FDN::updateMatrixCoefficients(std::vector<float> newMatrixCoef, int matrixSelection) {
  
    if(matrixSelection == 1) {
        delete mixingMatrix;
        mixingMatrix = new dsp::Matrix<float>(dsp::Matrix<float>::identity(nrDelayLines));
    }
    
    if(matrixSelection == 2) {
        for (int i = 0; i < newMatrixCoef.size(); ++i) {
            matrixCoefficients.insert(i, newMatrixCoef[i]);
        }
        delete mixingMatrix;
        mixingMatrix = new dsp::Matrix<float>(nrDelayLines, nrDelayLines, matrixCoefficients.getRawDataPointer());
    }
}

void FDN::updateMatrixCoefficientsOSC(std::vector<float> newMatrixCoef, String singleWhole) {
    if(singleWhole.compare("whole") == 0) {
        for (int i = 0; i < newMatrixCoef.size(); ++i) {
            matrixCoefficients.insert(i, newMatrixCoef[i]);
        }
        mixingMatrix = new dsp::Matrix<float>(nrDelayLines, nrDelayLines, matrixCoefficients.getRawDataPointer());
    }
    
    if(singleWhole.compare("single") == 0) {
        for (int i = 0; i < newMatrixCoef.size(); ++i) {
            matrixCoefficients.insert(i, newMatrixCoef[i]);
        }
        delete mixingMatrix;
        mixingMatrix = new dsp::Matrix<float>(nrDelayLines, nrDelayLines, matrixCoefficients.getRawDataPointer());
    }
}


String FDN::getMatrixValues() {
    return mixingMatrix->toString();
}

String FDN::getBGains() {
    bGainsString = "";
    for (int i = 0; i < nrDelayLines; ++i) {
        bGainsString.append(std::to_string(bGains[i])).append(", ");
    }
    bGainsString.pop_back(); bGainsString.pop_back(); // remove last space and comma
    return bGainsString;
}

String FDN::getCGains() {
    cGainsString = "";
    for(int i = 0; i < nrDelayLines; ++i) {
        cGainsString.append(std::to_string(cGains[i])).append(", ");
    }
    cGainsString.pop_back(); cGainsString.pop_back(); // remove last space and comma
    return cGainsString;
}

String FDN::getDelayValues() {
    delayValuesString = "";
    for (int i = 0; i < nrDelayLines; ++i) {
        delayValuesString.append(std::to_string(delayLength[i])).append(", ");
    }
    delayValuesString.pop_back(); delayValuesString.pop_back(); // remove last space and comma
    return delayValuesString;
}
