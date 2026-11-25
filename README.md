# R2RVERB 2.0 — DJ R2R Music

Native Windows x64 VST3 reverb for FL Studio 21 and compatible hosts.

## Controls
- Hall / Room / Plate voicings
- Size, Damping, Stereo Width
- Independent Dry and Wet level knobs
- Tempo-synchronised Decay Time selector: 1/2, 1/3, 1/4, 1/6, 1/8, 1/16, 1/32
- Bypass and parameter state save/restore
- White interface with cyan accents

Decay divisions follow the host tempo when the host reports BPM; 120 BPM is used when tempo is unavailable. The division controls the JUCE reverb room-size/decay character, not a guaranteed exact RT60 time because JUCE's stock reverb does not expose a direct decay-time-in-seconds parameter.

## Build
```powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target R2RVERB_VST3 --clean-first --parallel
```
Output: `build/R2RVERB_artefacts/Release/VST3/R2RVERB.vst3`
