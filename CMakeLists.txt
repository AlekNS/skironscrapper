cmake_minimum_required(VERSION 3.5)
project(windyconv)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory (libbmp)
add_subdirectory (libwgf3)
add_subdirectory (libgrib1)

set(SOURCE_FILES main.cpp
        converter/formats/FormatReaderAdapter.cpp
        converter/formats/FormatReaderAdapter.h
        converter/formats/Grib1FormatReaderAdapter.cpp
        converter/formats/Grib1FormatReaderAdapter.h
        converter/OutputWriter.cpp
        converter/OutputWriter.h
        )

add_executable(windyconv ${SOURCE_FILES})

include_directories(. libwgf3 libgrib1)

target_link_libraries (
    windyconv
        grib1
        wgf3
)
