#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <GL/glew.h>
#include <SDL3/SDL.h>

#include "shader.h"
#include "file_manager.h"
#include "prelude.h"
#include "block.h"
#include "vertex.h"
#include "model.h"

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

    SDL_GLContext glcontext = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        fprintf(stderr, "Could not initialize GLEW!\n");
        SDL_GL_DestroyContext(glcontext);
        SDL_DestroyWindow(window);
        exit(1);
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEPTH_TEST);
    glDebugMessageCallback(MessageCallback, 0);

    // brightness multipliers for faces
    const float FACE_BRIGHTNESS[6] = {
        1.0f,  // front
        0.7f,  // right
        0.5f,  // back
        0.7f,  // left
        1.2f,  // top
        0.4f   // bottom
    };

    // position, normal, color
    glm::vec4 cubeColor = {1.0f, 0.5f, 0.31f, 1.0f};
    std::vector<Point> cubeVerts = {
        // front face (z = 0, normal = +Z)
        { {-0.5f, -0.5f, 0.5f}, {0.f, 0.f, 1.f}, cubeColor },
        { {0.5f, -0.5f, 0.5f}, {0.f, 0.f, 1.f}, cubeColor },
        { {0.5f, 0.5f, 0.5f}, {0.f, 0.f, 1.f}, cubeColor },
        { {-0.5f, 0.5f, 0.5f}, {0.f, 0.f, 1.f}, cubeColor },

        // back face (z = 1, normal = -Z)
        { {-0.5f, -0.5f, -0.5f}, {0.f, 0.f, -1.f}, cubeColor },
        { {0.5f, -0.5f, -0.5f}, {0.f, 0.f, -1.f}, cubeColor },
        { {0.5f, 0.5f, -0.5f}, {0.f, 0.f, -1.f}, cubeColor },
        { {-0.5f, 0.5f, -0.5f}, {0.f, 0.f, -1.f}, cubeColor },

        // left face (x = 0, normal = -X)
        { {-0.5f, -0.5f, -0.5f}, {-1.f, 0.f, 0.f}, cubeColor },
        { {-0.5f, -0.5f, 0.5f}, {-1.f, 0.f, 0.f}, cubeColor },
        { {-0.5f, 0.5f, 0.5f}, {-1.f, 0.f, 0.f}, cubeColor },
        { {-0.5f, 0.5f, -0.5f}, {-1.f, 0.f, 0.f}, cubeColor },

        // right face (x = 1, normal = +X)
        { {0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f}, cubeColor },
        { {0.5f, -0.5f, 0.5f}, {1.f, 0.f, 0.f}, cubeColor },
        { {0.5f, 0.5f, 0.5f}, {1.f, 0.f, 0.f}, cubeColor },
        { {0.5f, 0.5f, -0.5f}, {1.f, 0.f, 0.f}, cubeColor },

        // top face (y = 1, normal = +Y)
        { {-0.5f, 0.5f, 0.5f}, {0.f, 1.f, 0.f}, cubeColor },
        { {0.5f, 0.5f, 0.5f}, {0.f, 1.f, 0.f}, cubeColor },
        { {0.5f, 0.5f, -0.5f}, {0.f, 1.f, 0.f}, cubeColor },
        { {-0.5f, 0.5f, -0.5f}, {0.f, 1.f, 0.f}, cubeColor },

        // bottom face (y = 0, normal = -Y)
        { {-0.5f, -0.5f, 0.5f}, {0.f, -1.f, 0.f}, cubeColor },
        { {0.5f, -0.5f, 0.5f}, {0.f, -1.f, 0.f}, cubeColor },
        { {0.5f, -0.5f, -0.5f}, {0.f, -1.f, 0.f}, cubeColor },
        { {-0.5f, -0.5f, -0.5f}, {0.f, -1.f, 0.f}, cubeColor },
    };

    std::vector<unsigned int> cubeIndices = {
        0,1,2,  2,3,0,      // front
        4,5,6,  6,7,4,      // back
        8,9,10, 10,11,8,    // left
        12,13,14, 14,15,12, // right
        16,17,18, 18,19,16, // top
        20,21,22, 22,23,20  // bottom
    };

    Vertices vertices;
    for (auto &v : cubeVerts) vertices.PushVertex(v);
    for (auto i : cubeIndices) vertices.PushIndex(i);
    vertices.Upload();

    Shader simpleShader;
    simpleShader.init(
        FileManager::read("./src/shaders/simple.vs"),
        FileManager::read("./src/shaders/simple.fs")
    );

    int screenWidth = WIDTH, screenHeight = HEIGHT;

    glm::vec3 cameraPosition(0.f, 0.f, 2.f);
    glm::vec3 cameraViewDirection(0.f, 0.f, -1.f);
    // glm::vec3 lightPosition(1.f, 3.5f, -2.f);
    glm::vec3 lightPosition = cameraPosition;

    glm::mat4 model(1.f);

    glm::mat4 view = glm::lookAt(
        cameraPosition,
        cameraPosition + cameraViewDirection,
        glm::vec3(0.f, 1.f, 0.f)
    );

    glm::mat4 projection = glm::perspective(
        (float)M_PI_2,
        (float)screenWidth / (float)screenHeight,
        0.01f,
        100.0f
    );

    float angle = 3.45f;
    Uint64 lastTicks = SDL_GetTicks();

    Object cube = Object::LoadFile("./assets/cube.obj");
    Object monkey = Object::LoadFile("./assets/monkey.obj");

    bool paused = false;

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
                        glViewport(
                            0,
                            0,
                            width,
                            height);
                    }
                    break;
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.key) {
                    case SDLK_SPACE:
                        paused = !paused;
                        break;
                    // case SDLK_F5:
                    //     reload_shaders(&context);
                    //     break;
                    default: break;
                    };
                    break;
                default: break;
            };
        }

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

            model = glm::rotate(
                glm::mat4(1.f),
                angle,
                glm::vec3(0.8f, -0.4f, 0.5f)
            );

            // lightPosition -= glm::vec3(0.05f, 0.f, 0.f) * deltaTime;

            simpleShader.setMat4("u_model", model);
            simpleShader.setMat4("u_view", view);
            simpleShader.setMat4("u_projection", projection);

            simpleShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
            simpleShader.setVec3("lightPos", lightPosition);
            simpleShader.setVec3("viewPos", cameraPosition);
            simpleShader.setFloat("ambientStrength", 0.2f);
            simpleShader.setFloat("specularStrength", 0.5f);

            vertices.Draw();
        }
        
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(glcontext);
    SDL_DestroyWindow(window);

    return 0;
}