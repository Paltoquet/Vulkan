#include "Quad.h"


Quad::Quad()
{
    /*m_vertices = {
        {{-0.5f, 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f,  0.0f, 0.5f}, {0.0f, 1.0f, 0.0f} , {1.0f, 0.0f}},
        {{0.5f,  0.0f,  0.5f}, {0.0f, 0.0f, 1.0f } , {1.0f, 1.0f}},
        {{0.5f, 0.0f,  -0.5f}, {1.0f, 1.0f, 1.0f} , {0.0f, 1.0f}}
    };*/

    m_vertices = {
        {{-1.0f,  -1.0f,  0.f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-1.0f,   1.0f,  0.f}, {0.0f, 1.0f, 0.0f} , {1.0f, 0.0f}},
        {{1.0f,    1.0f,  0.f}, {0.0f, 0.0f, 1.0f } , {1.0f, 1.0f}},
        {{1.0f,   -1.0f,  0.f}, {1.0f, 1.0f, 1.0f} , {0.0f, 1.0f}}
    };

    m_indices = {
        0, 2, 1, 0, 3, 2
    };
}

Quad::~Quad()
{

}