#pragma once

#include <glm/glm.hpp>


class Camera
{
public:
    Camera(const glm::vec2& screenDimension, const glm::vec3& eye, const glm::vec3& up, const glm::vec3& center, float verticalFov);
    ~Camera();

public:
    void setEye(const glm::vec3& eye);
    void setUp(const glm::vec3& up);
    void setCenter(const glm::vec3& center);
    void setArcBallModel(const glm::mat4& model);

    glm::vec3 eye() const;
    glm::vec3 up() const;
    glm::vec3 center() const;
    glm::mat4 arcBallModel() const;

    void resize(const glm::vec2& screenDimension);
    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;

private:
    glm::vec2 m_screenDimension;

    glm::vec3 m_eye;
    glm::vec3 m_up;
    glm::vec3 m_center;

    float m_verticalFov;
    float m_near;
    float m_far;

    glm::mat4 m_arcBallModel;
};
