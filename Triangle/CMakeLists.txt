file(GLOB SAMPLE_TRIANGLE_H *.h)
file(GLOB SAMPLE_TRIANGLE_CPP *.cpp)

add_executable("sample_triangle" ${SAMPLE_TRIANGLE_H} ${SAMPLE_TRIANGLE_CPP}) 
target_link_libraries(sample_triangle Common)
