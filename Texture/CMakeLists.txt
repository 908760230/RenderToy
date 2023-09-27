file(GLOB SAMPLE_TEXTURE_H *.h)
file(GLOB SAMPLE_TEXTURE_CPP *.cpp)

add_executable(04sample_texture ${SAMPLE_TEXTURE_H} ${SAMPLE_TEXTURE_CPP}) 
target_link_libraries(04sample_texture Common)
