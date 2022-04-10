#include "BrownianNoise3D.h"

#include <cmath> 
#include <cstdio> 
#include <random> 
#include <functional> 
#include <iostream> 
#include <fstream> 

BrownianNoise3D::BrownianNoise3D(const glm::ivec3& kernelSize, size_t nbLayers, float randomSeed):
    m_kernelSize(kernelSize),
    m_nbLayers(nbLayers),
    m_randomSeed(randomSeed),
    m_baseFrequency(0.05f),
    m_rateOffChanged(2.0f)
{
    computeKernel();
}


BrownianNoise3D::~BrownianNoise3D()
{

}

/* --------------------------------- Public methods --------------------------------- */

float BrownianNoise3D::evaluate(const glm::vec3& pos) const
{
    float value = 0.0;
    float noiseMax = 0.0;

    for (size_t i = 0; i < m_nbLayers; ++i)
    {
        float frequency = m_baseFrequency * pow(m_rateOffChanged, i);
        float amplitude = pow(m_rateOffChanged, i);
        value += computeNoise(pos * frequency) / amplitude;
        noiseMax += 1.0f / amplitude;
    }
    value = value / noiseMax;
    return value;
}

/* --------------------------------- Private methods --------------------------------- */

float BrownianNoise3D::computeNoise(const glm::vec3& pos) const
{
    int xi = std::floor(pos.x);
    int yi = std::floor(pos.y);
    int zi = std::floor(pos.z);

    float tx = pos.x - xi;
    float ty = pos.y - yi;
    float tz = pos.z - zi;

    size_t kernelWidth = m_kernelSize.x;
    size_t kernelPageSize = m_kernelSize.y * m_kernelSize.x;

    int rx0 = xi % m_kernelSize.x;
    int rx1 = (rx0 + 1) % m_kernelSize.x;
    int ry0 = yi % m_kernelSize.y;
    int ry1 = (ry0 + 1) % m_kernelSize.y;
    int rz0 = zi % m_kernelSize.z;
    int rz1 = (rz0 + 1) % m_kernelSize.z;

    // random values at the corners of the cell using permutation table
    const float & c000 = m_kernelData[rz0 * kernelPageSize + ry0 * kernelWidth + rx0];
    const float & c100 = m_kernelData[rz0 * kernelPageSize + ry0 * kernelWidth + rx1];
    const float & c010 = m_kernelData[rz0 * kernelPageSize + ry1 * kernelWidth + rx0];
    const float & c110 = m_kernelData[rz0 * kernelPageSize + ry1 * kernelWidth + rx1];

    const float & c001 = m_kernelData[rz1 * kernelPageSize + ry0 * kernelWidth + rx0];
    const float & c101 = m_kernelData[rz1 * kernelPageSize + ry0 * kernelWidth + rx1];
    const float & c011 = m_kernelData[rz1 * kernelPageSize + ry1 * kernelWidth + rx0];
    const float & c111 = m_kernelData[rz1 * kernelPageSize + ry1 * kernelWidth + rx1];

    // remapping of tx and ty using the Smoothstep function 
    float sx = smoothstep(tx);
    float sy = smoothstep(ty);
    float sz = smoothstep(tz);

    // linearly interpolate values along the x axis
    float nx00 = interpolate(c000, c100, sx);
    float nx10 = interpolate(c010, c110, sx);

    float nx01 = interpolate(c001, c101, sx);
    float nx11 = interpolate(c011, c111, sx);

    float ny0 = interpolate(nx00, nx10, sy);
    float ny1 = interpolate(nx01, nx11, sy);

    // linearly interpolate the nx0/nx1 along they z axis
    return interpolate(ny0, ny1, sz);
}

float BrownianNoise3D::smoothstep(float val) const
{
    return val * val * (3 - 2 * val);
}

float BrownianNoise3D::interpolate(float min, float max, float value) const
{
    return glm::mix(min, max, value);
}

void BrownianNoise3D::computeKernel()
{
    size_t kernel_size = m_kernelSize.x * m_kernelSize.y * m_kernelSize.z;
    m_kernelData.clear();
    m_kernelData.resize(kernel_size);

    std::mt19937 gen(m_randomSeed);
    std::uniform_real_distribution<float> distrFloat;
    auto randFloat = std::bind(distrFloat, gen);

    for (unsigned k = 0; k < kernel_size; ++k) {
        m_kernelData[k] = randFloat();
    }
}