#include "ViewParams.h"

/* --------------------------------- Constructors --------------------------------- */

ViewParams::ViewParams():
    m_fogScale(1.0f),
    m_noiseSize(3.0f),
    m_fogScaleChanged(false),
    m_noiseSizeChanged(false)
{

}

void ViewParams::update(float fogScale, float noiseSize)
{
    m_fogScaleChanged = false;
    m_noiseSizeChanged = false;

    if (m_fogScale != fogScale) {
        m_fogScale = fogScale;
        m_fogScaleChanged = true;
    }

    if (m_noiseSize != noiseSize) {
        m_noiseSize = noiseSize;
        m_noiseSizeChanged = true;
    }
}

/* --------------------------------- Public Methods --------------------------------- */

float ViewParams::fogScale()
{
    return m_fogScale;
}

float ViewParams::noiseSize()
{
    return m_noiseSize;
}

bool ViewParams::fogScaleChanged()
{
    return m_fogScaleChanged;
}

bool ViewParams::noiseSizeChanged()
{
    return m_noiseSizeChanged;
}
