#include "BrownianNoise.h"

#include <cmath> 
#include <cstdio> 
#include <random> 
#include <functional> 
#include <iostream> 
#include <fstream> 

BrownianNoise::BrownianNoise(const glm::ivec2& kernelSize, size_t nbLayers):
    m_kernelSize(kernelSize),
    m_nbLayers(nbLayers),
    m_randomSeed(42),
    m_baseFrequency(0.05),
    m_rateOffChanged(2.0)
{
    computeKernel();
}


BrownianNoise::~BrownianNoise()
{

}

/* --------------------------------- Public methods --------------------------------- */

float BrownianNoise::evaluate(const glm::vec2& pos) const
{
    float brownianNoise = 0.0;
    float noiseMax = 0.0;

    for (size_t i = 0; i < m_nbLayers; ++i)
    {
        float frequency = m_baseFrequency * pow(m_rateOffChanged, i);
        float amplitude = pow(m_rateOffChanged, i);
        brownianNoise += computeNoise(pos * frequency) / amplitude;
        noiseMax += 1.0 / amplitude;
    }
    brownianNoise = brownianNoise / noiseMax;
    return brownianNoise;
}

/* --------------------------------- Private methods --------------------------------- */

float BrownianNoise::computeNoise(const glm::vec2& pos) const
{
    int xi = std::floor(pos.x);
    int yi = std::floor(pos.y);

    float tx = pos.x - xi;
    float ty = pos.y - yi;

    size_t kernelWidth = m_kernelSize.x;

    int rx0 = xi % m_kernelSize.x;
    int rx1 = (rx0 + 1) % m_kernelSize.x;
    int ry0 = yi % m_kernelSize.y;
    int ry1 = (ry0 + 1) % m_kernelSize.y;

    // random values at the corners of the cell using permutation table
    const float & c00 = m_kernelData[ry0 * kernelWidth + rx0];
    const float & c10 = m_kernelData[ry0 * kernelWidth + rx1];
    const float & c01 = m_kernelData[ry1 * kernelWidth + rx0];
    const float & c11 = m_kernelData[ry1 * kernelWidth + rx1];

    // remapping of tx and ty using the Smoothstep function 
    float sx = smoothstep(tx);
    float sy = smoothstep(ty);

    // linearly interpolate values along the x axis
    float nx0 = interpolate(c00, c10, sx);
    float nx1 = interpolate(c01, c11, sx);

    // linearly interpolate the nx0/nx1 along they y axis
    return interpolate(nx0, nx1, sy);
}

float BrownianNoise::smoothstep(float val) const
{
    return val * val * (3 - 2 * val);
}

float BrownianNoise::interpolate(float min, float max, float value) const
{
    return glm::mix(min, max, value);
}

void BrownianNoise::computeKernel()
{
    size_t kernel_size = m_kernelSize.x * m_kernelSize.y;
    m_kernelData.clear();
    m_kernelData.resize(kernel_size);

    std::mt19937 gen(m_randomSeed);
    std::uniform_real_distribution<float> distrFloat;
    auto randFloat = std::bind(distrFloat, gen);

    for (unsigned k = 0; k < kernel_size; ++k) {
        m_kernelData[k] = randFloat();
    }
}