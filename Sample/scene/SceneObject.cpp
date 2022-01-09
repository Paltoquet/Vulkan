#include "SceneObject.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

SceneObject::SceneObject()
{

}

/* -------------------------- Public methods -------------------------- */

glm::mat4 SceneObject::transform() const
{
    return m_transform;
}