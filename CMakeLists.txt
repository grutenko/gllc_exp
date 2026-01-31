cmake_minimum_required(VERSION 3.28)

project(gllc_exp C)

include_directories(include)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(LIBREDWG_STATIC ON CACHE BOOL "" FORCE)

add_subdirectory(3rdparty)
add_subdirectory(tools)

add_subdirectory(libtess2)

set(SHADERS
    shaders/gllc_vertex_shader.vert
    shaders/gllc_fragment_shader.frag
)

set(GENERATED_HEADERS "")

foreach(SHADER ${SHADERS})
    get_filename_component(NAME ${SHADER} NAME_WE)
    set(OUT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.h")

    add_custom_command(
        OUTPUT ${OUT}
        COMMAND gen_shader_h ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} ${OUT}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} gen_shader_h
        COMMENT "Generating header for ${SHADER}"
        VERBATIM
    )

    list(APPEND GENERATED_HEADERS ${OUT})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${GENERATED_HEADERS})

set(FONT_FILES fonts/Simplex.ttf)

foreach(FONT ${FONT_FILES})
    get_filename_component(NAME ${FONT} NAME_WE)
    set(OUT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.h")

    add_custom_command(
        OUTPUT ${OUT}
        COMMAND gen_font_h ${CMAKE_CURRENT_SOURCE_DIR}/${FONT} ${OUT}
        DEPENDS ${FONT} gen_font_h
        COMMENT "Generating header ${OUT} from ${FONT}"
        VERBATIM
    )

    list(APPEND GENERATED_FONT_HEADERS ${OUT})
endforeach()

add_custom_target(Fonts ALL DEPENDS ${GENERATED_FONT_HEADERS})

set(SOURCES glad.c
  gllc_block_entity.c
  gllc_block.c
  gllc_draw_buffer.c
  gllc_drawing.c
  gllc_layer.c
  gllc_object.c
  gllc_polyline.c
  gllc_window_native_win32.c
  gllc_font.c
  gllc_window.c
  gllc_rect.c
  gllc_circle.c
  gllc_window_grid.c
  gllc_window_cursor.c
  gllc_window_selection.c
  gllc_text.c
  gllc_point.c
  ${GENERATED_HEADERS}
  ${GENERATED_FONT_HEADERS}
  stb_truetype.c
  main.c)

add_executable(gllc_exp ${SOURCES})

target_link_libraries(gllc_exp PRIVATE cglm freetype redwg tess2)

target_include_directories(gllc_exp PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if (WIN32)
  target_compile_definitions(gllc_exp PRIVATE
    WIN32
    _WIN32
    _AMD64_
    _CRT_SECURE_NO_WARNINGS
  )
  target_link_libraries(gllc_exp PRIVATE
    opengl32
    gdi32
    user32
    kernel32
  )
endif()