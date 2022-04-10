#pragma once

#include <glm/glm.hpp>

class ViewParams
{
public:
    ViewParams();

public:
    void update(float fogScale, float noiseSize, float randomSeed, float speed);

public:
    float fogScale() const;
    float noiseSize() const;
    float randomSeed() const;
    float speed() const;

    bool fogScaleChanged() const;
    bool noiseSizeChanged() const;
    bool randomSeedChanged() const;
    bool speedChanged() const;

private:
    float m_fogScale;
    float m_noiseSize;
    float m_randomSeed;
    float m_speed;

    bool m_fogScaleChanged;
    bool m_noiseSizeChanged;
    bool m_randomSeedChanged;
    bool m_speedChanged;
};
