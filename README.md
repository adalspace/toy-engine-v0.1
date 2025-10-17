

# Project Description

This is a basic future game engine for OpenGL 3D rendered games

## Building on Windows

In order to configure and run project on windows platform accomplish several steps.

### Configuring

```console
cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
```

### Building

```console
cmake --build build --config Release
```

### Static Linking

For static linking you just need to modify the configure command as follows:

```console
cmake -S . -B build -G "Visual Studio 17 2022"-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_BUILD_TYPE=Release -DENGINE_BUILD_SHARED=OFF
```

## Multi-GPU Devices

If you want to use non-primary GPU on your device when launching the game specifically on Linux you should specify additional environment variables before running. For example in my case I have a hybrid gaming laptop with 2 GPUs AMD from CPU and NVIDIA discrete.

The run command in that case would look following:

```console
__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia <executable_path>
```
