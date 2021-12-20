/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FDNReverbAudioProcessorEditor::FDNReverbAudioProcessorEditor (FDNReverbAudioProcessor& p, AudioProcessorValueTreeState& apvts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(apvts)
{

    // ==== T60 LOW DECAY ====
    setSlider(lowT60Slider, linHor, 0.05f, 20.0f, 1.f, "sec.");
    lowT60Attach.reset(new SliderAttachment (valueTreeState, "T60LOW", lowT60Slider));
        
    // ==== T60 HIGH DECAY ====
    setSlider(highT60Slider, linHor, 0.05, 10.f, 0.5f, "sec.");
    highT60Attach.reset(new SliderAttachment (valueTreeState, "T60HIGH", highT60Slider));
        
    // ==== TRANSITION FREQUENCY ====
    setSlider(lowCutoffSlider, linHor, 200.f, 1000.f, 400.f, "Hz.");
    lowCutoffAttach.reset(new SliderAttachment (valueTreeState, "LOWTRANSFREQ", lowCutoffSlider));
        
    setSlider(highCutoffSlider, linHor, 2000.f, 12000.f, 2500.f, "Hz.");
    highCutoffAttach.reset(new SliderAttachment (valueTreeState, "HIGHTRANSFREQ", highCutoffSlider));
    
    // ==== OTHER SLIDERS ====
    // ==== DELAY LINE LENGTH  ====
    setSlider(delayLengthSlider, linHor, 10.f, 30.f, 15.f, "ms.");
    delayLengthAttach.reset(new SliderAttachment (valueTreeState, "DELLINELENGTH", delayLengthSlider));
    
    setLabel(delayLengthLabel, "Delay Line Length", smallFont, leftJust);
    delayLengthLabel.attachToComponent(&delayLengthSlider, true);
    
    // ==== DELAY MOD ====
    setSlider(modRateSlider, linHor, 0.f, 2.f, 0.5, "Hz.");
    modRateAttach.reset(new SliderAttachment(valueTreeState, "MODRATE", modRateSlider));
    setLabel(modRateLabel, "Mod. Rate", smallFont, leftJust);
    modRateLabel.attachToComponent(&modRateSlider, true);
    
    setSlider(modDepthSlider, linHor, 0.f, 10.f, 6.f, "");
    modDepthAttach.reset(new SliderAttachment(valueTreeState, "MODDEPTH", modDepthSlider));
    setLabel(modDepthLabel, "Mod. Depth", smallFont, leftJust);
    modDepthLabel.attachToComponent(&modDepthSlider, true);
    
    addAndMakeVisible(modulationButton);
    modulationButton.onClick = [this] {changeModulationState();};
    
    setLabel(modulationButtonLabel, "Modulate", smallFont, leftJust);
    modulationButtonLabel.attachToComponent(&modulationButton, true);
    
    // ==== DRY/WET ====
    setSlider(dryWetSlider, linHor, 0.f, 100.f, 50.f, "%");
    dryWetSliderAttach.reset(new SliderAttachment (valueTreeState, "DRYWET", dryWetSlider));
    
    setLabel(dryWetLabel, "Dry/Wet Mix", smallFont, leftJust);
    dryWetLabel.attachToComponent(&dryWetSlider, true);
    
    // ===== LABELS ====
    
    setLabel(lowT60Label, "Low T60", smallFont, leftJust);
    lowT60Label.attachToComponent(&lowT60Slider, true);
        
    setLabel(highT60Label, "High T60", smallFont, leftJust);
    highT60Label.attachToComponent(&highT60Slider, true);
        
    setLabel(lowCutoffabel, "Cutoff Freq.", smallFont, leftJust);
    lowCutoffabel.attachToComponent(&lowCutoffSlider, true);
       
    setLabel(highCutoffLabel, "Cutoff Freq.", smallFont, leftJust);
    highCutoffLabel.attachToComponent(&highCutoffSlider, true);
    
    // === DELAY LINE SELECTION
    addAndMakeVisible(fdnOrderComboBox);
    fdnOrderComboBox.addItem("4", 1);
    fdnOrderComboBox.addItem("8", 2);
    fdnOrderComboBox.addItem("16", 3);
    fdnOrderComboBox.addItem("32", 4);
    fdnOrderComboBox.setSelectedId(3);
    fdnOrderComboBox.addListener(this);
    setLabel(fdnOrderLabel, "FDN Order", mediumFont, centreJust);
    fdnOrderLabel.attachToComponent(&fdnOrderComboBox, true);

    // ==== MATRIX SELECTION ====
    addAndMakeVisible(matrixComboBox);
    matrixComboBox.addItem("Identity", 1);
    matrixComboBox.addItem("Custom", 2);
    matrixComboBox.setSelectedId(2);
    matrixComboBox.onChange = [this] {matrixBoxChanged(); };
    matrixAttach.reset(new ComboBoxAttachment(valueTreeState, "MATRIXSELECTION", matrixComboBox));
    
    setLabel(matrixLabel, "Matrix Selection", mediumFont, centreJust);
    matrixLabel.attachToComponent(&matrixComboBox, true);
    
    // === HEADER ====
    setLabel(titleLabel, "FDN Reverb", bigFont, centreJust);
    
    // === NR DELAY LINES ===
    setLabel(nrDelayLinesLabel, "Nr. Delay Lines: " + std::to_string(audioProcessor.getNrDelayLines()), smallFont, leftJust);

    // === OSC LABELS ===
    setLabel(oscStatusLabel, "OSC Status: ", smallFont, Justification::left);
    
    setLabel(portNumberLabel, "Port Number:", smallFont, leftJust);
    
    // === B GAINS LABEL ===
    setLabel(bGainsTitle, "B Gains: ", smallFont, Justification::left);

    setLabel(bGainsValues, "", valueTextFont, leftJust);

    // === C GAINS LABEL ===
    setLabel(cGainsTitle, "C Gains: ", smallFont, leftJust);

    setLabel(cGainsValues, "", valueTextFont, leftJust);
    
    // === DELAY VALUES LABEL ===
    setLabel(delaysTitle, "Delay Lengths: ", smallFont, leftJust);
    
    setLabel(delaysValues, "", valueTextFont, leftJust);
  
    // === DELAY TOGGLE BUTTON ===
    addAndMakeVisible(delayButton);
    delayButton.onClick = [this] {changeDelaySelection();};
    
    setLabel(delayButtonLabel, "Delays from OSC", mediumFont, leftJust);
    delayButtonLabel.attachToComponent(&delayButton, true);
    
    // === SHOW MATRIX ====
    addAndMakeVisible(matrixWindowButton);
    matrixWindowButton.onClick = [this] {matrixWindowButtonClicked();};

    setLabel(matrixWindowLabel, "Show Matrix Values", smallFont, leftJust);
    matrixWindowLabel.attachToComponent(&matrixWindowButton, true);
    
    setLabel(matrixValueLabel, "", smallFont, centreJust);
    matrixValueLabel.setVisible(false);

    // === WHITE BOX AROUND INFO AREA ===
    addAndMakeVisible(infoLabel);
    
    // === TEXT EDITOR FOR OSC PORT ===
    addAndMakeVisible(portEditor);
    portEditor.setText(std::to_string(audioProcessor.getPortNumber()));
    portEditor.setJustification(Justification::centred);
    
    setLabel(portEditorLabel, "UDP Port", mediumFont, leftJust);
    portEditorLabel.attachToComponent(&portEditor, true);
    
    // === UDP CONNECTOR BUTTON ===
    addAndMakeVisible(acceptUDPConnectionButton);
    acceptUDPConnectionButton.onClick = [this] {connectUDP();};
    
    setLabel(acceptUDPConnectionLabel, "Connect UDP", smallFont, leftJust);
    acceptUDPConnectionLabel.attachToComponent(&acceptUDPConnectionButton, true);
    
    // === SHOW INFO BUTTON ===
    addAndMakeVisible(showInfoButton);
    showInfoButton.onClick = [this] {showInfoButtonClicked();};
    
    setLabel(showInfoLabel, "Show Info", smallFont, leftJust);
    showInfoLabel.attachToComponent(&showInfoButton, true);

    
    // === RESIZE WINDOW BASED ON NR DELAY LINES ===
    if(audioProcessor.getNrDelayLines() > 16) {
        setSize(800, 400);
        setResizable(true,true);
        setResizeLimits(800, 400, 1200, 800);
    } else if (audioProcessor.getNrDelayLines() == 16) {
        setSize (630, 400);
        setResizable(true, true);
        setResizeLimits(630, 400, 1200, 800);
    } else {
        setSize(430, 400);
        setResizable(true, true);
        setResizeLimits(430, 400, 1200, 800);
    }
    
    showInfoButtonClicked(); // make sure info is hidden on startup, can be changed if needed
}

FDNReverbAudioProcessorEditor::~FDNReverbAudioProcessorEditor()
{
}

//==============================================================================
void FDNReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setFont(Font("Roboto", 20.0f, Font::bold));
    g.setColour(Colours::ghostwhite);
    g.setFont(Font("Helvetica", 10.0f, Font::plain));

    startTimerHz(8);
}

