cmake_minimum_required(VERSION 3.11)
project(CodingGame LANGUAGES C CXX)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Add -ggdb to debug builds
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")

add_executable(CodingGame src/prelude.cpp src/file_manager.cpp src/shader.cpp src/block.cpp src/vertex.cpp src/texture.cpp src/model.cpp src/main.cpp)

set_property(TARGET CodingGame PROPERTY CXX_STANDARD_REQUIRED 17)

file(COPY src/shaders DESTINATION ${CMAKE_BINARY_DIR}/)

target_include_directories(CodingGame PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/contrib)
target_link_libraries(CodingGame PRIVATE SDL3::SDL3 OpenGL::GL GLEW::GLEW glm::glm)