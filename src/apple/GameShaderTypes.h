// -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*-

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

//
// GameShaderTypes: The Metal version of the shaders for x-engine
//

#ifndef XENGINE_GAMESHADERTYPES_H
#define XENGINE_GAMESHADERTYPES_H

#include <simd/simd.h>

enum GameVertexInputIndex {
    GameVertexInputIndexVertices = 0,
};

enum {
    GameTextureIndexColor = 0,
    GameTextureIndexRule = 1,
};

struct GameVertex {
    vector_float2 xy;
    vector_float2 uv;
    float alpha;
    float padding;  // This is absolutely needed for 64-bit alignments
};

#endif
