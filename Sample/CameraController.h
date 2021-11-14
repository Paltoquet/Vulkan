#pragma once

#include <glm/glm.hpp>


class CameraController
{
public:
    CameraController();
    ~CameraController();

public:
    void mouseMove(double xpos, double ypos);
    void mousePress(double xpos, double ypos);
    void mouseRelease(double xpos, double ypos);

private:
    glm::vec3 getArcBallVector(const glm::vec2& mousePos) const;

private:
    int m_width;
    int m_height;
    bool m_mousePressed;
    glm::vec3 m_previousVector;
};

