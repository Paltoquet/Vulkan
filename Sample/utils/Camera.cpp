#include "Camera.h"

#include <iostream>
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/string_cast.hpp>

/* --------------------------------- Constructors --------------------------------- */

Camera::Camera(const glm::vec2& screenDimension, const glm::vec3& eye, const glm::vec3& up, const glm::vec3& center, float verticalFov) :
    m_screenDimension(screenDimension),
    m_eye(eye),
    m_up(up),
    m_center(center),
    m_verticalFov(verticalFov),
    m_near(0.1f),
    m_far(10.0f),
    m_arcBallModel(1.0f)
{

}

Camera::~Camera()
{
}

/* --------------------------------- Public methods --------------------------------- */

void Camera::resize(const glm::vec2& screenDimension)
{
    m_screenDimension = screenDimension;
}

glm::mat4 Camera::viewMatrix() const
{
    // Use a right handed coordinate system, we are looking at the z negative direction
    return glm::lookAt(m_eye, m_center, m_up);
}

glm::mat4 Camera::projectionMatrix() const
{
    glm::mat4 projection = glm::perspective(glm::radians(m_verticalFov), m_screenDimension.x / m_screenDimension.y, m_near, m_far);
    // Vulkan NDC space points downward by default everything will get flipped
    // Note glm negate the sign of the z value to pass from right handed to left handed coordinate
    projection[1][1] *= -1.0f;
    return projection;
}

/* --------------------------------- Getter & Setters --------------------------------- */

void Camera::setEye(const glm::vec3& eye)
{
    m_eye = eye;
}

void Camera::setUp(const glm::vec3& up)
{
    m_up = up;
}

void Camera::setCenter(const glm::vec3& center)
{
    m_center = center;
}

void Camera::setArcBallModel(const glm::mat4& model)
{
    m_arcBallModel = model;
}

glm::vec3 Camera::eye() const
{
    return m_eye;
}

glm::vec3 Camera::up() const
{
    return m_up;
}

glm::vec3 Camera::center() const
{
    return m_center;
}

glm::mat4 Camera::arcBallModel() const
{
    return m_arcBallModel;
}