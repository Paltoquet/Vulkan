#pragma once

#include "Camera.h"

#include <glm/glm.hpp>


class CameraController
{
public:
    CameraController(Camera* camera, int width, int height);
    ~CameraController();

public:
    void resize(int width, int height);

    void mouseMove(double xpos, double ypos);
    void mousePress(double xpos, double ypos);
    void mouseRelease(double xpos, double ypos);
    void mouseScroll(double scrollDelta);

private:
    glm::vec3 getArcBallVector(const glm::vec2& mousePos) const;

private:
    Camera* m_camera;

    int m_width;
    int m_height;
    bool m_mousePressed;
    glm::vec3 m_previousVector;
};

