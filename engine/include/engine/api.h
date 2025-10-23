#pragma once

#include <memory>

#ifdef _WIN32
#include <corecrt_math_defines.h>
#endif

#include "engine/app/app.h"
#include "engine/renderer/core.h"

extern Core::IApplication* CreateApplication();

int main() {
    auto engine = Core::Engine::GetInstance();
    engine->Run(std::unique_ptr<Core::IApplication>(CreateApplication()));
    return 0;
}