#pragma once

#include <memory>

#ifdef _WIN32
#include <corecrt_math_defines.h>
#endif

#include "engine/app/app.h"
#include "engine/renderer/core.h"

extern IApplication* CreateApplication();

int main() {
    auto engine = Engine::GetInstance();
    engine->Run(std::unique_ptr<IApplication>(CreateApplication()));
    return 0;
}