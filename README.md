# R2RVERB — DJ R2R Music

**Black & Gold · Native Windows x64 VST3 · Hall / Room / Plate**

R2RVERB is a JUCE-based stereo reverb plugin project for Windows 10/11 and VST3 hosts such as FL Studio.

## Current project features

- Hall, Room and Plate voicing selector
- Size and Decay controls
- Pre-Delay
- Damping and Stereo Width
- Wet Mix
- Low Cut and High Cut on the reverb input
- Freeze and Bypass
- Black & Gold custom interface with DJ R2R Music branding
- Parameter state save/restore
- Windows x64 GitHub Actions build workflow

**DSP note:** The current first version uses JUCE's stereo reverb network with three tuned Hall/Room/Plate profiles. These are distinct parameter voicings, not three independent physical-model algorithms. Further DSP development can refine the plate and hall algorithms.

## Build on Windows

### Requirements
- Windows 10/11 64-bit
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.22 or newer
- Git
- Internet connection for JUCE FetchContent on first configure

### Configure and build
Open **x64 Native Tools Command Prompt for VS 2022** in this repository:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target R2RVERB_VST3
```

The VST3 bundle is generated under:

```
build/R2RVERB_artefacts/Release/VST3/R2RVERB.vst3
```

Copy the generated `.vst3` bundle to:

```
C:\Program Files\Common Files\VST3\
```

Then rescan plugins in your DAW.

## Automated build

The workflow **Build R2RVERB VST3 (Windows x64)** runs on pushes to `main` that touch the project source and can be run manually from the GitHub Actions tab. It uploads the VST3 build as a workflow artifact.

## Project layout

- `CMakeLists.txt` — JUCE plugin and x64 build configuration
- `Source/PluginProcessor.*` — audio processing, parameters and state
- `Source/PluginEditor.*` — Black & Gold interface
- `.github/workflows/windows-vst3.yml` — Windows build automation

© DJ R2R Music.