file(GLOB SAMPLE_IMGUI_H *.h)
file(GLOB SAMPLE_IMGUI_CPP *.cpp)

add_executable("sample_imgui" ${SAMPLE_IMGUI_H} ${SAMPLE_IMGUI_CPP}) 
target_link_libraries(sample_imgui Common)
