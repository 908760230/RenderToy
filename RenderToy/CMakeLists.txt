file(GLOB SAMPLE_VIKING_H *.h)
file(GLOB SAMPLE_VIKING_CPP *.cpp)

add_executable(03sample_viking ${SAMPLE_VIKING_H} ${SAMPLE_VIKING_CPP}) 
target_link_libraries(03sample_viking Common)