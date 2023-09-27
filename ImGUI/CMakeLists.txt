file(GLOB SAMPLE_IMGUI_H *.h)
file(GLOB SAMPLE_IMGUI_CPP *.cpp)

add_executable(02sample_imgui ${SAMPLE_IMGUI_H} ${SAMPLE_IMGUI_CPP}) 
target_link_libraries(02sample_imgui Common)
