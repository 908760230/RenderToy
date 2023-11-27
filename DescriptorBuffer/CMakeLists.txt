file(GLOB SAMPLE_DESCRIPTOR_BUFFER_H *.h)
file(GLOB SAMPLE_DESCRIPTOR_BUFFER_CPP *.cpp)

add_executable(06descriptor_buffer ${SAMPLE_DESCRIPTOR_BUFFER_H} ${SAMPLE_DESCRIPTOR_BUFFER_CPP}) 
target_link_libraries(06descriptor_buffer Common)