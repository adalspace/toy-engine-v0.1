#ifndef RENDERER_DEBUG_
#define RENDERER_DEBUG_

#include <GL/glew.h>

namespace Engine {

void MessageCallback(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam);

}

#endif // RENDERER_DEBUG_