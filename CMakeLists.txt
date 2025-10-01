cmake_minimum_required(VERSION 3.16)
project(CodingGame LANGUAGES C CXX)

if (UNIX)
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
elseif (MSVC) # vcpkg
	find_package(SDL3 CONFIG REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(GLEW CONFIG REQUIRED)
	find_package(glm CONFIG REQUIRED)
endif()

# Add -ggdb to debug builds
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")

add_executable(CodingGame
	src/IO/parser.cpp
	src/IO/file_manager.cpp

	src/renderer/debug.cpp
	src/renderer/basics.cpp
	src/renderer/mesh.cpp
	src/renderer/shader.cpp 
	src/renderer/texture.cpp 
	src/renderer/wavefront.cpp 
	
	src/main.cpp
)

set_property(TARGET CodingGame PROPERTY CXX_STANDARD 17)
set_property(TARGET CodingGame PROPERTY CXX_STANDARD_REQUIRED ON)

file(COPY src/shaders DESTINATION ${CMAKE_BINARY_DIR}/)

target_include_directories(CodingGame PRIVATE
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/contrib
)
target_link_libraries(CodingGame PRIVATE
	SDL3::SDL3
	OpenGL::GL
	GLEW::GLEW
	glm::glm
)

# Debug flags
if (MSVC)
	target_compile_options(CodingGame PRIVATE $<$<CONFIG:Debug>:/Zi>)
	target_link_options(CodingGame PRIVATE $<$<CONFIG:Debug>:/DEBUG:FULL>)
else()
	target_compile_options(CodingGame PRIVATE $<$<CONFIG:Debug>:-ggdb>)
endif()

if (WIN32)
	add_custom_command(TARGET CodingGame POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_RUNTIME_DLLS:CodingGame> $<TARGET_FILE_DIR:CodingGame>
		COMMAND_EXPAND_LISTS)
endif()