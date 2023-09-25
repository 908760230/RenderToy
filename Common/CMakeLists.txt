file(GLOB COMMON_H *.h)
file(GLOB COMMON_CPP *.cpp)

add_library("Common" ${COMMON_H} ${COMMON_CPP}) 
target_link_libraries(Common Vulkan::Vulkan)