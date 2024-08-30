cmake_minimum_required(VERSION 3.14)

# include(cmake/prelude.cmake)

project(
    myproject
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

# include(cmake/project-is-top-level.cmake)
# include(cmake/variables.cmake)

# 头文件目录
set(HEAD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)


# 静态库生成的路径
set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)



# 静态库的名字
set(CALC_LIB symbolKiCad8)

# 添加子目录
add_subdirectory(source)
add_subdirectory(sexpr_parser)
add_subdirectory(sqlite3)

add_subdirectory(symbolKiCad8)

# set(CMAKE_BUILD_TYPE Release)
# include_directories(${PROJECT_SOURCE_DIR}/sqlite3)