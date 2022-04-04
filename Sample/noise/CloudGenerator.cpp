#include "CloudGenerator.h"

#include <noise/BrownianNoise3D.h>
#include <noise/WorleyNoise3D.h>


CloudGenerator::CloudGenerator(uint32_t width, uint32_t height, uint32_t depth):
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
    BrownianNoise3D noiseGenerator = BrownianNoise3D(glm::ivec3(32, 32, 32), 5);
    WorleyNoise3D worleyGenerator = WorleyNoise3D(glm::ivec3(16, 16, 16));

    glm::vec3 pixelPos;
    float noiseValue, worleyValue, detailValue;
    float worleyScale = noiseScale / 2.0f;
    float detailScale = noiseScale * 6.0f;

    for (uint32_t z = 0; z < m_depth; z++) {
        for (uint32_t y = 0; y < m_height; y++) {
            for (uint32_t x = 0; x < m_width; x++) {
                pixelPos = glm::vec3(x, y, z); // / 64.0f;
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
                result[x + y * m_width + z * m_width * m_height] = worleyValue;
            }
        }
    }

    return result;
}

/* --------------------------------- Private methods --------------------------------- */
