#include "ViewParams.h"

/* --------------------------------- Constructors --------------------------------- */

ViewParams::ViewParams():
    m_fogScale(1.812f),
    m_noiseSize(2.37f),
    m_randomSeed(42.0f),
    m_speed(0.05f),
    m_fogScaleChanged(false),
    m_noiseSizeChanged(false),
    m_randomSeedChanged(false),
    m_speedChanged(false)
{

}

void ViewParams::update(float fogScale, float noiseSize, float randomSeed, float speed)
{
    m_fogScaleChanged = false;
    m_noiseSizeChanged = false;
    m_randomSeedChanged = false;
    m_speedChanged = false;

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
}

/* --------------------------------- Public Methods --------------------------------- */

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
