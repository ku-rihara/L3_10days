#pragma once
#include"Matrix4x4.h"
#include"Vector4.h"
#include <cstdint>


struct ParticleFprGPU {
    Matrix4x4 WVP;
    Matrix4x4 World;
    Matrix4x4 WorldInverseTranspose;
    Matrix4x4 UVTransform;
    Vector4 color;
    uint32_t isFlipX;
    uint32_t isFlipY;
};

