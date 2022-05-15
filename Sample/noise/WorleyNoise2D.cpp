#include "WorleyNoise2D.h"

#include <cmath> 
#include <cstdio> 
#include <random> 
#include <functional> 
#include <iostream> 
#include <fstream> 
#include <limits>

#include <glm/gtc/integer.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

WorleyNoise2D::WorleyNoise2D(const glm::vec2& kernelSize):
    m_kernelSize(kernelSize),
    m_randomSeed(42)
{
    computeKernel();
}

WorleyNoise2D::WorleyNoise2D(const glm::vec2& kernelSize, float randomSeed) :
    m_kernelSize(kernelSize),
    m_randomSeed(randomSeed)
{
    computeKernel();
}


WorleyNoise2D::~WorleyNoise2D()
{

}

/* --------------------------------- Public methods --------------------------------- */
/*
    Works on the unit cube, pos [0, 1]
*/
float WorleyNoise2D::evaluate(const glm::vec2& pos, float scale) const
{
    glm::vec2 scaledPosition = pos * scale;
    glm::vec2 index = glm::floor(scaledPosition);
    glm::vec2 fract = glm::fract(scaledPosition);
    glm::vec2 currentPos = index + fract;

    float minDist2 = 100.0f;
    glm::vec2 currentIndex, samplePoint;
    for (int32_t j = -1; j <= 1; j++) {
        for (int32_t i = -1; i <= 1; i++) {
            currentIndex = index + glm::vec2(i, j);
            samplePoint = currentIndex + getKernelData(currentIndex);
            minDist2 = glm::min(minDist2, glm::length2(currentPos - samplePoint));
        }
    }

    glm::vec2 lineCoef = glm::min(fract, glm::vec2(1.0f - fract));
    float d = std::sqrt(minDist2);
    d = glm::min(d, 1.0f);
    //if (lineCoef.x < 0.01f || lineCoef.y < 0.01f) {
    //    d = glm::max(1.0f - lineCoef.x, d);
    //    d = glm::max(1.0f - lineCoef.y, d);
    //}
    return d;
}

/* --------------------------------- Private methods --------------------------------- */

void WorleyNoise2D::computeKernel()
{
    glm::vec2 kernelSizeF = m_kernelSize;
    m_kernelOffset = glm::vec2(1.0f) / kernelSizeF;
    uint32_t width = static_cast<uint32_t>(m_kernelSize.x);
    uint32_t height = static_cast<uint32_t>(m_kernelSize.y);

    size_t kernel_size = width * height;
    m_kernelData.clear();
    m_kernelData.resize(kernel_size);

    std::mt19937 gen(m_randomSeed);
    std::uniform_real_distribution<float> distrFloat;
    auto randFloat = std::bind(distrFloat, gen);

    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            glm::vec2 randomOffset = glm::vec2(randFloat(), randFloat());
            m_kernelData[i + j * width] = randomOffset;
        }
    }
}
// pos inside [-1, scale] 
glm::vec2 WorleyNoise2D::getKernelData(const glm::vec2& pos) const
{
    //[0 : width + 1]
    glm::vec2 index = pos + glm::vec2(1.0);
    //[0 : kernelsize[
    index = glm::mod(index, m_kernelSize);
    std::size_t kernelIndex = static_cast<std::size_t>(index.x + index.y * m_kernelSize.x);
    return m_kernelData[kernelIndex];
}