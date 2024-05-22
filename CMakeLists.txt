# NOTE: IMPORTANT! This may fuck up since subdirectories CMakeLists.. not sure how that works?
cmake_minimum_required(VERSION 3.5)

set(PROJ_NAME "test")
set(TARGET_NAME "test-app")

project(${PROJ_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -Ofast")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    "./src"
    "./fungine/src"
    "./fungine/glfw/include"
    "./fungine/glew/include"
    "./fungine/freetype/include"
    "./fungine/assimp/include"
    "./fungine/assimp/build/include"
)

file(
    GLOB SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/NatureRenderingTest.cpp"
    "${PROJECT_SOURCE_DIR}/src/controllers/CameraController.cpp"
)

# This is to be able to call some opengl funcs directly for quick
# shadowmap debugging purposes -> not required if last lines
# of the main loop are commented out
find_package(OpenGL REQUIRED)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${TARGET_NAME} "${PROJECT_SOURCE_DIR}/fungine/build/libfungine-engine.so")
