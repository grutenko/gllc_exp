set(CGLM_STATIC ON)
add_subdirectory(cglm)

add_compile_options(
    -Wno-array-bounds
)

add_subdirectory(LibreDWG)