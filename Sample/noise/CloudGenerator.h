#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <noise/WorleyNoise3D.h>
#include <noise/WorleyNoise2D.h>

class CloudGenerator
{
public:
    CloudGenerator(uint32_t width, uint32_t height, uint32_t depth, float randomSeed);
    ~CloudGenerator() = default;

public:
    std::vector<unsigned char> compute3DTexture(float noiseScale);
    void computeWeatherTexture(float noiseScale, float randomSeed);
    float computeFBM(const glm::vec3& pixelPos, float scale);
    float heightProbabilityFunction(float height, float heightMax);
    float heightDensityFunction(float height);
    float darkeningEffect(float val);

private:
    WorleyNoise3D m_worleyGenerator;
    WorleyNoise2D m_weatherGenerator;

    std::vector<float> m_weatherTexture;
    std::vector<char> m_heightAlteringTexture;

    float m_randomSeed;
    float m_cloudDensity;
    uint32_t m_fbmLevels;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_depth;
};

