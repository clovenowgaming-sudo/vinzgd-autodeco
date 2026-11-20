cmake_minimum_required(VERSION 3.21)

project(AutoDecoBeatSync VERSION 1.0.0)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Set the GEODE_SDK environment variable to the path of your Geode SDK clone before configuring (see https://docs.geode-sdk.org/getting-started/geode-cli).")
endif()

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/AutoDecoGenerator.cpp
    src/AutoDecoPopup.cpp
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode SDK!")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

target_include_directories(${PROJECT_NAME} PRIVATE src)

setup_geode_mod(${PROJECT_NAME})
