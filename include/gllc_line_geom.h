#ifndef gllc_line_geom_h
#define gllc_line_geom_h

#include "glad.h"

int gllc_line_geom_make_weight(
    const GLfloat *vertices,
    GLuint vertices_count,
    int closed,
    GLfloat weight,
    GLfloat *out_vertices,
    GLuint *out_indices,
    GLuint *out_vertices_count,
    GLuint *out_indices_count);

#endif