void FDNReverbAudioProcessorEditor::resized()
{
    auto infoArea = getLocalBounds();
    auto infoHeight = getHeight()/20;
    
    auto headerHeight = 36;
    auto headerY = 5;
    auto headerX = 5;
    auto headerWidth = 100;
    
    auto sliderLeft = 50;
    auto sliderRight = getWidth() - 60;
    auto sliderHeight = (getHeight()/2)/8;
    auto sliderWidth = getWidth() - sliderLeft;
    
    float infoButtonHeight;
    
    if(showInfoButton.getToggleState() == 1) {
        infoButtonHeight = getHeight() * 0.65;
    } else {
        infoButtonHeight = getHeight() - (infoHeight * 3);
    }

    // === Header Item Bounds ===
    titleLabel.setBounds(headerX, headerY, headerWidth, headerHeight);
    nrDelayLinesLabel.setBounds(100, headerY+2, headerWidth, headerHeight);
    
    portEditor.setBounds(sliderRight, headerY + 10, 50, 16);
    acceptUDPConnectionButton.setBounds(getWidth() * 0.75, headerY + 7, buttonSize, buttonSize);

    // === Sliders Bounds ===
    lowT60Slider.setBounds(sliderLeft, 30, sliderWidth, sliderHeight);
    lowCutoffSlider.setBounds(sliderLeft, lowT60Slider.getY() + sliderHeight, sliderWidth, sliderHeight);
    highT60Slider.setBounds(sliderLeft, lowCutoffSlider.getY() + sliderHeight, sliderWidth, sliderHeight);
    highCutoffSlider.setBounds(sliderLeft, highT60Slider.getY() + sliderHeight, sliderWidth, sliderHeight);
    
    delayLengthSlider.setBounds(sliderLeft, highCutoffSlider.getY() + sliderHeight, getWidth() * 0.6, sliderHeight);
    
    modRateSlider.setBounds(sliderLeft, delayLengthSlider.getY() + sliderHeight, getWidth()/2 - sliderLeft, sliderHeight);
    
    modDepthSlider.setBounds(getWidth()/2 + sliderLeft, delayLengthSlider.getY() + sliderHeight, getWidth()/2 - sliderLeft, sliderHeight);
    
    modulationButton.setBounds(sliderLeft, modRateSlider.getY() + sliderHeight, buttonSize, buttonSize);

    dryWetSlider.setBounds(sliderLeft, modulationButton.getY() + sliderHeight, getWidth() - sliderLeft, sliderHeight);
    // === ComboBox BBunds ===
    fdnOrderComboBox.setBounds(getWidth()/2, dryWetSlider.getY() + sliderHeight, 70, 20);
    matrixComboBox.setBounds(sliderRight - 50, dryWetSlider.getY() + sliderHeight, 80, 20);
    
    // === Button Bounds ===
    delayButton.setBounds(sliderRight, delayLengthSlider.getY(), buttonSize, buttonSize);
    matrixWindowButton.setBounds(sliderRight, infoButtonHeight, buttonSize, buttonSize);
    showInfoButton.setBounds(getWidth()/2, infoButtonHeight, buttonSize, buttonSize);
    
    // === Matrix Values Bounds ===
    matrixValueLabel.setBounds(0, sliderHeight, getWidth(), matrixWindowButton.getY() - sliderHeight);

    
    // === OSC Values ===
    portNumberLabel.setBounds(infoArea.removeFromBottom(infoHeight));
    oscStatusLabel.setBounds(getWidth()/2, portNumberLabel.getY(), getWidth()/2, infoHeight);

    bGainsTitle.setBounds(infoArea.removeFromBottom(infoHeight));
    bGainsValues.setBounds(40, bGainsTitle.getY(), getWidth() - 40, infoHeight);

    cGainsTitle.setBounds(infoArea.removeFromBottom(infoHeight));
    cGainsValues.setBounds(40, cGainsTitle.getY(), getWidth() - 40, infoHeight);
    
    delaysTitle.setBounds(infoArea.removeFromBottom(infoHeight));
    delaysValues.setBounds(70, delaysTitle.getY(), getWidth() - 60, infoHeight);

    
    // === Draw Lines around info section ===
    if(showInfoButton.getToggleState() == 1)
        drawPaths(getHeight() * 0.8);
    else
        drawPaths(getHeight() * 0.95);
}

