# Product

<!-- impeccable:product-schema 1 -->

## Platform

desktop

Native JUCE audio plugin (AU/VST3/Standalone, macOS 11+). Sibling of the MXA suite; family design authority: `../PhaserMXA/DESIGN.md`; family product context: `../PhaserMXA/PRODUCT.md`.

## Product Purpose

A passive-style program equalizer: a low shelf boost/atten pair on a selectable frequency (the atten corner sits ×1.6 higher — pushing both carves the classic dip), a broad high peak on a sweepable frequency, and a high shelf cut.

## Capabilities and Constraints

- Exactly six parameters: `lf` (30/60/100 Hz choice), `lfboost` (0..+14 dB), `lfcut` (0..−16 dB), `hffreq` (3–16 kHz), `hfboost` (0..+16 dB peak, Q 0.7), `hfcut` (0..−16 dB shelf at hffreq).
- Four RBJ biquads in cascade, identical coefficients both channels, gains slewed one step per block (`EQEngine`).
- UI truth taps: static `sectionsFor()` / `responseDb()` / `lfFreqFor()` — the single source of truth for FIG. 1's composite curve (dip included, with a live dip tally) and FIG. 2's printed section values.
- Editor: Service Manual family sheet, 820×470, spot ink Prussian-blue #5C85E6, DWG NO. MXA-EQ-01.

## Brand Commitments

Inherits the family's: MXAudio, "BY MESCALINA" credit, one spot ink per sibling (EQ = Prussian-blue).

## Evidence on Hand

Working DSP (`Source/EQEngine.*`). No users/testimonials — nothing may be fabricated.
