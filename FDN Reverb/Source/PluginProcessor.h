/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FDN.hpp"

using namespace dsp;

//==============================================================================
/**
*/
class FDNReverbAudioProcessor  : public juce::AudioProcessor,
                                          public ValueTree::Listener,
                                          public OSCReceiver,
                                          public OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>,
                                          public OSCSender
{
public:
    //==============================================================================
    FDNReverbAudioProcessor();
    ~FDNReverbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    enum {
        highestFDNOrder = 32,
    };
    
    
    void numChannelsChanged() override;
    void updateOSCPort(int newPortNumber);
    


    void updateParameters();
    
    int getNrDelayLines();
    
    void setNrDelayLines(int newDelayNr);

    void oscMessageReceived(const OSCMessage &message) override;
    
    void setPortNumber(int newPortNumber);
    int getPortNumber();
    void connectUDP(bool connected);
    
    String getOSCConnectionStatus();
    String getOSCMessageStatus();
    String getMatrixValues();
    
    std::string oscConnectionStatus = "Not Connected";
    std::string oscMessageStatus = "";
    std::string matrixValues = "";
    
    String getBGains();
    String getCGains();
    String getDelayValues();

    AudioProcessorValueTreeState tree;
     
    // Audio Plugin Parameters
    std::atomic<float>* t60LOW;
    std::atomic<float>* t60HIGH;
    std::atomic<float>* transFREQLow;
    std::atomic<float>* transFREQHigh;
    std::atomic<float>* delLineLength;
    std::atomic<float>* wet;
    std::atomic<float>* matrixSelec;
    std::atomic<float>* modRate;
    std::atomic<float>* modDepth;
    
    // not used at the moment, could be used for delay line smoothing
    SmoothedValue<float, ValueSmoothingTypes::Linear> smoother;

    
    float Fs;
    int nrOutputs = 2;
    
    float lowDel = 5.f; // ms
    float highDel = 20.f; // ms
    
    bool updateProcessing;
    bool updateDryWetBool = false;
    bool updateFilterBool = false;
    bool updateDelayBool = false;
    bool updateMatrixBool = true;
    bool updateMatrixOSCBool = false;
    bool updateDelayOSCBool = false;
    

private:

    bool isActive = false;
    bool changingFDNOrder = false;
    
    void valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override;
    int nrDelayLines;
    int maxDelaySamples = 176400;
    int maxNumChannels;

    std::vector<float> bGains;
    std::vector<float> cGains;
    
    std::vector<float> matrixCoefs;
    std::vector<float> delayVector;
    
    // OSC variables
    OSCReceiver oscReceiver;
    int portNumber;
    int count = 0;
        
    // Coupled rooms variables
    float coupling = 0.3f;
    
//    Initialise FDN
    FDN fdn;
  


    //==============================================================================
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FDNReverbAudioProcessor)
};
