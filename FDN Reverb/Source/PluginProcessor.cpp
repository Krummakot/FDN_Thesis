/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FDNReverbAudioProcessor::FDNReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), tree(*this, nullptr, Identifier("FDNReverb"), {
         std::make_unique<AudioParameterFloat>
         ("T60LOW",
          "Low T60",
          0.05f,
          20.0f,
          1.f),
         std::make_unique<AudioParameterFloat>
         ("T60HIGH",
          "High T60",
          0.05f,
          10.f,
          0.5f),
         std::make_unique<AudioParameterFloat>
         ("LOWTRANSFREQ",
          "Transitional Freq.",
          200.0f,
          1000.0f,
          400.0f),
         std::make_unique<AudioParameterFloat>
         ("HIGHTRANSFREQ",
          "Transitional Freq.",
          2000.0f,
          12000.f,
          2500.f),
         std::make_unique<AudioParameterFloat>
         ("MODRATE",
          "Mod. Rate",
          0.f,
          2.0f,
          0.5f),
         std::make_unique<AudioParameterFloat>
         ("MODDEPTH",
          "Mod. Depth",
          0.f,
          10.0f,
          6.f),
         std::make_unique<AudioParameterFloat>
         ("DELLINELENGTH",
          "Delay Line Length",
          10.f,
          30.0f,
          15.f),
         std::make_unique<AudioParameterFloat>
         ("DRYWET",
          "Dry/Wet",
          0.0f,
          100.f,
          50.f),
         std::make_unique<AudioParameterInt>
         ("MATRIXSELECTION",
          "Matrix Selection",
          1,
          2,
          2)
     })
#endif
{
    t60LOW = tree.getRawParameterValue("T60LOW");
    t60HIGH = tree.getRawParameterValue("T60HIGH");
    transFREQLow = tree.getRawParameterValue("LOWTRANSFREQ");
    transFREQHigh = tree.getRawParameterValue("HIGHTRANSFREQ");
    delLineLength = tree.getRawParameterValue("DELLINELENGTH");
    modRate = tree.getRawParameterValue("MODRATE");
    modDepth = tree.getRawParameterValue("MODDEPTH");
    wet = tree.getRawParameterValue("DRYWET");
    matrixSelec = tree.getRawParameterValue("MATRIXSELECTION");
}

FDNReverbAudioProcessor::~FDNReverbAudioProcessor()
{
}

//==============================================================================
const juce::String FDNReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FDNReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FDNReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FDNReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FDNReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FDNReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FDNReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FDNReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FDNReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void FDNReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FDNReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    Fs = getSampleRate();
    nrDelayLines = highestFDNOrder;

    fdn.init(Fs, nrDelayLines, lowDel, highDel);
    
    smoother.reset(Fs, 1.f);
    
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    fdn.reset();
    fdn.prepare(spec);
    
    bGains.resize(nrDelayLines);
    cGains.resize(nrDelayLines);
    matrixCoefs.resize(nrDelayLines * nrDelayLines);
    delayVector.resize(nrDelayLines);

    // OSC paramters
    portNumber = 6448;
    
    updateParameters();
    isActive = true;
}

void FDNReverbAudioProcessor::releaseResources()
{
    fdn.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FDNReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif



void FDNReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if(!isActive)
        return;
    
    if (updateProcessing) {
        updateParameters();
    }
    
    fdn.updateModulation(modDepth->load(), modRate->load());

        auto* left = buffer.getWritePointer(0);
        auto* right = buffer.getWritePointer(1);
        const int numSamples = buffer.getNumSamples();

        smoother.setTargetValue(delLineLength->load());
        for (int n = 0; n < numSamples; ++n) {
            float dryLeft = left[n];
            float dryRight = right[n];

            float leftPath = fdn.processFDN(0, left[n]);
            float rightPath = fdn.processFDN(1, right[n]);

            left[n] = 0.6 * ((1.0f - (wet->load()/100)) * dryLeft + (wet->load()/100) * leftPath);
            right[n] = 0.6 * ((1.0f - (wet->load()/100)) * dryRight + (wet->load()/100) * rightPath);
        }
    if (changingFDNOrder) {
        fdn.updateFDN(nrDelayLines, matrixSelec->load());
        changingFDNOrder = !changingFDNOrder;
    }
}

//==============================================================================
bool FDNReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FDNReverbAudioProcessor::createEditor()
{
    return new FDNReverbAudioProcessorEditor(*this, tree);
//    return new GenericAudioProcessorEditor(*this);
}

//==============================================================================
void FDNReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    ValueTree state = tree.copyState();
    std::unique_ptr<XmlElement> xml = state.createXml();
    copyXmlToBinary(*xml.get(), destData);
}

void FDNReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState = getXmlFromBinary(data, sizeInBytes);
    ValueTree state = ValueTree::fromXml(*xmlState.get());
    tree.replaceState(state);
}

void FDNReverbAudioProcessor::numChannelsChanged() {

}

void FDNReverbAudioProcessor::updateParameters() {

    updateProcessing = false;
    
    float g_DC = t60LOW->load();
    float g_PI = t60HIGH->load();
    float l_fT = transFREQLow->load();
    float h_fT = transFREQHigh->load();
    float delLength = delLineLength->load();
    float dryWet = wet->load();
    int newMatrixValue = matrixSelec->load();
    
    float m_Rate = modRate->load();
    float m_Depth = modDepth->load();

    // make sure only needed variables are updated
        if(updateFilterBool) {
            fdn.updateFilter(g_DC, g_PI, l_fT, h_fT);
            updateFilterBool = false;
        }
        if(updateDelayBool) {
            fdn.updateDelay(delLength);
            fdn.updateFilter(g_DC, g_PI, l_fT, h_fT);
            updateDelayBool = false;
        }

        if(updateDryWetBool) {
            fdn.updateDryMix(dryWet/100.f);
            updateDryWetBool = false;
        }
    
        if(updateMatrixBool) {
            fdn.updateMatrixCoefficients(matrixCoefs, newMatrixValue);
            updateMatrixBool = false;
        }
    
    fdn.updateModulation(m_Depth, m_Rate);
    }

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FDNReverbAudioProcessor();
}


void FDNReverbAudioProcessor::valueTreePropertyChanged(ValueTree &treeWhosePropertyChanged, const Identifier &property) {
     updateProcessing = true;
}


int FDNReverbAudioProcessor::getNrDelayLines() {
    return nrDelayLines;
}

