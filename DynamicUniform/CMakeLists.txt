file(GLOB SAMPLE_DYNAMIC_UNIFORM_BUFFER_H *.h)
file(GLOB SAMPLE_DYNAMIC_UNIFORM_BUFFER_CPP *.cpp)

add_executable(05dynamic_uniform_buffer ${SAMPLE_DYNAMIC_UNIFORM_BUFFER_H} ${SAMPLE_DYNAMIC_UNIFORM_BUFFER_CPP}) 
target_link_libraries(05dynamic_uniform_buffer  Common)