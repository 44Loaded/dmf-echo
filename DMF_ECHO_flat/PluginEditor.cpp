#include "PluginEditor.h"

namespace {
const juce::Colour bg (0xff08070b), panel (0xff100d14), violet (0xffb34cff), violet2 (0xff7130b5), pale (0xffd9b6ef), muted (0xffa18ba9);
void styleCombo (juce::ComboBox& c) { c.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff100b15)); c.setColour (juce::ComboBox::textColourId, pale); c.setColour (juce::ComboBox::outlineColourId, violet2); c.setColour (juce::ComboBox::arrowColourId, violet); }
}

void DMFEchoAudioProcessorEditor::NeonLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&)
{
    const float size = (float) juce::jmin (w, h) * 0.78f;
    const float cx = x + w * 0.5f, cy = y + h * 0.48f, r = size * 0.5f;
    const float angle = start + pos * (end - start);
    g.setColour (juce::Colour (0xff050408)); g.fillEllipse (cx-r-5,cy-r-5,(r+5)*2,(r+5)*2);
    g.setColour (violet.withAlpha (0.10f)); g.drawEllipse (cx-r-7,cy-r-7,(r+7)*2,(r+7)*2,2.0f);
    juce::Path track; track.addCentredArc (cx,cy,r-3,r-3,0,start,end,true);
    g.setColour (juce::Colour (0xff392544)); g.strokePath (track, juce::PathStrokeType (5.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    juce::Path value; value.addCentredArc (cx,cy,r-3,r-3,0,start,angle,true);
    g.setColour (violet); g.strokePath (value, juce::PathStrokeType (5.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour (juce::Colour (0xff1a1420)); g.fillEllipse (cx-r+9,cy-r+9,(r-9)*2,(r-9)*2);
    g.setColour (juce::Colour (0xff493653)); g.drawEllipse (cx-r+9,cy-r+9,(r-9)*2,(r-9)*2,1.0f);
    juce::Path pointer; pointer.addRoundedRectangle (-1.5f,-r+13,3.0f,15.0f,1.5f);
    g.setColour (pale); g.fillPath (pointer,juce::AffineTransform::rotation (angle).translated (cx,cy));
    g.setColour (violet.withAlpha (0.22f)); g.fillEllipse (cx-5,cy-5,10,10);
}

DMFEchoAudioProcessorEditor::DMFEchoAudioProcessorEditor (DMFEchoAudioProcessor& p)
 : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&look);
    auto& state = processor.parameters;
    logo.setText ("DMF ECHO", juce::dontSendNotification); logo.setFont (juce::Font (juce::FontOptions (34.0f, juce::Font::bold))); logo.setColour (juce::Label::textColourId, violet); addAndMakeVisible (logo);
    brand.setText ("D I G I T A L\nM A F I A", juce::dontSendNotification); brand.setFont (juce::Font (juce::FontOptions (11.0f))); brand.setColour (juce::Label::textColourId, pale); addAndMakeVisible (brand);
    title.setText ("M U S I C A L   D E L A Y", juce::dontSendNotification); title.setFont (juce::Font (juce::FontOptions (12.0f))); title.setColour (juce::Label::textColourId, muted); title.setJustificationType (juce::Justification::centred); addAndMakeVisible (title);
    footerBrand.setText ("D I G I T A L   M A F I A", juce::dontSendNotification); footerBrand.setFont (juce::Font (juce::FontOptions (10.0f))); footerBrand.setColour (juce::Label::textColourId, muted); addAndMakeVisible (footerBrand);
    preset.addItemList ({"DREAMY VOCALS","CRYSTAL ECHO","DARK MELODY","LIQUID ARP","FLOATING NOTES","UNDERGROUND","REVERSE DREAMS","MINIMAL ECHO"}, 1);
    preset.setSelectedId (1); preset.setColour (juce::ComboBox::backgroundColourId, panel); preset.setColour (juce::ComboBox::textColourId, pale); preset.setColour (juce::ComboBox::outlineColourId, violet2); addAndMakeVisible (preset);
    preset.onChange = [this] { processor.setCurrentProgram (preset.getSelectedId() - 1); };
    saveButton.setColour (juce::TextButton::buttonColourId, panel); saveButton.setColour (juce::TextButton::textColourOffId, violet); addAndMakeVisible (saveButton);
    saveButton.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser> ("Save DMF ECHO preset", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.dmfecho");
        fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting, [this] (const juce::FileChooser& fc) {
            auto f = fc.getResult(); if (f != juce::File()) { auto target = f.hasFileExtension ("dmfecho") ? f : f.withFileExtension ("dmfecho");
                const bool ok = processor.savePresetToFile (target); juce::AlertWindow::showMessageBoxAsync (ok ? juce::AlertWindow::InfoIcon : juce::AlertWindow::WarningIcon, "DMF ECHO", ok ? "Preset saved." : "Could not save preset."); }
        });
    };
    settingsButton.setColour (juce::TextButton::buttonColourId, panel); settingsButton.setColour (juce::TextButton::textColourOffId, pale); addAndMakeVisible (settingsButton);
    settingsButton.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser> ("Load DMF ECHO preset", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.dmfecho");
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc) {
            auto f = fc.getResult(); if (f != juce::File()) { const bool ok = processor.loadPresetFromFile (f); juce::AlertWindow::showMessageBoxAsync (ok ? juce::AlertWindow::InfoIcon : juce::AlertWindow::WarningIcon, "DMF ECHO", ok ? "Preset loaded." : "Could not load preset."); }
        });
    };
    sync.setColour (juce::ToggleButton::textColourId, pale); syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "sync", sync); addAndMakeVisible (sync);
    chordMode.setColour (juce::ToggleButton::textColourId, pale); chordAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "chordOn", chordMode); addAndMakeVisible (chordMode);
    division.addItemList ({"1/1","1/2","1/4","1/8","1/16","1/1 D","1/2 D","1/4 D","1/8 D","1/16 D","1/1 T","1/2 T","1/4 T","1/8 T"}, 1); styleCombo (division);
    divisionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "division", division); addAndMakeVisible (division);
    // Menu attachments
    auto makeMenu = [&state] (Menu& m, const char* id, juce::StringArray items) {
        m.box = std::make_unique<juce::ComboBox>(); m.box->addItemList (items, 1); styleCombo (*m.box);
        m.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, id, *m.box);
    };
    makeMenu (key, "key", {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"});
    makeMenu (scale, "scale", {"Major","Natural Minor","Harmonic Minor","Melodic Minor","Dorian","Phrygian","Lydian","Mixolydian","Locrian","Major Pentatonic","Minor Pentatonic"});
    makeMenu (motion, "motion", {"STATIC","UP","DOWN","WAVE","RANDOM"});
    makeMenu (chord, "chord", {"Major","Minor","Diminished","Augmented","Sus2","Sus4","Major 7","Minor 7","Dominant 7"});
    for (auto* m : {&key,&scale,&motion,&chord}) addAndMakeVisible (*m->box);
    const char* ids[] = {"delay","feedback","mix","mutate","space","dirt"};
    const char* names[] = {"DELAY","FEEDBACK","MIX","MUTATE","SPACE","DIRT"};
    for (int i=0;i<6;++i) {
        auto& k = knobs[i]; k.slider = std::make_unique<juce::Slider>(); k.slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag); k.slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0); k.slider->setLookAndFeel (&look);
        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, ids[i], *k.slider); addAndMakeVisible (*k.slider);
        k.label = std::make_unique<juce::Label>(); k.label->setText (names[i], juce::dontSendNotification); k.label->setJustificationType (juce::Justification::centred); k.label->setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold))); k.label->setColour (juce::Label::textColourId, pale); addAndMakeVisible (*k.label);
        k.value = std::make_unique<juce::Label>(); k.value->setJustificationType (juce::Justification::centred); k.value->setColour (juce::Label::textColourId, muted); addAndMakeVisible (*k.value);
        k.slider->onValueChange = [this,i] { auto& a=knobs[i]; const auto v=a.slider->getValue(); juce::String s;
            if (i==0) s=juce::String ((int) v)+" ms"; else s=juce::String ((int) std::round (v*100))+"%"; a.value->setText (s, juce::dontSendNotification); }; k.slider->onValueChange();
    }
    setResizable (true, true); setResizeLimits (900, 506, 1600, 900);
    setSize (1200, 675);
    startTimerHz (30);
}
DMFEchoAudioProcessorEditor::~DMFEchoAudioProcessorEditor() { for (auto& k : knobs) if (k.slider) k.slider->setLookAndFeel (nullptr); setLookAndFeel (nullptr); }

