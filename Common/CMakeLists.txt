file(GLOB COMMON_H *.h)
file(GLOB COMMON_CPP *.cpp)

set(KTX_SOURCES
    ${KTX_DIR}/lib/texture.c
    ${KTX_DIR}/lib/hashlist.c
    ${KTX_DIR}/lib/checkheader.c
    ${KTX_DIR}/lib/swap.c
    ${KTX_DIR}/lib/memstream.c
    ${KTX_DIR}/lib/filestream.c)

add_library("Common" ${COMMON_H} ${COMMON_CPP} ${KTX_SOURCES}) 
target_link_libraries(Common Vulkan::Vulkan)