cmake_minimum_required(VERSION 3.21)
project(marty_mem CXX)


if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


set(MODULE_ROOT "${CMAKE_CURRENT_LIST_DIR}")

file(GLOB_RECURSE sources "${MODULE_ROOT}/*.cpp")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Sources" FILES ${sources})

file(GLOB_RECURSE headers "${MODULE_ROOT}/*.h")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Headers" FILES ${headers})


add_library(${PROJECT_NAME} ${sources} ${headers})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# target_include_directories(${PROJECT_NAME} PRIVATE ${MODULE_ROOT}/..)
