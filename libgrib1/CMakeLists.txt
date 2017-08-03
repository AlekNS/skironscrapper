add_library(grib1 STATIC
    bits.c
    conv_float.c
    grib1_write.c
    grib1_unpack.c
)

target_include_directories (grib1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
