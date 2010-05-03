#ifndef INCLUDE_AL_GRAPHICS_DEBUG_H
#define INCLUDE_AL_GRAPHICS_DEBUG_H

#include <stdio.h>

namespace al {
namespace gfx{

int error(const char *pre="", FILE * out=stderr);

} // ::al::gfx
} // ::al

#endif
