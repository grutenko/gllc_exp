cmake_minimum_required(VERSION 3.28)

project(gllc_exp C)

include_directories(include)

add_subdirectory(3rdparty/cglm)
add_subdirectory(3rdparty/freetype)
add_subdirectory(3rdparty/GLFW)
add_executable(gllc_exp glad.c  gllc_block_entity.c gllc_block.c gllc_draw_buffer.c gllc_drawing.c gllc_layer.c gllc_object.c gllc_polyline.c gllc_window_native_win32.c gllc_window.c main.c)
target_link_libraries(gllc_exp PRIVATE cglm freetype glfw)