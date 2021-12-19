/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <string>
//==============================================================================
/**
*/
class FDNReverbAudioProcessorEditor  : public juce::AudioProcessorEditor, public Slider::Listener, private Timer, public ComboBox::Listener, public Button::Listener, public TextEditor::Listener
{
public:
    FDNReverbAudioProcessorEditor (FDNReverbAudioProcessor&, AudioProcessorValueTreeState&);
    ~FDNReverbAudioProcessorEditor() override;

    
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
    
    
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void sliderValueChanged(Slider *slider) override;
    void nrOfRoomsChanged();
    void comboBoxChanged(ComboBox *comboBox) override;
    void buttonClicked(Button *button) override{};
    void changeDelaySelection();
    void matrixWindowButtonClicked();
    void showInfoButtonClicked();
    void connectUDP();
    
    void matrixBoxChanged();
    void updatePortNumber();
    
    void setLabel(Label& label, const String& name, const Font& font, const Justification& justific);
    
    void setSlider(Slider& slider, Slider::SliderStyle& style ,float min, float max, float def, const String& suffix);
    
    void drawPaths(float yValue);

private:
    FDNReverbAudioProcessor& audioProcessor;
    AudioProcessorValueTreeState& valueTreeState;
    
    
    Label lowT60Label, highT60Label, lowCutoffabel, highCutoffLabel, dryWetLabel, delayLengthLabel, modDepthLabel, modRateLabel, matrixLabel, fdnOrderLabel, matrixValueLabel, infoLabel, delayButtonLabel, matrixWindowLabel, showInfoLabel, acceptUDPConnectionLabel;
    
    Slider lowT60Slider, highT60Slider, lowCutoffSlider, highCutoffSlider, dryWetSlider, delayLengthSlider, modDepthSlider, modRateSlider;

    std::unique_ptr<SliderAttachment> lowT60Attach, highT60Attach, lowCutoffAttach, highCutoffAttach, dryWetSliderAttach, delayLengthAttach, modDepthAttach, modRateAttach;
    
    ComboBox fdnOrderComboBox, matrixComboBox;
    
    std::unique_ptr<ComboBoxAttachment> nrOfRoomsAttach, matrixAttach;
    
    ToggleButton delayButton, matrixWindowButton, showInfoButton, acceptUDPConnectionButton;
    
    // === Layout Labels === //
    Label titleLabel, nrDelayLinesLabel, oscStatusLabel, portNumberLabel, portEditorLabel, bGainsTitle, bGainsValues, cGainsTitle, cGainsValues, delaysTitle, delaysValues;
    
    int textBoxWidth = 50;
    int textBoxHeight = 15;
    float valueTextSize = 9.f;
    float buttonSize = 20.f;
    
    Font valueTextFont = Font("Arial", 9.f, Font::plain);
    Font smallFont = Font("Arial", 10.f, Font::plain);
    Font mediumFont = Font("Arial", 15.f, Font::plain);
    Font bigFont = Font("Arial", 17.f, Font::plain);
    
    Justification leftJust = Justification::left;
    Justification centreJust = Justification::centred;
    
    Slider::SliderStyle linHor = Slider::SliderStyle::LinearHorizontal;
    
    TextEditor portEditor;

    bool updateMatrixBool = true;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FDNReverbAudioProcessorEditor)
};
