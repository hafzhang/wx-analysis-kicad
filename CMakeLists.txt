cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    wx-drawing-tutorial
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

add_subdirectory(source)