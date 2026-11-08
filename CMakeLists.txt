cmake_minimum_required(VERSION 3.20)
project(MokaLab LANGUAGES C CXX)

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)
find_package(Freetype REQUIRED)

find_package(fmt CONFIG REQUIRED)

# Bunlar büyük ihtimalle her sistemde düzgün bulunmayabilir
find_package(tinygltf CONFIG QUIET)
find_package(yaml-cpp CONFIG REQUIRED)
# find_package(OpenMesh REQUIRED)



add_library(glad STATIC
    external/glad/src/gl.c
)
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)




add_library(stb INTERFACE)

target_include_directories(stb INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
)




add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

target_link_libraries(imgui PUBLIC
    glfw
    OpenGL::GL
)




# set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
# 
# add_subdirectory(external/yaml-cpp)




add_library(imguizmo STATIC
    external/imguizmo/ImGuizmo.cpp
    external/imguizmo/ImGradient.cpp
    external/imguizmo/GraphEditor.cpp
    external/imguizmo/ImCurveEdit.cpp
    external/imguizmo/ImSequencer.cpp
)

target_include_directories(imguizmo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imguizmo
)

target_link_libraries(imguizmo PUBLIC imgui)








file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(MokaLab
    ${PROJECT_SOURCES}
)

target_include_directories(MokaLab PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(MokaLab PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)

target_link_libraries(MokaLab PRIVATE
    OpenGL::GL
    glfw
    glad
    glm::glm
    assimp::assimp
    Freetype::Freetype
    stb
    fmt::fmt
    imgui
    yaml-cpp
    imguizmo
)


if(tinygltf_FOUND)
    target_link_libraries(MokaLab PRIVATE tinygltf::tinygltf)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(MokaLab PRIVATE dl pthread)
endif()

# if(OpenMesh_FOUND)
#     target_link_libraries(MokaLab PRIVATE
#         OpenMeshCore
#         OpenMeshTools
#     )
# endif()
