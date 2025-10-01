
# Setup VCPKG

```console
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
# install deps for 64-bit Windows
C:\vcpkg\vcpkg install sdl3 glew glm --triplet x64-windows
```

# Configure

```console
cmake -S . -B build `
  -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DCMAKE_BUILD_TYPE=Debug
```

# Build

```console
cmake --build build --config Debug
```
