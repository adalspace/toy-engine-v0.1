#pragma once

#include <memory>

#ifdef _WIN32
#include <corecrt_math_defines.h>
#endif

#include "engine/app/app.h"
#include "engine/renderer/core.h"

extern Engine::IApplication* CreateApplication();

int main() {
    auto engine = Engine::Engine::GetInstance();
    engine->Run(std::unique_ptr<Engine::IApplication>(CreateApplication()));
    return 0;
}