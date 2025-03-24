cmake_minimum_required(VERSION 3.10)
project(NCLI)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/single-header)

# Create custom target for generating the single header
add_custom_target(generate_single_header
    COMMAND python3 ../tools/bundle.py
    COMMENT "Generating single header NCLI.h"
)

# Define the executable target without the comma
add_executable(NCLI src/main.cpp)

# Ensure that NCLI depends on generating the single header
add_dependencies(NCLI generate_single_header)

# Set the include directories for NCLI
target_include_directories(NCLI PRIVATE ${CMAKE_SOURCE_DIR}/single-header)
