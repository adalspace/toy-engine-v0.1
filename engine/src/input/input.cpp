#include "engine/input/input.h"

namespace Core {

bool* Input::s_keys_state = nullptr;
int Input::s_keys_state_size = 0;

// Engine only function
void Input::Update(Timestep dt) {
    s_keys_state = (bool*)SDL_GetKeyboardState(&s_keys_state_size);
}

glm::vec2 Input::GetRelativeMouse() {
    glm::vec2 mouse;
    SDL_GetRelativeMouseState(&mouse.x, &mouse.y);
    return mouse;
}

bool Input::IsKeyPressed(SDL_Scancode keyCode) {
    assert(keyCode < s_keys_state_size && "Key is out of bounds of the key input state");
    return s_keys_state[keyCode];
}

}
