

# Project Description

This is a basic future game engine for OpenGL 3D rendered games

## Building on Windows

In order to configure and run project on windows platform use following commands.

Configuring:

```console
cmake -S . -B build `       
   -G "Visual Studio 17 2022" -A x64 `
   -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
   -DVCPKG_TARGET_TRIPLET=x64-windows `
   -DCMAKE_BUILD_TYPE=Debug
```

## Multi-GPU Devices

If you want to use non-primary GPU on your device when launching the game specifically on Linux you should specify additional environment variables before running. For example in my case I have a hybrid gaming laptop with 2 GPUs AMD from CPU and NVIDIA discrete.

The run command in that case would look following:

```console
__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia ./build/CodingGame
```
