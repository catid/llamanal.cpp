cmake_minimum_required(VERSION 3.10)
project(analysis LANGUAGES C CXX)
set(TARGET analysis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${TARGET}
    main.cpp
    rate_prompt.cpp
    rate_prompt.hpp
    walk_directory.cpp
    walk_directory.hpp
)

# For command-line argument parsing
find_package(Boost 1.58.0 COMPONENTS program_options REQUIRED)

# Supported file types:
option(ENABLE_CPP_SUPPORT "Enable Clang support for AST parsing of C++ files" ON)

if(ENABLE_CPP_SUPPORT)
    find_package(Clang REQUIRED)

    target_include_directories(${TARGET} PRIVATE ${CLANG_INCLUDE_DIRS})
    target_link_libraries(${TARGET} PRIVATE ${CLANG_LIBRARIES})
    target_compile_definitions(${TARGET} PRIVATE ENABLE_CPP_SUPPORT)
    target_sources(${TARGET} PRIVATE cpp_analysis.cpp cpp_analysis.hpp)
endif()

target_link_libraries(${TARGET} PRIVATE
    common
    llama
    ${CMAKE_THREAD_LIBS_INIT}
    Boost::program_options
)
