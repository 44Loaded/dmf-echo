#include "PluginProcessor.h"
#include "PluginEditor.h"

DMFEchoAudioProcessor::DMFEchoAudioProcessor()
 : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
   parameters (*this, nullptr, "DMFECHO_STATE", createParameterLayout())
{
    applyFactoryPreset (0);
}

juce::AudioProcessorValueTreeState::ParameterLayout DMFEchoAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    auto choice = [&p] (const char* id, const char* name, StringArray items, int def) {
        p.push_back (std::make_unique<AudioParameterChoice> (id, name, items, def)); };
    auto boolean = [&p] (const char* id, const char* name, bool def) {
        p.push_back (std::make_unique<AudioParameterBool> (id, name, def)); };
    auto ranged = [&p] (const char* id, const char* name, NormalisableRange<float> range, float def, const char* suffix = "") {
        p.push_back (std::make_unique<AudioParameterFloat> (id, name, range, def, AudioParameterFloatAttributes().withLabel (suffix))); };
    boolean ("sync", "BPM Sync", true);
    choice ("division", "Division", {"1/1","1/2","1/4","1/8","1/16","1/1 D","1/2 D","1/4 D","1/8 D","1/16 D","1/1 T","1/2 T","1/4 T","1/8 T"}, 2);
    ranged ("delay", "Delay", {20.0f, 2000.0f, 0.1f}, 250.0f, "ms");
    ranged ("feedback", "Feedback", {0.0f, 0.92f, 0.001f}, 0.60f, "%");
    ranged ("mix", "Mix", {0.0f, 1.0f, 0.001f}, 0.70f, "%");
    ranged ("mutate", "Mutate", {0.0f, 1.0f, 0.001f}, 0.40f, "%");
    ranged ("space", "Space", {0.0f, 1.0f, 0.001f}, 0.75f, "%");
    ranged ("dirt", "Dirt", {0.0f, 1.0f, 0.001f}, 0.30f, "%");
    choice ("key", "Key", {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 6);
    choice ("scale", "Scale", {"Major","Natural Minor","Harmonic Minor","Melodic Minor","Dorian","Phrygian","Lydian","Mixolydian","Locrian","Major Pentatonic","Minor Pentatonic"}, 1);
    choice ("motion", "Pitch Motion", {"STATIC","UP","DOWN","WAVE","RANDOM"}, 3);
    boolean ("chordOn", "Chord Mode", false);
    choice ("chord", "Chord", {"Major","Minor","Diminished","Augmented","Sus2","Sus4","Major 7","Minor 7","Dominant 7"}, 1);
    ranged ("bpm", "Host BPM", {30.0f, 300.0f, 0.1f}, 120.0f, "BPM");
    return { p.begin(), p.end() };
}

void DMFEchoAudioProcessor::prepareToPlay (double sr, int samples)
{
    engine.prepare (sr, getTotalNumOutputChannels());
}
void DMFEchoAudioProcessor::releaseResources() { engine.reset(); }
bool DMFEchoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
        && out == layouts.getMainInputChannelSet();
}
void DMFEchoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int inputs = getTotalNumInputChannels();
    for (int ch = inputs; ch < getTotalNumOutputChannels(); ++ch) buffer.clear (ch, 0, buffer.getNumSamples());
    auto v = [this] (const char* id) { return parameters.getRawParameterValue (id)->load(); };
    double bpm = 120.0;
    if (auto* ph = getPlayHead()) if (auto pos = ph->getPosition()) if (auto b = pos->getBpm()) bpm = *b;
    engine.process (buffer, v("delay"), v("feedback"), v("mix"), v("mutate"), v("space"), v("dirt"),
                    v("sync") > 0.5f, (float) bpm, (int) v("division"), (int) v("key"), (int) v("scale"),
                    (int) v("motion"), v("chordOn") > 0.5f, (int) v("chord"));
    float peak = buffer.getMagnitude (0, 0, buffer.getNumSamples());
    if (buffer.getNumChannels() > 1) peak = juce::jmax (peak, buffer.getMagnitude (1, 0, buffer.getNumSamples()));
    meter.store (juce::jlimit (0.0f, 1.0f, peak));
}
juce::AudioProcessorEditor* DMFEchoAudioProcessor::createEditor() { return new DMFEchoAudioProcessorEditor (*this); }
void DMFEchoAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = parameters.copyState(); std::unique_ptr<juce::XmlElement> xml (state.createXml()); copyXmlToBinary (*xml, dest);
}
void DMFEchoAudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml && xml->hasTagName (parameters.state.getType())) parameters.replaceState (juce::ValueTree::fromXml (*xml));
}
const juce::String DMFEchoAudioProcessor::getProgramName (int i)
{
    static const char* names[] = {"DREAMY VOCALS","CRYSTAL ECHO","DARK MELODY","LIQUID ARP","FLOATING NOTES","UNDERGROUND","REVERSE DREAMS","MINIMAL ECHO"};
    return names[juce::jlimit (0, 7, i)];
}
void DMFEchoAudioProcessor::setCurrentProgram (int i) { currentProgram = juce::jlimit (0, 7, i); applyFactoryPreset (currentProgram); }
void DMFEchoAudioProcessor::applyFactoryPreset (int i)
{
    struct Preset { float d,f,m,u,s,x; int key,scale,motion; bool chord; int chordType; };
    static constexpr Preset presets[] = {
        {250,.60f,.70f,.40f,.75f,.30f,6,1,3,false,1}, {420,.48f,.55f,.65f,.90f,.12f,2,0,1,true,6},
        {680,.72f,.42f,.82f,.65f,.58f,1,2,2,false,1}, {180,.52f,.48f,.72f,.88f,.18f,9,4,3,true,7},
        {900,.38f,.62f,.55f,.95f,.08f,4,0,4,false,0}, {330,.78f,.50f,.32f,.45f,.70f,6,1,0,false,1},
        {560,.58f,.68f,.90f,.92f,.22f,8,5,4,true,5}, {125,.25f,.35f,.0f,.30f,.05f,0,0,0,false,0}
    };
    const auto& a = presets[juce::jlimit (0, 7, i)];
    auto set = [this] (const char* id, float value) { parameters.getParameterAsValue (id).setValue (value); };
    set ("delay", a.d); set ("feedback", a.f); set ("mix", a.m); set ("mutate", a.u); set ("space", a.s); set ("dirt", a.x);
    set ("key", (float) a.key); set ("scale", (float) a.scale); set ("motion", (float) a.motion); set ("chordOn", a.chord ? 1.0f : 0.0f); set ("chord", (float) a.chordType);
}

bool DMFEchoAudioProcessor::savePresetToFile (const juce::File& file) const
{
    auto xml = parameters.copyState().createXml();
    return xml != nullptr && xml->writeTo (file);
}
bool DMFEchoAudioProcessor::loadPresetFromFile (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || !xml->hasTagName (parameters.state.getType())) return false;
    parameters.replaceState (juce::ValueTree::fromXml (*xml));
    return true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DMFEchoAudioProcessor(); }
