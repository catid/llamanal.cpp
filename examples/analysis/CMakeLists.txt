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
    logging.cpp
    logging.hpp
)

# For command-line argument parsing
find_package(Boost 1.58.0 COMPONENTS program_options log log_setup REQUIRED)

# Supported file types:
option(ENABLE_CPP_SUPPORT "Enable Clang support for AST parsing of C++ files" ON)

# Create variables to store libraries, compile definitions, sources, and include directories
set(LINK_LIBS common llama ${CMAKE_THREAD_LIBS_INIT} Boost::program_options Boost::log Boost::log_setup)
set(CPP_DEFINITIONS)
set(CPP_SOURCES)
set(CPP_INCLUDE_DIRS)

if(ENABLE_CPP_SUPPORT)
    find_package(Clang REQUIRED)

    # WORK-AROUND: On Ubuntu, CLANG_LIBRARIES is broken, so manually include it:
    find_library(BACKUP_CLANG_LIBRARY NAMES clang PATHS
        ${CLANG_INSTALL_PREFIX}/lib/
    )

    # Add Clang libraries, definitions, sources, and include directories to the variables
    list(APPEND LINK_LIBS ${CLANG_LIBRARIES} ${BACKUP_CLANG_LIBRARY})
    list(APPEND CPP_DEFINITIONS ENABLE_CPP_SUPPORT)
    list(APPEND CPP_SOURCES cpp_analysis.cpp cpp_analysis.hpp)
    list(APPEND CPP_INCLUDE_DIRS ${CLANG_INCLUDE_DIRS})
endif()

message(STATUS "Linked libraries: ${LINK_LIBS}")
message(STATUS "Compile definitions: ${CPP_DEFINITIONS}")
message(STATUS "Sources: ${CPP_SOURCES}")
message(STATUS "Include directories: ${CPP_INCLUDE_DIRS}")

# Ignore warnings from system headers
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${TARGET} PRIVATE -isystem ${CPP_INCLUDE_DIRS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${TARGET} PRIVATE /external:I ${CPP_INCLUDE_DIRS})
endif()

# Use the variables for linking, defining, sourcing, and including
target_link_libraries(${TARGET} PRIVATE ${LINK_LIBS})
target_compile_definitions(${TARGET} PRIVATE ${CPP_DEFINITIONS})
target_sources(${TARGET} PRIVATE ${CPP_SOURCES})
target_include_directories(${TARGET} PRIVATE ${CPP_INCLUDE_DIRS})
