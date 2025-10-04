#ifndef WIN32
#define GLEW_STATIC
#endif

#include "renderer/engine.h"

int main() {
    Engine engine;

    engine.Run();

    return 0;
}