#include "ViewParams.h"

/* --------------------------------- Constructors --------------------------------- */

ViewParams::ViewParams():
    m_lightPosition(0.0f, 0.82f, 1.2f),
    m_fogScale(1.05f),
    m_noiseSize(2.37f),
    m_randomSeed(42.0f),
    m_speed(0.05f),
    m_lightAbsorption(0.052f),
    m_densityTreshold(0.343f),
    m_lightColor(1.0f, 1.0f, 0.8, 1.0f),
    m_inScatering(0.652f),
    m_outScatering(0.5f),
    m_phaseFactor(0.519f),
    m_phaseOffset(0.663f),
    m_fogScaleChanged(false),
    m_noiseSizeChanged(false),
    m_randomSeedChanged(false),
    m_speedChanged(false),
    m_lightAbsorptionChanged(false),
    m_densityTresholdChanged(false),
    m_lightColorChanged(false),
    m_inScateringChanged(false),
    m_outScateringChanged(false),
    m_phaseFactorChanged(false),
    m_phaseOffsetChanged(false)
{

}

void ViewParams::update(float fogScale, float noiseSize, float randomSeed, float speed, float lightAbsorption, float densityTreshold, 
        const glm::vec4& lightColor, float inScatering, float outScatering, float phaseFactor, float phaseOffset)
{
    m_fogScaleChanged = false;
    m_noiseSizeChanged = false;
    m_randomSeedChanged = false;
    m_speedChanged = false;
    m_lightAbsorptionChanged = false;
    m_densityTresholdChanged = false;
    m_lightColorChanged = false;
    m_inScateringChanged = false;
    m_outScateringChanged = false;
    m_phaseFactorChanged = false;
    m_phaseOffsetChanged = false;

    if (m_fogScale != fogScale) {
        m_fogScale = fogScale;
        m_fogScaleChanged = true;
    }

    if (m_noiseSize != noiseSize) {
        m_noiseSize = noiseSize;
        m_noiseSizeChanged = true;
    }

    if (m_randomSeed != randomSeed) {
        m_randomSeed = randomSeed;
        m_randomSeedChanged = true;
    }

    if (m_speed != speed) {
        m_speed = speed;
        m_speedChanged = true;
    }

    if (m_lightAbsorption != lightAbsorption) {
        m_lightAbsorption = lightAbsorption;
        m_lightAbsorptionChanged = true;
    }

    if (m_densityTreshold != densityTreshold) {
        m_densityTreshold = densityTreshold;
        m_densityTresholdChanged = true;
    }

    if (m_lightColor != lightColor) {
        m_lightColor = lightColor;
        m_lightColorChanged = true;
    }

    if (m_inScatering != inScatering) {
        m_inScatering = inScatering;
        m_inScateringChanged = true;
    }

    if (m_outScatering != outScatering) {
        m_outScatering = outScatering;
        m_outScateringChanged = true;
    }

    if (m_phaseFactor != phaseFactor) {
        m_phaseFactor = phaseFactor;
        m_phaseFactorChanged = true;
    }

    if (m_phaseOffset != phaseOffset) {
        m_phaseOffset = phaseOffset;
        m_phaseOffsetChanged = true;
    }
}

/* --------------------------------- Public Methods --------------------------------- */

glm::vec3 ViewParams::lightPosition() const
{
    return m_lightPosition;
}

float ViewParams::fogScale() const
{
    return m_fogScale;
}

float ViewParams::noiseSize() const
{
    return m_noiseSize;
}

float ViewParams::randomSeed() const
{
    return m_randomSeed;
}

float ViewParams::speed() const
{
    return m_speed;
}

float ViewParams::lightAbsorption() const
{
    return m_lightAbsorption;
}

float ViewParams::densityTreshold() const
{
    return m_densityTreshold;
}

glm::vec4 ViewParams::lightColor() const
{
    return m_lightColor;
}

float ViewParams::inScatering() const
{
    return m_inScatering;
}

float ViewParams::outScatering() const
{
    return m_outScatering;
}

float ViewParams::phaseFactor() const
{
    return m_phaseFactor;
}

float ViewParams::phaseOffset() const
{
    return m_phaseOffset;
}

bool ViewParams::fogScaleChanged() const
{
    return m_fogScaleChanged;
}

bool ViewParams::noiseSizeChanged() const
{
    return m_noiseSizeChanged;
}

bool ViewParams::randomSeedChanged() const
{
    return m_randomSeedChanged;
}

bool ViewParams::speedChanged() const
{
    return m_speedChanged;
}

bool ViewParams::lightAbsorptionChanged() const
{
    return m_lightAbsorptionChanged;
}

bool ViewParams::densityTreshHoldChanged() const
{
    return m_densityTresholdChanged;
}

bool ViewParams::lightColorChanged() const
{
    return m_lightColorChanged;
}

bool ViewParams::inScateringChanged() const
{
    return m_inScateringChanged;
}

bool ViewParams::outScateringChanged() const
{
    return m_outScateringChanged;
}

bool ViewParams::phaseFactorChanged() const
{
    return m_phaseFactorChanged;
}

bool ViewParams::phaseOffsetChanged() const
{
    return m_phaseOffsetChanged;
}
