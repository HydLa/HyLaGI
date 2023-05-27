cmake_minimum_required(VERSION 3.22)

# Project information
project(HyLaGI
    VERSION 0.9.5
    DESCRIPTION "HydLa Guaranteed Implementation"
    LANGUAGES C CXX)

set(SLIM_PACKAGE_NAME "HyLaGI")
set(SLIM_URL "https://github.com/HydLa/HyLaGI")
set(SLIM_BUGREPORT "hydla@ueda.info.waseda.ac.jp")

# ================================

# Global configuration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# ================================

# Find necessary packages
find_package(Mathematica REQUIRED)
find_package(Boost REQUIRED program_options)
find_package(Python3 REQUIRED Development)
find_package(Git REQUIRED)

# ================================

# Include directories
add_subdirectory(src)