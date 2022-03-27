#include "ViewParams.h"

/* --------------------------------- Constructors --------------------------------- */

ViewParams::ViewParams():
    m_fogScale(1.0f)
{

}

/* --------------------------------- Public Methods --------------------------------- */

float& ViewParams::fogScale()
{
    return m_fogScale;
}
