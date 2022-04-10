#include "CloudGenerator.h"

#include <noise/BrownianNoise3D.h>

#include <iostream>

/// Map a value from from the [l0-h0] to [l1-h1] range
float remap(float val, float l0, float h0, float l1, float h1)
{
    float result = l1 + (val - l0) * (h1 - l1) / (h0 - l0);
    return result;
}

CloudGenerator::CloudGenerator(uint32_t width, uint32_t height, uint32_t depth, float randomSeed):
    m_worleyGenerator(glm::ivec3(4, 4, 4)),
    m_randomSeed(randomSeed),
    m_fbmLevels(3),
    m_width(width),
    m_height(height),
    m_depth(depth)
{

}

/* --------------------------------- Public methods --------------------------------- */

std::vector<unsigned char> CloudGenerator::compute3DTexture(float noiseScale)
{
    const uint32_t texMemSize = m_width * m_height * m_depth;
    auto result = std::vector<unsigned char>(texMemSize);
    BrownianNoise3D noiseGenerator = BrownianNoise3D(glm::ivec3(32, 32, 32), 5, m_randomSeed);

    glm::vec3 pixelPos;
    float noiseValue, worleyValue, detailValue;
    float worleyScale = noiseScale / 2.0f;
    float detailScale = noiseScale * 6.0f;

    float fullScale = noiseScale;
    float firstScale = noiseScale * 2.0f;
    float secondScale = noiseScale * 4.0f;
    float thirdScale = noiseScale * 8.0f;

    for (uint32_t z = 0; z < m_depth; z++) {
        for (uint32_t y = 0; y < m_height; y++) {
            for (uint32_t x = 0; x < m_width; x++) {
                /*pixelPos = glm::vec3(x, y, z); // / 64.0f;
                noiseValue = noiseGenerator.evaluate(pixelPos * noiseScale);
                detailValue = noiseGenerator.evaluate(pixelPos * detailScale);
                pixelPos = glm::vec3(x / m_width, y / m_height, z / m_depth); // / 64.0f;
                //data[x + y * imageInfo.textureSize.width + z * imageInfo.textureSize.width * imageInfo.textureSize.height] = noiseValue;
                worleyValue = worleyGenerator.evaluate(pixelPos, worleyScale);
                //worleyValue *= worleyValue;
                worleyValue = 2.0f * noiseValue * worleyValue;
                //worleyValue += worleyValue * noiseValue;
                worleyValue = 2.0f * worleyValue * (1.1f - detailValue);
                worleyValue = glm::min(worleyValue, 1.0f);
                worleyValue *= 255.0f;
                result[x + y * m_width + z * m_width * m_height] = worleyValue;*/

                pixelPos = glm::vec3(x, y, z);
                float value = noiseGenerator.evaluate(pixelPos * fullScale);
                float fbm = computeFBM(pixelPos, firstScale);
                value = remap(value, fbm - 1.0f, 1.0f, 0.0f, 1.0f) * 255.0f;
                result[x + y * m_width + z * m_width * m_height] = value;
            }
        }
    }

    return result;
}

/* --------------------------------- Private methods --------------------------------- */

float CloudGenerator::computeFBM(const glm::vec3& pixelPos, float scale)
{
    float result = 0.0f;
    float amplitude = 1.0f;
    float scaleFactor = 1.0f;
    float max = 0.0f;
    for (uint32_t level = 0; level < m_fbmLevels; level++)
    {
        result += amplitude * m_worleyGenerator.evaluate(pixelPos, scale * scaleFactor);
        max += amplitude;
        scaleFactor *= 2.0f;
        amplitude /= 2.0f;
    }

    result /= max;
    return result;
}