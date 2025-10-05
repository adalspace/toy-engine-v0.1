#include "renderer/debug.h"

#include <iostream>

void MessageCallback(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam)
{
    (void) source;
    (void) id;
    (void) length;
    (void) userParam;
    std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "")
              << " type = 0x" << type
              << ", severity = 0x" << severity
              << ", message = " << message << std::endl;
    // std::cerr << "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
    //         (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
    //         type, severity, message);
}
