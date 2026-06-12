cmake_minimum_required(VERSION 4.3)

project(large_address_aware_patcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY
    "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wcast-align
    -Wconversion
    -Wdouble-promotion
    -Wformat=2
    -Wnull-dereference
)

include_directories(include)

add_executable(large_address_aware_patcher WIN32
    src/main.cpp
    src/patcher.cpp
    src/util.cpp
    src/gui.cpp
)

set_target_properties(large_address_aware_patcher PROPERTIES
    OUTPUT_NAME "Large-Address-Aware-Patcher"
)

target_compile_definitions(large_address_aware_patcher PRIVATE
    UNICODE
    _UNICODE
)

target_link_libraries(large_address_aware_patcher PRIVATE
    shell32
    comdlg32
    comctl32
)

install(TARGETS large_address_aware_patcher
    RUNTIME DESTINATION bin
)