void DMFEchoAudioProcessorEditor::drawVisualizer (juce::Graphics& g, juce::Rectangle<float> r)
{
    auto c = r.getCentre();
    const float radius = juce::jmin (r.getWidth(), r.getHeight()) * (0.29f + processor.parameters.getRawParameterValue ("mix")->load() * 0.07f);
    const float mutation = processor.parameters.getRawParameterValue ("mutate")->load();
    const float width = processor.parameters.getRawParameterValue ("space")->load();
    const float feedback = processor.parameters.getRawParameterValue ("feedback")->load();
    g.saveState(); g.reduceClipRegion (r.toNearestInt());
    for (int i=0;i<8;++i) { float rr=radius*(0.45f+i*0.105f); g.setColour (violet.withAlpha (0.10f + (i%2)*0.04f)); g.drawEllipse (c.x-rr,c.y-rr,rr*2,rr*2, i==0?1.8f:0.8f); }
    g.setColour (violet.withAlpha (0.18f)); g.drawLine (r.getX(),c.y,r.getRight(),c.y,0.7f); g.drawLine (c.x,r.getY(),c.x,r.getBottom(),0.7f);
    juce::Path orbit; orbit.addEllipse (c.x-radius*(1.15f+width*0.4f),c.y-radius*0.48f,radius*(2.3f+width*0.8f),radius*0.96f);
    juce::AffineTransform rot = juce::AffineTransform::rotation (-0.28f,c.x,c.y); orbit.applyTransform (rot);
    g.setColour (violet.withAlpha (0.85f)); g.strokePath (orbit, juce::PathStrokeType (2.4f));
    const int repeatDots = 3 + (int) std::round (feedback * 4.0f);
    for (int i=0;i<repeatDots;++i) { float a=visualPhase*(0.5f+i*0.07f)+i*juce::MathConstants<float>::twoPi/(float) repeatDots; float x=c.x+std::cos(a)*radius*(0.62f+(i%3)*0.2f); float y=c.y+std::sin(a)*radius*(0.62f+(i%3)*0.2f)*(0.25f+width*0.25f);
        g.setColour (violet.withAlpha (0.15f)); g.fillEllipse (x-11,y-11,22,22); g.setColour (pale.withAlpha (0.65f+mutation*0.3f)); g.fillEllipse (x-3.5f,y-3.5f,7,7); }
    g.setColour (violet); g.fillEllipse (c.x-5,c.y-5,10,10); g.setColour (violet.withAlpha (0.12f)); g.fillEllipse (c.x-radius*.22f,c.y-radius*.22f,radius*.44f,radius*.44f);
    g.restoreState();
}
void DMFEchoAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto b=getLocalBounds().toFloat(); g.fillAll (bg);
    juce::ColourGradient grad (juce::Colour (0xff211027),0,0,bg,b.getWidth(),b.getHeight(),false); g.setGradientFill (grad); g.fillRect (b);
    g.setColour (violet.withAlpha (0.12f)); for (int i=0;i<12;++i) g.drawLine (0.0f,120.0f+i*31.0f,b.getWidth(),120.0f+i*31.0f,0.4f);
    auto border = b.reduced (12); g.setColour (violet.withAlpha (0.75f)); g.drawRoundedRectangle (border,12,1.2f);
    g.setColour (juce::Colour (0xff160f1b)); g.fillRoundedRectangle (juce::Rectangle<float>(22,98,b.getWidth()-44,320),10);
    g.setColour (violet.withAlpha (0.45f)); g.drawRoundedRectangle (juce::Rectangle<float>(22,98,b.getWidth()-44,320),10,1.0f);
    g.setColour (panel); g.fillRoundedRectangle (juce::Rectangle<float>(34,120,215,260),8);
    g.fillRoundedRectangle (juce::Rectangle<float>(b.getWidth()-230,120,196,260),8);
    g.setColour (violet.withAlpha (0.5f)); g.drawRoundedRectangle (juce::Rectangle<float>(34,120,215,260),8,1);
    g.drawRoundedRectangle (juce::Rectangle<float>(b.getWidth()-230,120,196,260),8,1);
    drawVisualizer (g, {260,110,b.getWidth()-510,290});
    g.setColour (juce::Colour (0xff0d0a10)); g.fillRoundedRectangle (juce::Rectangle<float>(22,430,b.getWidth()-44,202),10);
    g.setColour (violet.withAlpha (0.5f)); g.drawRoundedRectangle (juce::Rectangle<float>(22,430,b.getWidth()-44,202),10,1);
    g.setColour (violet.withAlpha (0.28f)); g.drawLine (28,94,b.getWidth()-28,94,1.0f);
    g.setColour (violet.withAlpha (0.45f)); for (int i=0;i<45;++i) { float h=8+(float)((i*19+7)%32); g.fillRect (35.0f+i*5.4f,410.0f-h,3.0f,h); }
    g.setColour (violet.withAlpha (0.8f)); g.drawLine (b.getWidth()*0.36f,650,b.getWidth()*0.64f,650,1.2f);
    g.setColour (pale.withAlpha (0.65f)); g.setFont (12); g.drawText ("KEY", (int)b.getWidth()-218,125,60,18,juce::Justification::left);
    g.drawText ("SCALE", (int)b.getWidth()-218,188,80,18,juce::Justification::left);
    g.drawText ("PITCH MOTION", (int)b.getWidth()-218,251,130,18,juce::Justification::left);
    g.drawText ("DELAY TIME", 52,300,150,18,juce::Justification::centred);
    g.setColour (violet); g.setFont (juce::Font (juce::FontOptions (28,juce::Font::bold))); g.drawText (juce::String ((int)processor.parameters.getRawParameterValue("delay")->load())+" ms",52,320,150,42,juce::Justification::centred);
    const float meter=processor.getMeter(); g.setColour (violet.withAlpha (.18f)); g.fillRoundedRectangle (34,603,b.getWidth()-68,10,4); g.setColour (violet); g.fillRoundedRectangle (34,603,(b.getWidth()-68)*meter,10,4);
}
void DMFEchoAudioProcessorEditor::resized()
{
    if (knobs[5].value == nullptr) return;
    auto b=getLocalBounds(); const float sx=b.getWidth()/1200.0f, sy=b.getHeight()/675.0f; auto R=[&](float x,float y,float w,float h){return juce::Rectangle<int>((int)(x*sx),(int)(y*sy),(int)(w*sx),(int)(h*sy));};
    logo.setBounds (R(34,18,175,62)); brand.setBounds (R(210,24,115,48)); title.setBounds (R(34,76,300,16)); preset.setBounds (R(430,25,340,38)); saveButton.setBounds (R(780,25,42,38)); settingsButton.setBounds (R(1110,24,48,40)); footerBrand.setBounds (R(930,638,220,22));
    sync.setBounds (R(52,145,175,38)); division.setBounds (R(52,214,175,40));
    key.box->setBounds (R(990,145,165,34)); scale.box->setBounds (R(990,208,165,34)); motion.box->setBounds (R(990,271,165,34)); chordMode.setBounds (R(990,324,160,28)); chord.box->setBounds (R(990,354,165,30));
    const int start=35, gap=(b.getWidth()-70)/6;
    for (int i=0;i<6;++i) { int x=start+i*gap; knobs[i].slider->setBounds (x+gap/2-61,(int)(sy*450),122,(int)(sy*142)); knobs[i].label->setBounds (x,(int)(sy*588),gap,22); knobs[i].value->setBounds (x,(int)(sy*611),gap,20); }
}
void DMFEchoAudioProcessorEditor::timerCallback() { visualPhase += 0.025f + processor.getMeter()*0.035f; repaint (juce::Rectangle<int>(250,100,getWidth()-500,320)); }
