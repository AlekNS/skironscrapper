add_library (wgf3 STATIC
    wgf3.cpp
)

target_link_libraries (wgf3 bmp)

target_include_directories (wgf3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
