# R2RVERB 2.0 — DJ R2R Music

**Native Windows x64 VST3 · FL Studio 21 · Hall / Room / Plate**

R2RVERB 2.0 is a clean JUCE-based stereo reverb plugin rebuild. The previous processor and editor source files have been replaced with a smaller, fresh implementation. The new VST3 identity is versioned separately so FL Studio can distinguish it from the previous build.

## Features
- Hall, Room and Plate voicings
- Size, Decay, Damping, Stereo Width and Wet Mix
- Bypass
- Parameter state save/restore
- New DJ R2R Music black / cyan interface
- Windows x64 build using the static MSVC runtime

The three modes are tuned voicings of JUCE's stereo reverb engine; they are not separate physical-model algorithms.

## Build
Requirements: Windows 10/11 x64, Visual Studio 2022 Desktop C++, CMake 3.22+, Git and internet access for JUCE.

```powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target R2RVERB_VST3 --clean-first --parallel
```

The generated VST3 bundle is under:

```
build/R2RVERB_artefacts/Release/VST3/R2RVERB.vst3
```

Copy the entire `R2RVERB.vst3` folder into:

```
C:\Program Files\Common Files\VST3\
```

Then close and reopen FL Studio 21, open **Options → Manage plugins**, and run **Find installed plugins**.

## GitHub Actions
A push to `main` that changes the source, CMake configuration, README or workflow starts a clean Windows x64 build. The workflow verifies the x64 VST3 binary exists before uploading the complete bundle as `R2RVERB-2.0-Windows-x64-VST3`.

**Important:** A successful CI build confirms compilation and bundle creation. Final validation still requires opening the plugin in FL Studio on a Windows machine.

© DJ R2R Music
