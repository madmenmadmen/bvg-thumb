cmake_minimum_required(VERSION 3.15)
project(BvgThumb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(BvgThumb SHARED
    BvgThumbProvider.cpp
    BvgThumb.cpp
)
target_include_directories(BvgThumb PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bvg/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(BvgThumb PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bvg/lib/bvg.lib
    windowscodecs
    ole32
    uuid
    shlwapi
)
set_target_properties(BvgThumb PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
)
target_sources(BvgThumb PRIVATE BvgThumb.def)
set(BVG_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bvg/bin")
set(TARGET_DIR "$<TARGET_FILE_DIR:BvgThumb>")

add_custom_command(TARGET BvgThumb POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${BVG_BIN_DIR}/zstd.dll"
        "${BVG_BIN_DIR}/z.dll"
        "${BVG_BIN_DIR}/lz4.dll"
        "${TARGET_DIR}"
)