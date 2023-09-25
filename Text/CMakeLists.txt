file(GLOB SAMPLE_TEXT_H *.h)
file(GLOB SAMPLE_TEXT_CPP *.cpp)

add_executable("sample_text" ${SAMPLE_TEXT_H} ${SAMPLE_TEXT_CPP}) 
target_link_libraries(sample_text Common)
