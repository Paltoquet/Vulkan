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

glm::ivec3 intergerMod(const glm::ivec3& first, const glm::ivec3& second)
{
    return glm::ivec3(first.x % second.x, first.y % second.y, first.z % second.z);
}

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
                samplePoint.z = 0.0f;
                minDist2 = glm::min(minDist2, glm::length2(currentPos - samplePoint));
                //std::cout << "current pos " << glm::to_string(currentPos) << std::endl;
                //std::cout << "sample point" << glm::to_string(samplePoint) << std::endl;
                //std::cout << "dist " << glm::length(currentPos - samplePoint) << std::endl;
            }
        }
    }
    //std::cout << std::endl;

    float result = std::sqrt(minDist2); // / std::sqrt(2);
    return result;


    /*glm::ivec3 currentIndex = pos;
    //std::cout << "pos " << glm::to_string(pos) << " index " << glm::to_string(currentIndex) << std::endl;
    currentIndex = intergerMod(currentIndex, m_kernelSize);
    uint32_t width = m_kernelSize.x;
    uint32_t height = m_kernelSize.y;
    uint32_t pageSize = width * height;
    glm::vec3 currentPoint = glm::mod(currentIndex, glm::vec3(1.0f));
    glm::vec3 randomPoint;
    float minDist2 = 100.0f; // std::numeric_limits<float>::max();

    for (int32_t k = -1; k <= 1; k++) {
        for (int32_t j = -1; j <= 1; j++) {
            for (int32_t i = -1; i <= 1; i++) {
                glm::ivec3 index = currentIndex + glm::ivec3(i, j, k);
                index = intergerMod(index, m_kernelSize);
                index.x = index.x < 0 ? m_kernelSize.x - 1 : index.x;
                index.y = index.y < 0 ? m_kernelSize.y - 1 : index.y;
                index.z = index.z < 0 ? m_kernelSize.z - 1 : index.z;

                //std::cout << "index " << glm::to_string(index) << std::endl;

                randomPoint = m_kernelData[index.x + index.y * width + index.z * pageSize];

                //std::cout << "current point " << glm::to_string(currentPoint) << std::endl;
                //std::cout << "random " << glm::to_string(randomPoint) << std::endl;

                float dist = glm::length2(currentPoint - randomPoint);
                minDist2 = std::min(minDist2, dist);
            }
        }
    }

    //std::cout << "distance " << minDist2 << std::endl;

    float result = std::sqrt(minDist2);

    //std::cout << "distance " << result << std::endl;

    // body diagonal of a cube
    glm::vec3 boxDiagonal = m_kernelOffset * m_kernelOffset;
    float maxDist = static_cast<float>(std::sqrt(boxDiagonal.x + boxDiagonal.y + boxDiagonal.z));

    //std::cout << "max dust " << maxDist << std::endl;

    result /= maxDist;
    return result;*/
}

/* --------------------------------- Private methods --------------------------------- */

void WorleyNoise3D::computeKernel()
{
    glm::vec3 kernelSizeF = m_kernelSize;
    m_kernelOffset = glm::vec3(1.0f) / kernelSizeF;
    uint32_t width = m_kernelSize.x;
    uint32_t height = m_kernelSize.y;
    uint32_t depth = m_kernelSize.z;
    uint32_t pageSize = width * height;

    size_t kernel_size = width * height * depth;
    m_kernelData.clear();
    m_kernelData.resize(kernel_size);

    std::mt19937 gen(m_randomSeed);
    std::uniform_real_distribution<float> distrFloat;
    auto randFloat = std::bind(distrFloat, gen);

    glm::vec3 topLeft;
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
    glm::vec3 index = glm::min(glm::vec3(5.0f), pos + glm::vec3(1.0));
    //[0 : kernelsize[
    index = glm::mod(index, m_kernelSize);
    return m_kernelData[index.x + index.y * m_kernelSize.x + index.z * m_kernelSize.y * m_kernelSize.x];
}