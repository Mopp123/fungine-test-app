# NOTE: IMPORTANT! This may fuck up since subdirectories CMakeLists.. not sure how that works?
cmake_minimum_required(VERSION 3.5)

set(PROJ_NAME "test")
set(TARGET_NAME "test-app")

project(${PROJ_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -Ofast")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ADDITIONAL_LINK_FLAGS " ")

set(SRC_FILES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/controllers/*.cpp"
)


if(${BUILD_TARGET} MATCHES "desktop")
    include_directories(
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/fungine/src"
        "${PROJECT_SOURCE_DIR}/fungine/glfw/include"
        "${PROJECT_SOURCE_DIR}/fungine/glew/include"
        "${PROJECT_SOURCE_DIR}/fungine/freetype/include"
        "${PROJECT_SOURCE_DIR}/fungine/assimp/include"
        "${PROJECT_SOURCE_DIR}/fungine/assimp/build/include"
    )

    # This is to be able to call some opengl funcs directly for quick
    # shadowmap debugging purposes -> not required if last lines
    # of the main loop are commented out
    find_package(OpenGL REQUIRED)

elseif(${BUILD_TARGET} MATCHES "web")
    include_directories(
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/fungine/src"
        "${PROJECT_SOURCE_DIR}/fungine/assimp/include"
        "${PROJECT_SOURCE_DIR}/fungine/assimp/build/include"
    )
    # NOTE: Below doesn't work if using add_compile_options() instead! ...for some reason...
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_FREETYPE=1 -fexceptions -sUSE_ZLIB=1")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
    set(HTML_SHELL_LOCATION "${PROJECT_SOURCE_DIR}/shell.html")
    # NOTE: This gets converted to camel case for some reason...
    # NOTE: html_out becomes the PROJ_NAME for some reason... was too afraid to delete it from here since
    # Emscripten recommends using the -o out.html
    set(HTML_OUT "test.html")

    set(ADDITIONAL_LINK_FLAGS "-fexceptions --use-port=contrib.glfw3 -sFULL_ES3 -o ${HTML_OUT} --shell-file ${HTML_SHELL_LOCATION} -sFORCE_FILESYSTEM --use-preload-plugins -sFETCH -sALLOW_MEMORY_GROWTH -l websocket.js")
    # NOTE: Engine source is included as a part of the web build
    # -> specify all engine files here
    list(APPEND SRC_FILES
        ${PROJECT_SOURCE_DIR}/fungine/src/graphics/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/graphics/shaders/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/graphics/opengl/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/graphics/opengl/shaders/*.cpp

        ${PROJECT_SOURCE_DIR}/fungine/src/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/core/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/core/window/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/core/window/input/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/core/window/openglWindow/*.cpp

        ${PROJECT_SOURCE_DIR}/fungine/src/core/resourceManaging/*.cpp

        ${PROJECT_SOURCE_DIR}/fungine/src/components/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/components/common/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/components/guiComponents/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/components/rendering/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/components/rendering/lighting/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/components/rendering/lighting/shadows/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/components/rendering/renderers/*.cpp

        ${PROJECT_SOURCE_DIR}/fungine/src/entities/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/entities/commonEntitiesLib/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/entities/commonEntitiesLib/shapes2D/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/entities/commonEntitiesLib/shapes3D/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/entities/guiEntities/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/entities/terrain/*.cpp

        ${PROJECT_SOURCE_DIR}/fungine/src/utils/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/utils/fontUtils/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/utils/imgUtils/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/utils/modelLoading/*.cpp
        ${PROJECT_SOURCE_DIR}/fungine/src/utils/myMathLib/*.cpp
    )
else()
    message(FATAL_ERROR "No build target specified")
    return()
endif()


file(
    GLOB SOURCE_FILES
    ${SRC_FILES}
)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS ${ADDITIONAL_LINK_FLAGS})

if(${BUILD_TARGET} MATCHES "desktop")
    target_link_libraries(${TARGET_NAME} ${OPENGL_LIBRARIES})
    target_link_libraries(${TARGET_NAME} "${PROJECT_SOURCE_DIR}/fungine/build/libfungine-engine.so")
elseif(${BUILD_TARGET} MATCHES "web")
    target_link_libraries(${TARGET_NAME} "${PROJECT_SOURCE_DIR}/fungine/assimp/lib/libassimp.a")
    #target_link_libraries(${TARGET_NAME} "${PROJECT_SOURCE_DIR}/assimp-5.0.1/lib/libIrrXML.a")
    #target_link_libraries(${TARGET_NAME} "${PROJECT_SOURCE_DIR}/assimp-5.0.1/lib/libzlib.a")
endif()
