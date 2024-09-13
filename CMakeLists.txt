cmake_minimum_required(VERSION 3.30.0)
project(eis2 VERSION 0.1.0 LANGUAGES C CXX)

set(target_name eis2)
set(package_name dd99)
set(component_name eis2)
# set(version_major 0)
# set(version_minor 1)
# set(version_patch 0)
# set(version_meta "-alpha")

add_library(${target_name} INTERFACE)
add_library(${package_name}::${component_name} ALIAS ${target_name})
# make_version(${target_name} MAJOR ${version_major} MINOR ${version_minor} PATCH ${version_patch} META ${version_meta})
target_compile_features(${target_name} INTERFACE cxx_std_23)
target_include_directories(${target_name} INTERFACE include)


# *********
# * TESTS *
# *********

if(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
    set(DD99_EIS2_ENABLE_TESTING TRUE)
endif()


if(DD99_EIS2_ENABLE_TESTING)
    include(cmake/CompilerWarnings.cmake)
    add_subdirectory(tests)
endif()