void FDNReverbAudioProcessorEditor::timerCallback() {
    lowT60Slider.setValue(*valueTreeState.getRawParameterValue("T60LOW"));
    highT60Slider.setValue(*valueTreeState.getRawParameterValue("T60HIGH"));
    lowCutoffSlider.setValue(*valueTreeState.getRawParameterValue("LOWTRANSFREQ"));
    highCutoffSlider.setValue(*valueTreeState.getRawParameterValue("HIGHTRANSFREQ"));
    delayLengthSlider.setValue(*valueTreeState.getRawParameterValue("DELLINELENGTH"));
    modRateSlider.setValue(*valueTreeState.getRawParameterValue("MODRATE"));
    modDepthSlider.setValue(*valueTreeState.getRawParameterValue("MODDEPTH"));
    dryWetSlider.setValue(*valueTreeState.getRawParameterValue("DRYWET"));
    matrixComboBox.setSelectedId(*valueTreeState.getRawParameterValue("MATRIXSELECTION"));

    if (audioProcessor.modulateFDNBool) {
        modulationButton.setToggleState(true, false);
    } else {
        modulationButton.setToggleState(false, false);
    }
    
    oscStatusLabel.setText("OSC Updates: " + audioProcessor.getOSCMessageStatus(), dontSendNotification);
    
    nrDelayLinesLabel.setText("Nr. Delay Lines: " + std::to_string(audioProcessor.getNrDelayLines()), dontSendNotification);
    
    // == Values of B and C Gains and Delay line lenghts in sample
    bGainsValues.setText(audioProcessor.getBGains(), dontSendNotification);
    cGainsValues.setText(audioProcessor.getCGains(), dontSendNotification);
    delaysValues.setText(audioProcessor.getDelayValues(), dontSendNotification);

    portEditor.onReturnKey = [this]{updatePortNumber();};
    portNumberLabel.setText("UDP is " + audioProcessor.getOSCConnectionStatus() + " - Port Number: " + std::to_string(audioProcessor.getPortNumber()), dontSendNotification);

    if(audioProcessor.getNrDelayLines() >= 32) {
        matrixValueLabel.setFont(Font("Roboto", 10.f, Font::plain));
    } else if(audioProcessor.getNrDelayLines() >= 16 && audioProcessor.getNrDelayLines() < 32) {
        matrixValueLabel.setFont(Font("Roboto", 12.f, Font::plain));
    } else {
        matrixValueLabel.setFont(Font("Roboto", 12.f, Font::plain));
    }
    matrixValueLabel.setText(audioProcessor.getMatrixValues(), dontSendNotification);
}

