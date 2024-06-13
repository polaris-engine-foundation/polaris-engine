// -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*-

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

//
// GameShaders: The Metal version of the shaders
//

#include <metal_stdlib>
#include <metal_common>
#include <simd/simd.h>

using namespace metal;

#include "GameShaderTypes.h"

// Vertex shader outputs and fragment shader inputs
struct RasterizerData {
    float4 position [[position]];
    float2 textureCoordinate;
    float alpha;
};

vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             constant GameVertex *vertexArray [[buffer(GameVertexInputIndexVertices)]])
{
    RasterizerData out;
    out.position = vector_float4(vertexArray[vertexID].xy, 0, 1);
    out.textureCoordinate = vertexArray[vertexID].uv;
    out.alpha = vertexArray[vertexID].alpha;
    return out;
}

fragment float4
fragmentNormalShader(RasterizerData in [[stage_in]],
                     texture2d<half> colorTexture [[texture(GameTextureIndexColor)]])
{
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);
    const half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    return float4(colorSample.r, colorSample.g, colorSample.b, colorSample.a * in.alpha);
}

fragment float4
fragmentCopyShader(RasterizerData in [[stage_in]],
                   texture2d<half> colorTexture [[texture(GameTextureIndexColor)]])
{
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);
    half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    colorSample.a = 1.0;
    return float4(colorSample);
}

fragment float4
fragmentDimShader(RasterizerData in [[stage_in]],
                  texture2d<half> colorTexture [[texture(GameTextureIndexColor)]])
{
    constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);
    half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    colorSample.rgb *= 0.5;
    return float4(colorSample);
}

fragment float4
fragmentRuleShader(RasterizerData in [[stage_in]],
                   texture2d<half> colorTexture [[texture(GameTextureIndexColor)]],
                   texture2d<half> ruleTexture [[texture(GameTextureIndexRule)]])
{
    constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);
    half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    const half4 ruleSample = ruleTexture.sample(textureSampler, in.textureCoordinate);
    colorSample.a = 1.0 - step(in.alpha, (float)ruleSample.b);
    return float4(colorSample.r, colorSample.g, colorSample.b, colorSample.a);
}

fragment float4
fragmentMeltShader(RasterizerData in [[stage_in]],
                   texture2d<half> colorTexture [[texture(GameTextureIndexColor)]],
                   texture2d<half> ruleTexture [[texture(GameTextureIndexRule)]])
{
    constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);
    half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    const half4 ruleSample = ruleTexture.sample(textureSampler, in.textureCoordinate);
    colorSample.a = clamp((1.0 - ruleSample.b) + (in.alpha * 2.0 - 1.0), 0.0, 1.0);
    return float4(colorSample);
}
