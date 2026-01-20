cmake_minimum_required(VERSION 3.28)

project(gllc_exp C)

add_subdirectory(3rdparty/cglm)
add_subdirectory(3rdparty/freetype)
add_subdirectory(3rdparty/GLFW)
add_executable(gllc_exp glad.c main.c)
target_link_libraries(gllc_exp PRIVATE cglm freetype glfw)