void FDNReverbAudioProcessorEditor::sliderValueChanged(Slider *slider) {
    
    if (slider == &lowT60Slider) {
        *audioProcessor.t60LOW = lowT60Slider.getValue();
        audioProcessor.updateFilterBool = true;
        audioProcessor.updateProcessing = true;
    }
    
    if (slider == &highT60Slider) {
        *audioProcessor.t60HIGH = highT60Slider.getValue();
        audioProcessor.updateFilterBool = true;
        audioProcessor.updateProcessing = true;
    }
    
    if (slider == &lowCutoffSlider) {
        *audioProcessor.transFREQLow = lowCutoffSlider.getValue();
        audioProcessor.updateFilterBool = true;
        audioProcessor.updateProcessing = true;
    }
    
    if (slider == &highCutoffSlider) {
        *audioProcessor.transFREQHigh = highCutoffSlider.getValue();
        audioProcessor.updateFilterBool = true;
        audioProcessor.updateProcessing = true;
    }
    
    if (slider == &delayLengthSlider) {
        *audioProcessor.delLineLength = delayLengthSlider.getValue();
        audioProcessor.updateDelayBool = true;
        audioProcessor.updateProcessing = true;
    }
    
    if (slider == & dryWetSlider) {
        *audioProcessor.wet = dryWetSlider.getValue();
        audioProcessor.updateDryWetBool = true;
        audioProcessor.updateProcessing = true;
    }
}

void FDNReverbAudioProcessorEditor::comboBoxChanged(ComboBox *comboBox) {
    
    if(comboBox == &matrixComboBox) {
        switch(matrixComboBox.getSelectedId()) {
            case 1: *audioProcessor.matrixSelec = 1; break;
            case 2: *audioProcessor.matrixSelec = 2; break;
                
            default:
                break;
        }
    }
    
    if(comboBox == &fdnOrderComboBox) {
        switch(fdnOrderComboBox.getSelectedId()) {
            case 1: audioProcessor.setNrDelayLines(4); break;
            case 2: audioProcessor.setNrDelayLines(8); break;
            case 3: audioProcessor.setNrDelayLines(16); break;
            case 4: audioProcessor.setNrDelayLines(32); break;
                
            default:
                break;
        }
    }
    audioProcessor.updateProcessing = true;
}

