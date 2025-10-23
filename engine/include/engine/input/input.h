#ifndef ENGINE_INPUT_H_
#define ENGINE_INPUT_H_

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "engine/time/timestep.h"

#include "engine/export.h"

namespace Core {

class ENGINE_API Input {
public:
    Input() = delete;
private:
    friend class Engine;
    static void Update(Timestep dt);
public:
    [[nodiscard]] static glm::vec2 GetRelativeMouse();
    [[nodiscard]] static bool IsKeyPressed(SDL_Scancode keyCode);
private:
    static bool* s_keys_state;
    static int s_keys_state_size;
};

}

#endif // ENGINE_INPUT_H_