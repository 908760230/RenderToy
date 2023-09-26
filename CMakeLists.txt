cmake_minimum_required(VERSION 3.10)

project(samples)


find_package(Vulkan REQUIRED)
message(STATUS "Vulkan FOUND = ${Vulkan_FOUND}")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)


include_directories("${PROJECT_SOURCE_DIR}/Common")
include_directories("${PROJECT_SOURCE_DIR}/ThirdParty/glm")
include_directories("${PROJECT_SOURCE_DIR}/ThirdParty/ktx/include")

set(KTX_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/ktx")

add_subdirectory(Common)
add_subdirectory(ImGUI)
add_subdirectory(RenderToy)
add_subdirectory(Text)
add_subdirectory(Texture)
add_subdirectory(Triangle)

