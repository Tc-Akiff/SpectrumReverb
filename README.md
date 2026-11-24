# Spectra Reverb

Spectra Reverb is a stereo algorithmic reverb plugin for Windows and macOS DAWs. It builds as VST3 on both platforms, AU on macOS, and optionally AAX on Windows and macOS for Pro Tools. The interface follows the supplied silver-and-charcoal reference, with warm orange controls and live level displays.

Each plugin format and operating system requires its own build. VST3 is supported by many DAWs, AU is used by Logic Pro, and AAX is used by Pro Tools; no single plugin format is supported by every DAW. Building AAX requires a separately obtained Avid AAX SDK.

## DSP

The reverb uses an eight-line feedback delay network (FDN) with a Householder feedback matrix. Each line's feedback gain is derived from its delay time and the requested T60 decay:

`g_i = 10^(-3 * tau_i / T60)`

This sets the ideal decay to approximately 60 dB at `T60`. A damped feedback path, input all-pass diffusion, modulated fractional delays, stereo width, pre-delay, and equal-power dry/wet mixing shape the result. Parameter changes are smoothed to avoid zipper noise.

## Controls

- **Pre-delay:** 0-250 ms
- **Decay Time:** 0.2-20 s
- **Size / Damping / Mix**
- **Diffusion / Modulation / Color / Width**
- **Output:** -24 to +12 dB
- **Factory presets:** Init - Clear, Vocal Hall, Dark Plate, Wide Ambience, Cosmic Bloom

All continuous controls are exposed to DAW automation. Plugin state is saved and restored by the host.

## Build on Windows

Requirements: Visual Studio 2022 with the **Desktop development with C++** workload, CMake 3.24 or newer, and an internet connection for the first configure (JUCE 8.0.6 is fetched by CMake).

To build the VST3:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target SpectraReverb_VST3
```

The VST3 bundle is generated under `build\SpectraReverb_artefacts\Release\VST3\`. To also build AAX for Pro Tools, obtain the Avid AAX SDK and configure with its path:

```powershell
cmake -S . -B build-aax -G "Visual Studio 17 2022" -A x64 -DSPECTRA_BUILD_AAX=ON -DJUCE_AAX_SDK_PATH="C:\path\to\AAX_SDK"
cmake --build build-aax --config Release --target SpectraReverb_AAX
```

## Build on macOS

Requirements: Xcode command-line tools, CMake 3.24 or newer, and an internet connection for the first configure. VST3 and AU can be built without the AAX SDK. To include AAX, provide the SDK path when configuring.

```sh
cmake -S . -B build -G Xcode
cmake --build build --config Release --target SpectraReverb_VST3 SpectraReverb_AU
```

To build AAX as well:

```sh
cmake -S . -B build-aax -G Xcode -DSPECTRA_BUILD_AAX=ON -DJUCE_AAX_SDK_PATH="/path/to/AAX_SDK"
cmake --build build-aax --config Release --target SpectraReverb_AAX
```

Install the generated plugin bundles in the matching system plugin folder and rescan plugins in the DAW. AAX distribution and Pro Tools validation may require additional Avid developer requirements.
