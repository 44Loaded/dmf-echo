# DMF ECHO — Musical Delay

A JUCE/C++17 VST3 and Standalone audio effect project for DIGITAL MAFIA.

## Architecture

- `PluginProcessor`: JUCE AudioProcessor, APVTS parameter tree, host tempo, state persistence, factory programs.
- `DSP.h`: preallocated stereo delay buffers, feedback loop, scale/chord note mapping, pitch-shift stage, stereo width and saturation.
- `PluginEditor`: custom-drawn dark/violet interface, orbital animation, parameter attachments, factory preset selector and user preset file dialogs.

## Build prerequisites

- CMake 3.22+
- C++17 toolchain (Xcode Command Line Tools on macOS; Visual Studio 2022 with Desktop C++ workload on Windows)
- JUCE 8.0.6 (CMake FetchContent downloads it on first configure; alternatively point FetchContent at a local JUCE checkout)

## Build — macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target DMF_ECHO_VST3
```

The VST3 bundle is normally written under `build/DMF_ECHO_artefacts/Release/VST3/DMF ECHO.vst3`.

## Build — Windows

Open an x64 Native Tools Command Prompt for VS 2022:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target DMF_ECHO_VST3
```

The bundle is normally under `build\DMF_ECHO_artefacts\Release\VST3\DMF ECHO.vst3`.

## FL Studio installation

1. Build the VST3 bundle for your operating system.
2. Copy `DMF ECHO.vst3` to the system VST3 directory:
   - macOS: `/Library/Audio/Plug-Ins/VST3/`
   - Windows: `C:\Program Files\Common Files\VST3\`
3. In FL Studio, open **Options → Manage plugins** and scan the VST3 location.
4. Add **DMF ECHO** to a Mixer insert effect slot.
5. Enable BPM Sync to follow the host tempo; disable it to use the free millisecond delay control.

## Testing checklist

- Test mono and stereo input/output at 44.1, 48, 88.2 and 96 kHz.
- Automate Delay, Feedback, Mix, Mutate, Space and Dirt from the DAW.
- Change host tempo while BPM Sync is active.
- Save and reopen a DAW project to verify parameter-state restoration.
- Test all eight factory programs and each key/scale/motion selection.
- Check output level and feedback stability at maximum Feedback.

## Important engineering notes

This is a functional first implementation, not a certified commercial release. The pitch stage uses a lightweight dual-head modulated delay-line shifter intended for echo feedback; it can introduce grain/chorus coloration and is not equivalent to a high-end phase-vocoder or commercial pitch-shift library. The visualizer is a lightweight UI animation driven by the output meter and selected settings. User presets can be saved to and loaded from `.dmfecho` XML files; DAW project state is also restored through APVTS. The DSP buffers are allocated in `prepareToPlay`, not in `processBlock`.

Before commercial distribution, run the JUCE AudioPluginHost and DAW validation suites, add oversampling/anti-alias filtering where desired, improve pitch-shifter transient handling, implement a true user preset browser/file workflow, and perform listening tests across the supported sample rates and buffer sizes.
