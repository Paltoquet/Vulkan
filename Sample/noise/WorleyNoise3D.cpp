#include "WorleyNoise3D.h"

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

WorleyNoise3D::WorleyNoise3D(const glm::ivec3& kernelSize):
    m_kernelSize(kernelSize),
    m_randomSeed(42)
{
    computeKernel();
}


WorleyNoise3D::~WorleyNoise3D()
{

}

/* --------------------------------- Public methods --------------------------------- */
/*
    Works on the unit cube, pos [0, 1]
*/
float WorleyNoise3D::evaluate(const glm::vec3& pos, float scale) const
{
    glm::vec3 scaledPosition = pos * scale;
    glm::vec3 index = glm::floor(scaledPosition);
    glm::vec3 fract = glm::fract(scaledPosition);
    glm::vec3 currentPos = index + fract;

    float minDist2 = 100.0f;
    glm::vec3 currentIndex, samplePoint;
    for (int32_t k = -1; k <= 1; k++) {
        for (int32_t j = -1; j <= 1; j++) {
            for (int32_t i = -1; i <= 1; i++) {
                currentIndex = index + glm::vec3(i, j, k);
                samplePoint = currentIndex + getKernelData(currentIndex);
                minDist2 = glm::min(minDist2, glm::length2(currentPos - samplePoint));
            }
        }
    }

    float result = std::sqrt(minDist2); 
    result = glm::min(result, 1.0f);
    return result;
}

/* --------------------------------- Private methods --------------------------------- */

void WorleyNoise3D::computeKernel()
{
    glm::vec3 kernelSizeF = m_kernelSize;
    m_kernelOffset = glm::vec3(1.0f) / kernelSizeF;
    uint32_t width = static_cast<uint32_t>(m_kernelSize.x);
    uint32_t height = static_cast<uint32_t>(m_kernelSize.y);
    uint32_t depth = static_cast<uint32_t>(m_kernelSize.z);
    uint32_t pageSize = width * height;

    size_t kernel_size = width * height * depth;
    m_kernelData.clear();
    m_kernelData.resize(kernel_size);

    std::mt19937 gen(m_randomSeed);
    std::uniform_real_distribution<float> distrFloat;
    auto randFloat = std::bind(distrFloat, gen);

    for (uint32_t k = 0; k < depth; k++) {
        for (uint32_t j = 0; j < height; j++) {
            for (uint32_t i = 0; i < width; i++) {
                glm::vec3 randomOffset = glm::vec3(randFloat(), randFloat(), randFloat());
                m_kernelData[i + j * width + k * pageSize] = randomOffset;
            }
        }
    }
}
// pos inside [-1, scale] 
glm::vec3 WorleyNoise3D::getKernelData(const glm::vec3& pos) const
{
    //[0 : width + 1]
    glm::vec3 index = pos + glm::vec3(1.0);
    //[0 : kernelsize[
    index = glm::mod(index, m_kernelSize);
    std::size_t kernelIndex = static_cast<std::size_t>(index.x + index.y * m_kernelSize.x + index.z * m_kernelSize.y * m_kernelSize.x);
    return m_kernelData[kernelIndex];
}