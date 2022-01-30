#include "QuadTexture.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

QuadTexture::QuadTexture(Quad& mesh, TextureMaterial& material) :
    SceneObject(),
    m_mesh(mesh),
    m_material(material)
{

}

QuadTexture::~QuadTexture()
{

}

/* -------------------------- Public methods -------------------------- */

void QuadTexture::update(RenderContext& renderContext, Camera& camera, const DescriptorEntry& descriptorEntry)
{

}

Mesh* QuadTexture::getMesh()
{
    return &m_mesh;
}

Material* QuadTexture::getMaterial()
{
    return &m_material;
}