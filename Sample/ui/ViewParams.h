#pragma once

#include <glm/glm.hpp>

class ViewParams
{
public:
    ViewParams();

public:
    float& fogScale();

private:
    float m_fogScale;
};
