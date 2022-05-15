#pragma once

#include <glm/glm.hpp>

class ViewParams
{
public:
    ViewParams();

public:
    void update(float fogScale, float noiseSize, float randomSeed, float speed, float lightAbsorption, float densityTreshHold, 
        const glm::vec4& lightColor, float inScatering, float outScatering, float phaseFactor, float phaseOffset);

public:
    glm::vec3 lightPosition() const;

    float fogScale() const;
    float noiseSize() const;
    float randomSeed() const;
    float speed() const;
    float lightAbsorption() const;
    float densityTreshold() const;
    glm::vec4 lightColor() const;
    float inScatering() const;
    float outScatering() const;
    float phaseFactor() const;
    float phaseOffset() const;

    bool fogScaleChanged() const;
    bool noiseSizeChanged() const;
    bool randomSeedChanged() const;
    bool speedChanged() const;
    bool lightAbsorptionChanged() const;
    bool densityTreshHoldChanged() const;
    bool lightColorChanged() const;
    bool inScateringChanged() const;
    bool outScateringChanged() const;
    bool phaseFactorChanged() const;
    bool phaseOffsetChanged() const;

private:
    glm::vec3 m_lightPosition;

    float m_fogScale;
    float m_noiseSize;
    float m_randomSeed;
    float m_speed;
    float m_lightAbsorption;
    float m_densityTreshold;
    glm::vec4 m_lightColor;
    float m_inScatering;
    float m_outScatering;
    float m_phaseFactor;
    float m_phaseOffset;

    bool m_fogScaleChanged;
    bool m_noiseSizeChanged;
    bool m_randomSeedChanged;
    bool m_speedChanged;
    bool m_lightAbsorptionChanged;
    bool m_densityTresholdChanged;
    bool m_lightColorChanged;
    float m_inScateringChanged;
    float m_outScateringChanged;
    float m_phaseFactorChanged;
    float m_phaseOffsetChanged;
};
