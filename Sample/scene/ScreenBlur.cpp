#include "ScreenBlur.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

ScreenBlur::ScreenBlur(Quad& mesh, BlurMaterial& material) :
    SceneObject(),
    m_mesh(mesh),
    m_material(material)
{

}

ScreenBlur::~ScreenBlur()
{

}

/* -------------------------- Public methods -------------------------- */

void ScreenBlur::update(RenderContext& renderContext, Camera& camera, const ViewParams& viewParams, const DescriptorEntry& descriptorEntry)
{

}

Mesh* ScreenBlur::getMesh()
{
    return &m_mesh;
}

Material* ScreenBlur::getMaterial()
{
    return &m_material;
}