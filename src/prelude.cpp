#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "prelude.h"

#define GLEW_STATIC
#include <GL/glew.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

void panic_errno(const char *fmt, ...)
{
    fprintf(stderr, "ERROR: ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, ": %s\n", strerror(errno));

    exit(1);
}

char *slurp_file(const char *file_path)
{
#define SLURP_FILE_PANIC panic_errno("Could not read file `%s`", file_path)
    FILE *f = fopen(file_path, "r");
    if (f == NULL) SLURP_FILE_PANIC;
    if (fseek(f, 0, SEEK_END) < 0) SLURP_FILE_PANIC;

    long size = ftell(f);
    if (size < 0) SLURP_FILE_PANIC;

    char *buffer = (char*)malloc(size + 1);
    if (buffer == NULL) SLURP_FILE_PANIC;

    if (fseek(f, 0, SEEK_SET) < 0) SLURP_FILE_PANIC;

    fread(buffer, 1, size, f);
    if (ferror(f) < 0) SLURP_FILE_PANIC;

    buffer[size] = '\0';

    if (fclose(f) < 0) SLURP_FILE_PANIC;

    return buffer;
#undef SLURP_FILE_PANIC
}

bool compile_shader_source(const GLchar *source, GLenum shader_type, GLuint *shader)
{
    *shader = glCreateShader(shader_type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);

    GLint compiled = 0;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLchar message[1024];
        GLsizei message_size = 0;
        glGetShaderInfoLog(*shader, sizeof(message), &message_size, message);
        fprintf(stderr, "%.*s\n", message_size, message);
        return false;
    }

    return true;
}

bool compile_shader_file(const char *file_path, GLenum shader_type, GLuint *shader)
{
    char *source = slurp_file(file_path);
    bool err = compile_shader_source(source, shader_type, shader);
    free(source);
    return err;
}

bool link_program(GLuint vert_shader, GLuint frag_shader, GLuint *program)
{
    *program = glCreateProgram();

    glAttachShader(*program, vert_shader);
    glAttachShader(*program, frag_shader);
    glLinkProgram(*program);

    GLint linked = 0;
    glGetProgramiv(*program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLsizei message_size = 0;
        GLchar message[1024];

        glGetProgramInfoLog(*program, sizeof(message), &message_size, message);
        fprintf(stderr, "Program Linking: %.*s\n", message_size, message);
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return program;
}

void reload_shaders(RenderContext* context)
{
    glDeleteProgram(context->program);

    context->program_failed = false;
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLuint vert = 0;
    if (!compile_shader_file("./main.vert", GL_VERTEX_SHADER, &vert)) {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        context->program_failed = true;
        return;
    }

    GLuint frag = 0;
    if (!compile_shader_file("./main.frag", GL_FRAGMENT_SHADER, &frag)) {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        context->program_failed = true;
        return;
    }

    if (!link_program(vert, frag, &context->program)) {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        context->program_failed = true;
        return;
    }

    glUseProgram(context->program);

    context->resolution_location = glGetUniformLocation(context->program, "resolution");
    context->time_location = glGetUniformLocation(context->program, "time");

    printf("Successfully Reload the Shaders\n");
}

void window_size_callback(SDL_Window* window, int width, int height)
{
    (void) window;
    glViewport(
        width / 2 - SCREEN_WIDTH / 2,
        height / 2 - SCREEN_HEIGHT / 2,
        SCREEN_WIDTH,
        SCREEN_HEIGHT);
}

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
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

void process_prelude(RenderContext *context)
{
    // glfwSetKeyCallback(window, key_callback);
    // glfwSetFramebufferSizeCallback(window, window_size_callback);

    glClear(GL_COLOR_BUFFER_BIT);

    if (!context->program_failed) {
        glUniform2f(context->resolution_location,
                    SCREEN_WIDTH,
                    SCREEN_HEIGHT);
        glUniform1f(context->time_location, context->time);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    int64_t cur_time = SDL_GetTicks();
    if (!context->pause) {
        context->time += cur_time - context->prev_time;
    }
    context->prev_time = cur_time;
}
