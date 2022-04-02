#pragma once

#include <glm/glm.hpp>

class ViewParams
{
public:
    ViewParams();

public:
    void update(float fogScale, float noiseSize);

public:
    float fogScale();
    float noiseSize();

    bool fogScaleChanged();
    bool noiseSizeChanged();

private:
    float m_fogScale;
    float m_noiseSize;

    bool m_fogScaleChanged;
    bool m_noiseSizeChanged;
};
