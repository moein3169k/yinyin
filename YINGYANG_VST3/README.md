# YINGYANG VST3 — prototype 0.1

A JUCE-based Windows x64 VST3 vocal processor prototype inspired by fast FET and slow optical compression behavior. This is an original DSP design, not a circuit-level emulation of any branded hardware.

## Signal chain
1. Three-band tone (broad low/mid/high bands) + soft saturation
2. Blendable fast/slow envelope compressor character
3. Split-band de-esser
4. Gentle low-level downward expander (not spectral AI denoising)
5. Output gain

## Build locally (optional)
Requires CMake, a C++17 compiler / Visual Studio C++ workload, and internet access so CMake can fetch JUCE 8.0.6.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target YINGYANG_VST3
```

## Build without installing a compiler locally
1. Create a GitHub repository and upload the contents of this folder to its `main` branch.
2. Open the repository's **Actions** tab and run **Build YINGYANG VST3 (Windows x64)**.
3. Download the `YINGYANG-VST3-Windows-x64` artifact from the completed workflow run.
4. Extract the `.vst3` bundle and install it in the standard VST3 location (normally `C:\Program Files\Common Files\VST3`).
5. In FL Studio, open **Manage plugins** and run a scan.

## Important prototype notes
- This source has not been compiled or auditioned in this environment. Treat it as an initial development build, not a release-ready commercial plugin.
- The Noise Reduction control is a gentle downward expander; it is not a neural/spectral denoiser. Use moderate settings to avoid pumping or cutting breaths.
- The compressor uses shared threshold/ratio with a fast detector and a slower detector blend. It is not an exact 1176 or LA-2A model.
- Before critical work, level-match bypass and processed audio. Saturation and compression can increase perceived loudness.