void FDNReverbAudioProcessorEditor::matrixBoxChanged() {
    if(matrixComboBox.getSelectedId() == 2) {
        audioProcessor.updateMatrixOSCBool = true;
    } else {
        audioProcessor.updateMatrixOSCBool = false;
    }
    audioProcessor.updateMatrixBool = true;
    audioProcessor.updateProcessing = true;
}

void FDNReverbAudioProcessorEditor::drawPaths(float yValue) {
    infoLabel.setBounds(-5, yValue, getWidth() + 30, getHeight() * 0.7 + 10);
    infoLabel.setColour(Label::ColourIds::outlineColourId, Colours::ghostwhite);
}

void FDNReverbAudioProcessorEditor::changeDelaySelection() {
    if(delayButton.getToggleState() == 1) {
        delayLengthSlider.setEnabled(false);
        audioProcessor.updateDelayOSCBool = true;
    } else {
        delayLengthSlider.setEnabled(true);
        audioProcessor.updateDelayOSCBool = false;
    }
}

void FDNReverbAudioProcessorEditor::matrixWindowButtonClicked() {
    if(matrixWindowButton.getToggleState() == 1) {

        lowT60Slider.setVisible(false);
        highT60Slider.setVisible(false);
        lowCutoffSlider.setVisible(false);
        highCutoffSlider.setVisible(false);
        dryWetSlider.setVisible(false);
        delayLengthSlider.setVisible(false);
        modRateSlider.setVisible(false);
        modDepthSlider.setVisible(false);
        modulationButton.setVisible(false);
        matrixComboBox.setVisible(false);
        delayButton.setVisible(false);
        fdnOrderComboBox.setVisible(false);
        
        matrixValueLabel.setVisible(true);
    } else {
        matrixValueLabel.setVisible(false);
        lowT60Slider.setVisible(true);
        highT60Slider.setVisible(true);
        lowCutoffSlider.setVisible(true);
        highCutoffSlider.setVisible(true);
        dryWetSlider.setVisible(true);
        delayLengthSlider.setVisible(true);
        modRateSlider.setVisible(true);
        modDepthSlider.setVisible(true);
        modulationButton.setVisible(true);
        matrixComboBox.setVisible(true);
        delayButton.setVisible(true);
        fdnOrderComboBox.setVisible(true);
    }
}

void FDNReverbAudioProcessorEditor::showInfoButtonClicked() {
    
    resized();
    
    if(showInfoButton.getToggleState() == 1) {
        delaysTitle.setVisible(true);
        delaysValues.setVisible(true);
        bGainsTitle.setVisible(true);
        bGainsValues.setVisible(true);
        cGainsTitle.setVisible(true);
        cGainsValues.setVisible(true);
    } else {
        delaysTitle.setVisible(false);
        delaysValues.setVisible(false);
        bGainsTitle.setVisible(false);
        bGainsValues.setVisible(false);
        cGainsTitle.setVisible(false);
        cGainsValues.setVisible(false);
    }
}

void FDNReverbAudioProcessorEditor::updatePortNumber() {
    int portNumber = portEditor.getText().getIntValue();
    audioProcessor.updateOSCPort(portNumber);
}

void FDNReverbAudioProcessorEditor::connectUDP() {
    if (acceptUDPConnectionButton.getToggleState() == 1) {
        audioProcessor.connectUDP(true);
    } else {
        audioProcessor.connectUDP(false);
    }
}

void FDNReverbAudioProcessorEditor::changeModulationState() {
    if (modulationButton.getToggleState() == 1) {
        audioProcessor.modulateFDN(true);
    } else {
        audioProcessor.modulateFDN(false);
    }
}

// one line function to write out the labels
void FDNReverbAudioProcessorEditor::setLabel(Label& label, const String& name, const Font& font, const Justification& justific) {
    label.setText(name, dontSendNotification);
    label.setFont(font);
    label.setJustificationType(justific);
    addAndMakeVisible(label);
}

// one line function to write out the sliders
void FDNReverbAudioProcessorEditor::setSlider(Slider& slider, Slider::SliderStyle& style ,float min, float max, float def, const String& suffix) {
    addAndMakeVisible(&slider);
    slider.setSliderStyle(style);
    slider.setRange(min, max);
    slider.setValue(def);
    slider.setTextValueSuffix(suffix);
    slider.setTextBoxStyle(Slider::TextBoxLeft, false, textBoxWidth, textBoxHeight);
    slider.addListener(this);
}