void FDNReverbAudioProcessor::oscMessageReceived(const OSCMessage &message) {
    
    // A check has to be made for it to work
    if (message[0].isString()) {
        String messageString = message[0].getString(); // get the string argument
        
        if(messageString.compare("connected") == 0) {
            oscConnectionStatus = "Connected";
        }
        // === Gain updates ===
        // Whole gain updates
        if(messageString.compare("bGainWhole") == 0) {
            oscMessageStatus = "Updating B Gains...";

            if(message[1].isFloat32()) {
                bGains[count] = message[1].getFloat32();
                count++;
                if(count >= nrDelayLines) {
                    fdn.setBGains(bGains);
                    oscMessageStatus = "B Gains Updated";
                    count = 0;
                }
            }
        }
        if(messageString.compare("cGainWhole") == 0) {
            oscMessageStatus = "Updating C Gains...";
            
            if (message[1].isFloat32()) {
                cGains[count] = message[1].getFloat32();
                count++;
                if(count >= nrDelayLines) {
                    fdn.setCGains(cGains);
                    oscMessageStatus = "C Gains Updated";
                    count = 0;
                }
            }
        }
        // Single gain value updates
        if(messageString.compare("bGainSingle") == 0) {
            if (message[1].isInt32() && message[2].isFloat32()) {
                fdn.setSingleBGain(message[1].getInt32(), message[2].getFloat32());
                oscMessageStatus = "B Gain Index [" + std::to_string(message[1].getInt32()) + "] Updated";
            }
        }
        
        if(messageString.compare("cGainSingle") == 0) {
            if (message[1].isInt32() && message[2].isFloat32()) {
                fdn.setSingleCGain(message[1].getInt32(), message[2].getFloat32());
                oscMessageStatus = "C Gain Index [" + std::to_string(message[1].getInt32()) + "] Updated";
            }
        }
        // ======================================
        // === Matrix Updates ====
        if(messageString.compare("matrixWhole") == 0) {
            updateMatrixBool = true;
            if(updateMatrixOSCBool) {
                oscMessageStatus = "Updating Matrix Coefficients...";
                if (message[1].isFloat32()) {
                    matrixCoefs[count] = message[1].getFloat32();
                    count++;
                    if (count >= nrDelayLines * nrDelayLines) {
                        fdn.updateMatrixCoefficientsOSC(matrixCoefs, "whole");
                        oscMessageStatus = "Matrix Updated";
                        count = 0;
                        updateMatrixBool = false;
                    }
                }
            }
        }
        
        if(messageString.compare("matrixSingle") == 0) {
            updateMatrixBool = true;
            if (updateMatrixOSCBool) {
                oscMessageStatus = "Updating Matrix Coefficients...";
                if(message[1].isInt32() && message[2].isInt32() && message[3].isFloat32()) {
                    int row = message[1].getInt32();
                    int col = message[2].getInt32();
                    float val = message[3].getFloat32();
                    matrixCoefs[row * nrDelayLines + col] = val;
                    fdn.updateMatrixCoefficientsOSC(matrixCoefs, "single");
                    oscMessageStatus = "Matrix Index [" + std::to_string(row) + "," + std::to_string(col) + "] Updated";
                }
            }
        }
        // ======================================
        // === Delay Updates ===
        if(messageString.compare("delayWhole") == 0) {
            if(updateDelayOSCBool) {
                oscMessageStatus = "Updating Delay Lengths...";
                if (message[1].isFloat32()) {
                    delayVector[count] = message[1].getFloat32();
                    count++;
                    if (count >= nrDelayLines) {
                        delLineLength->operator=(*std::max_element(delayVector.begin(), delayVector.end()));
                        fdn.setDelayOSCWhole(delayVector);
                        oscMessageStatus = "Delays Updated";
                        count = 0;
                    }
                }
            }
        }
        
        if(messageString.compare("delaySingle") == 0) {
            if (updateDelayOSCBool) {
                oscMessageStatus = "Updating Delay Lengths...";
                
                if (message[1].isInt32() && message[2].isInt32()) {
                    delayVector[message[1].getInt32()] = message[2].getInt32();
                    delLineLength->operator=(*std::max_element(delayVector.begin(), delayVector.end()));
                    fdn.setDelayOSCSingle(message[1].getInt32(), message[2].getInt32());
                    oscMessageStatus = "Delay Line Index [" + std::to_string(message[1].getInt32()) + "] Updated";
                }
            }
        }
        // ======================================
        // === UPDATE DRY WET ===
        if(messageString.compare("dryWet") == 0) {
            oscMessageStatus = "Updating Dry/Wet Value...";
            if (message[1].isFloat32()) {
                wet->operator=(message[1].getFloat32());
                fdn.updateDryMix(message[1].getFloat32()/100.f);
                oscMessageStatus = "Dry/Wet Value updated";
            }
        }
        
        // ======================================
        // === UPDATE DECAY AND TRANSITION FREQ ===
        if(messageString.compare("highT60") == 0) {
            oscMessageStatus = "Updating High T60 Value...";
            
            if (message[1].isFloat32()) {
                t60HIGH->operator=(message[1].getFloat32());
                fdn.updateFilter(t60LOW->load(), t60HIGH->load(), transFREQLow->load(), transFREQHigh->load());
                oscMessageStatus = "High T60 Updated";
            }
        }
        
        if(messageString.compare("lowT60") == 0) {
            oscMessageStatus = "Updating Low T60 Value...";
            
            if (message[1].isFloat32()) {
                t60LOW->operator=(message[1].getFloat32());
                fdn.updateFilter(t60LOW->load(), t60HIGH->load(), transFREQLow->load(), transFREQHigh->load());
                oscMessageStatus = "Low T60 Updated";
            }
        }
        
        if(messageString.compare("transFreq") == 0) {
            oscMessageStatus = "Updating Transitional Frequency...";
            
            if (message[1].isFloat32()) {
                transFREQLow->operator=(message[1].getFloat32());
                fdn.updateFilter(t60LOW->load(), t60HIGH->load(), transFREQLow->load(), transFREQHigh->load());
                oscMessageStatus = "Transitional Frequency Updated";
            }
        }
    }

}

void FDNReverbAudioProcessor::connectUDP(bool isConnected) {
    if (isConnected) {
        oscReceiver.disconnect();
        oscReceiver.connect(portNumber);
        oscReceiver.addListener(this, "/juce");
        oscConnectionStatus = "Connected";
    } else {
        oscReceiver.disconnect();
        oscConnectionStatus = "Not Connected";
    }
}

void FDNReverbAudioProcessor::updateOSCPort(int newPortNumber) {
    oscReceiver.disconnect();
    portNumber = newPortNumber;
    oscReceiver.connect(portNumber);
    oscReceiver.addListener(this, "/juce");
}

int FDNReverbAudioProcessor::getPortNumber() {
    return portNumber;
}

void FDNReverbAudioProcessor::setNrDelayLines(int newDelayNr) {
    nrDelayLines = newDelayNr;
    bGains.resize(nrDelayLines);
    cGains.resize(nrDelayLines);
    matrixCoefs.resize(nrDelayLines*nrDelayLines);
    delayVector.reserve(nrDelayLines);
    changingFDNOrder = true;
    
}

String FDNReverbAudioProcessor::getOSCConnectionStatus() {
    return oscConnectionStatus;
}

String FDNReverbAudioProcessor::getOSCMessageStatus() {
    return oscMessageStatus;
}

String FDNReverbAudioProcessor::getMatrixValues() {
    return fdn.getMatrixValues();
}

String FDNReverbAudioProcessor::getBGains() {
    return fdn.getBGains();
}

String FDNReverbAudioProcessor::getCGains() {
    return fdn.getCGains();
}

String FDNReverbAudioProcessor::getDelayValues() {
    return fdn.getDelayValues();
}



