#include "renderer/engine.h"

#ifdef WIN32
#include <corecrt_math_defines.h>
#endif
#include <GL/glew.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "IO/file_manager.h"
#include "renderer/shader.h"
#include "renderer/wavefront.h"

Engine::Engine() {
    m_window = std::make_unique<Window>();
    m_isRunning = true;
    m_projection = glm::perspective(
        static_cast<float>(M_PI_2),
        static_cast<float>(m_window->GetWidth()) / static_cast<float>(m_window->GetHeight()),
        0.01f,
        100.0f
    );

    m_window->subscribe<WindowResized>([this](const WindowResized& e) {
        HandleWindowResized(e);
    });

    m_window->subscribe<WindowCloseRequested>([this](const WindowCloseRequested& e) {
        Stop();
    });
}

bool Engine::Running() const {
    return m_isRunning && m_window->IsOpen();
}

void Engine::Stop() {
    m_isRunning = false;
}

void Engine::HandleWindowResized(const WindowResized& event) {
    m_projection = glm::perspective(
        static_cast<float>(M_PI_2),
        static_cast<float>(event.w) / static_cast<float>(event.h),
        0.01f,
        100.0f
    );
}


void Engine::Run() {
    Shader simpleShader;
    simpleShader.init(
        FileManager::read("./src/shaders/simple.vs"),
        FileManager::read("./src/shaders/simple.fs")
    );

    glm::vec3 cameraPosition(0.f, 0.f, 2.f);
    // glm::vec3 cameraViewDirection(0.f, 0.f, -1.f);
    // glm::vec3 lightPosition(1.f, 3.5f, -2.f);
    glm::vec3 lightPosition(-5.f, 5.f, 5.f);

    glm::mat4 model(1.f);

    float angle = 3.45f;
    Uint64 lastTicks = SDL_GetTicks();

    Object lightSource = Object::LoadFile("./assets/cube.obj");
    Object target = Object::LoadFile("./assets/monkey.obj");

    bool paused = false;

    float yaw   = -90.0f; // looking along -Z initially
    float pitch = 0.0f;   // no vertical tilt

    // FPS tracking
    Uint64 startTicks = SDL_GetTicks();
    int frameCount = 0;

    while (m_isRunning) {
        m_window->ProcessEvents();

        Uint64 currentTicks = SDL_GetTicks();
        float deltaTime = static_cast<float>(currentTicks - lastTicks) / 1000.0f; // seconds

        lastTicks = currentTicks;

        float mouseXRel, mouseYRel;
        SDL_GetRelativeMouseState(&mouseXRel, &mouseYRel);

        float sensitivity = 0.1f; // tweak as needed
        yaw   += mouseXRel * sensitivity;
        pitch -= mouseYRel * sensitivity; // invert Y for typical FPS control

        // clamp pitch to avoid flipping
        // if (pitch > 89.0f)  pitch = 89.0f;
        // if (pitch < -89.0f) pitch = -89.0f;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

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

        glClearColor(0x18/255.0f, 0x18/255.0f, 0x18/255.0f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Triangle render
        {
            simpleShader.use();

            simpleShader.setMat4("u_view", view);
            simpleShader.setMat4("u_projection", m_projection);

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

        m_window->SwapBuffers();

        frameCount++;
        currentTicks = SDL_GetTicks();
        Uint64 elapsed = currentTicks - startTicks;

        if (elapsed >= 1000) { // one second passed
            double fps = static_cast<double>(frameCount) / (static_cast<double>(elapsed) / 1000.0);
            std::cout << "FPS: " << fps << std::endl;
            frameCount = 0;
            startTicks = currentTicks;
        }
    }
}

Engine::~Engine() {
    Destroy();
}

void Engine::Destroy() const {
    m_window->Destroy();
}

