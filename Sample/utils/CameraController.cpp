#include "CameraController.h"

#include <iostream>
#include <glm/gtc/quaternion.hpp> 

/* --------------------------------- Constructors --------------------------------- */

CameraController::CameraController(Camera* camera, int width, int height):
    m_camera(camera),
    m_width(width),
    m_height(height),
    m_mousePressed(false)
{

}


CameraController::~CameraController()
{
}

/* --------------------------------- Public methods --------------------------------- */

void CameraController::resize(int width, int height)
{
    m_width = width;
    m_height = height;
}

void CameraController::mouseMove(double xpos, double ypos)
{
    if (m_mousePressed) {
        glm::vec3 currentVector = getArcBallVector(glm::vec2(xpos, ypos));
        glm::vec3 rotationVector = glm::cross(currentVector, m_previousVector);

        // Don't know why but i must invert the y rotation axis
        rotationVector.y = -1.0f * rotationVector.y;
        float angle = glm::dot(currentVector, m_previousVector);
        glm::quat rotation = glm::angleAxis(angle, rotationVector);
        m_previousVector = currentVector;

        glm::mat4 arcBallModel = m_camera->arcBallModel();
        arcBallModel = glm::mat4(rotation) * arcBallModel;
        m_camera->setArcBallModel(arcBallModel);
    }
}

void CameraController::mousePress(double xpos, double ypos)
{
    m_mousePressed = true;
    m_previousVector = getArcBallVector(glm::vec2(xpos, ypos));
}

void CameraController::mouseRelease(double xpos, double ypos)
{
    m_mousePressed = false;
}

void CameraController::mouseScroll(double scrollDelta)
{
    constexpr float scrollSensitivity = 10.0f;
    glm::vec3 center = m_camera->center();
    glm::vec3 eye = m_camera->eye();
    glm::vec3 dist = eye - center;
    eye += dist * float(-1.0f * scrollDelta / scrollSensitivity);
    m_camera->setEye(eye);
}

/* --------------------------------- Private methods --------------------------------- */

glm::vec3 CameraController::getArcBallVector(const glm::vec2& mousePos) const
{
    glm::vec3 result;
    result.x = mousePos.x / m_width;
    result.y = mousePos.y / m_height;
    result = result * 2.0f - 1.0f;
    float projectedVecSquared = result.x * result.x + result.y * result.y;
    if (projectedVecSquared <= 1) {
        result.z = glm::sqrt(1.0f - projectedVecSquared);  // Pythagore
    } else {
        result.z = 0;
    }
    result = glm::normalize(result);
    return result;
}