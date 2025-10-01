#include <iostream>

// #ifdef WIN32
#define _USE_MATH_DEFINES
#include <cmath>
// #endif

#ifndef WIN32
#define GLEW_STATIC
#endif

#include <vector>
#include <glm/glm.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <GL/glew.h>
#include <SDL3/SDL.h>

#include "renderer/shader.h"
#include "IO/file_manager.h"
#include "renderer/debug.h"
#include "renderer/wavefront.h"

#define WIDTH 1024
#define HEIGHT 768

int main() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    SDL_Window *window = SDL_CreateWindow("OpenGL Test", WIDTH, HEIGHT, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALWAYS_ON_TOP);

    SDL_SetWindowRelativeMouseMode(window, true);

    SDL_GLContext glcontext = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        fprintf(stderr, "Could not initialize GLEW!\n");
        SDL_GL_DestroyContext(glcontext);
        SDL_DestroyWindow(window);
        exit(1);
    }

    std::cout << "GL_VENDOR:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GL_VERSION:  " << glGetString(GL_VERSION) << std::endl;

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEPTH_TEST);
    glDebugMessageCallback(MessageCallback, 0);

    Shader simpleShader;
    simpleShader.init(
        FileManager::read("./src/shaders/simple.vs"),
        FileManager::read("./src/shaders/simple.fs")
    );

    int screenWidth = WIDTH, screenHeight = HEIGHT;

    glm::vec3 cameraPosition(0.f, 0.f, 2.f);
    // glm::vec3 cameraViewDirection(0.f, 0.f, -1.f);
    // glm::vec3 lightPosition(1.f, 3.5f, -2.f);
    glm::vec3 lightPosition(-5.f, 5.f, 5.f);

    glm::mat4 model(1.f);

    glm::mat4 projection = glm::perspective(
        (float)M_PI_2,
        (float)screenWidth / (float)screenHeight,
        0.01f,
        100.0f
    );

    float angle = 3.45f;
    Uint64 lastTicks = SDL_GetTicks();

    // Object teapot = Object::LoadFile("./assets/kastrula/kastrula.obj");
    // Object bricks = Object::LoadFile("./assets/bricks/bricks.obj");
    Object lightSource = Object::LoadFile("./assets/cube.obj");
    Object target = Object::LoadFile("./assets/car/car.obj");

    bool paused = false;

    float yaw   = -90.0f; // looking along -Z initially
    float pitch = 0.0f;   // no vertical tilt

    // FPS tracking
    Uint64 startTicks = SDL_GetTicks();
    int frameCount = 0;

    bool quit = false;
    while (!quit) {
        Uint64 currentTicks = SDL_GetTicks();
        float deltaTime = (currentTicks - lastTicks) / 1000.0f; // seconds

        lastTicks = currentTicks;

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    int width, height;
                    if (SDL_GetWindowSize(window, &width, &height)) {
                        screenWidth = width;
                        screenHeight = height;
                        glViewport(
                            0,
                            0,
                            width,
                            height);
                        projection = glm::perspective(
                            (float)M_PI_2,
                            (float)screenWidth / (float)screenHeight,
                            0.01f,
                            100.0f
                        );
                    }
                    break;
                default: break;
            };
        }

        float mouseXRel, mouseYRel;
        Uint32 mouseState = SDL_GetRelativeMouseState(&mouseXRel, &mouseYRel);

        float sensitivity = 0.1f; // tweak as needed
        yaw   += mouseXRel * sensitivity;
        pitch -= mouseYRel * sensitivity; // invert Y for typical FPS control

        // clamp pitch to avoid flipping
        if (pitch > 89.0f)  pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        // convert to direction vector
        glm::vec3 cameraViewDirection(0.f, 0.f, -1.f);
        cameraViewDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraViewDirection.y = sin(glm::radians(pitch));
        cameraViewDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraViewDirection = glm::normalize(cameraViewDirection);

        glm::vec3 velocity(0.f);

        const bool* state = SDL_GetKeyboardState(nullptr);

        if (state[SDL_SCANCODE_P]) paused = !paused;

        glm::vec3 front = glm::normalize(glm::vec3(cameraViewDirection.x, 0.f, cameraViewDirection.z));
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.f, 1.f, 0.f)));

        if (state[SDL_SCANCODE_W]) velocity += front;
        if (state[SDL_SCANCODE_S]) velocity -= front;
        if (state[SDL_SCANCODE_A]) velocity -= right;
        if (state[SDL_SCANCODE_D]) velocity += right;
        if (state[SDL_SCANCODE_SPACE]) velocity.y += 1.f;
        if (state[SDL_SCANCODE_LSHIFT]) velocity.y -= 1.f;

        cameraPosition += velocity * deltaTime * 2.5f; // speed is e.g. 2.5f

        glm::mat4 view = glm::lookAt(
            cameraPosition,
            cameraPosition + cameraViewDirection,
            glm::vec3(0.f, 1.f, 0.f)
        );

        // update rotation
        if (!paused) {
            angle += glm::radians(45.0f) * deltaTime; // 72° per second
            if (angle > glm::two_pi<float>()) {
                angle -= glm::two_pi<float>(); // keep value small
            }
        }

        // std::cout << "angle = " << angle << std::endl;

        glClearColor(0x18/255.0f, 0x18/255.0f, 0x18/255.0f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Triangle render
        {
            simpleShader.use();

            simpleShader.setMat4("u_view", view);
            simpleShader.setMat4("u_projection", projection);

            simpleShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
            simpleShader.setVec3("lightPos", lightPosition);
            simpleShader.setVec3("viewPos", cameraPosition);

            model = glm::mat4(1.f);
            model = glm::translate(model, lightPosition);

            simpleShader.setMat4("u_model", model);

            lightSource.Render(simpleShader);

            // lightPosition -= glm::vec3(0.05f, 0.f, 0.f) * deltaTime;

            model = glm::rotate(
                glm::mat4(1.f),
                angle,
                glm::vec3(0.f, -0.5f, 0.0f)
            ) * 0.5f;
            
            simpleShader.setMat4("u_model", model);

            target.Render(simpleShader);
        }
        
        SDL_GL_SwapWindow(window);

        frameCount++;
        currentTicks = SDL_GetTicks();
        Uint64 elapsed = currentTicks - startTicks;

        if (elapsed >= 1000) { // one second passed
            double fps = (double)frameCount / (elapsed / 1000.0);
            std::cout << "FPS: " << fps << std::endl;
            frameCount = 0;
            startTicks = currentTicks;
        }
    }

    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);

    return 0;
}