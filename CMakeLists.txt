# NOTE: IMPORTANT! This may fuck up since subdirectories CMakeLists.. not sure how that works?
cmake_minimum_required(VERSION 3.5)

set(PROJ_NAME "test")
set(TARGET_NAME "test-app")

project(${PROJ_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -Ofast")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    "./src"
    "./fungine/include"
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



add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} "${PROJECT_SOURCE_DIR}/fungine/build/libfungine-engine.so")
