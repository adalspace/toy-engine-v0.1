#ifndef PRELUDE_H_
#define PRELUDE_H_

#ifndef WIN32
#define GLEW_STATIC
#endif
#include <GL/glew.h>

#include "SDL3/SDL.h"

struct RenderContext {
    Uint64 time;
    Uint64 prev_time;
    bool program_failed = false;
    GLuint program = 0;
    GLint resolution_location = 0;
    GLint time_location = 0;
    bool pause = false;
};

bool compile_shader_source(const GLchar *source, GLenum shader_type, GLuint *shader);

bool compile_shader_file(const char *file_path, GLenum shader_type, GLuint *shader);

bool link_program(GLuint vert_shader, GLuint frag_shader, GLuint *program);

void reload_shaders(RenderContext*context);

// void key_callback(SDL_Window* window, int key, int scancode, int action, int mods);

void window_size_callback(SDL_Window* window, int width, int height);

void MessageCallback(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam);

void process_prelude(RenderContext *context);

#endif // PRELUDE_H_