# EQMXA

A passive-style program equalizer: a low shelf boost/atten pair on a selectable frequency (the atten corner sits ×1.6 higher — pushing both carves the classic dip), a broad high peak on a sweepable frequency, and a high shelf cut.

Audio plugin (AU / VST3 / Standalone) built with [JUCE](https://juce.com). Part of the MXA plugin suite. macOS 11+.

## Build

```sh
git clone --recurse-submodules https://github.com/uprod/EQMXA.git
cd EQMXA
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

If you already have the MXA suite checked out with a shared `../JUCE` folder, the submodule is optional — the build falls back to the sibling folder automatically.
