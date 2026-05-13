// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

//==============================================================
//                      SETUP FOR CRTS SHADER
//==============================================================
// Try different masks
// #define CRTS_MASK_GRILLE 1
// #define CRTS_MASK_GRILLE_LITE 1
// #define CRTS_MASK_NONE 1
#define CRTS_MASK_SHADOW 1
//--------------------------------------------------------------
// Scanline thinness
//  0.50 = fused scanlines
//  0.70 = recommended default
//  1.00 = thinner scanlines (too thin)
#define INPUT_THIN 0.70
//--------------------------------------------------------------
// Horizontal scan blur
//  -3.0 = pixely
//  -2.5 = default
//  -2.0 = smooth
//  -1.0 = too blurry
#define INPUT_BLUR -2.5
//--------------------------------------------------------------
// Shadow mask effect, ranges from,
//  0.25 = large amount of mask (not recommended, too dark)
//  0.50 = recommended default
//  1.00 = no shadow mask
#define INPUT_MASK 0.5
//--------------------------------------------------------------
// How much bright pixels widen their scanline (phosphor saturation).
// Brighter beam = wider beam spot = scanline bleeds into the dark gaps.
//  0.0 = constant scanline thickness (original CRTS behavior)
//  0.3 = recommended subtle effect
//  0.5 = strong, very visible bloom-into-gaps
#define INPUT_BEAM_BLOOM 0.0
//--------------------------------------------------------------
// Corner vignetting strength (lens / phosphor edge falloff). Applied in
// warped screen space so corners darken more than edges, matching the
// geometry of a real curved CRT face.
//  0.00 = off
//  0.20 = recommended subtle effect
//  0.50 = strong, very dark corners
#define INPUT_VIGNETTE 0.20

