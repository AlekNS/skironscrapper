add_library (bmp STATIC
    bitmap_context.cpp
    bitmap.cpp
    random.cpp
    unique_id_class.cpp
    unique_id.cpp
)

target_include_directories (bmp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
