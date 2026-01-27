#ifndef gllc_window_cursor_h
#define gllc_window_cursor_h

#include "glad.h"

#define CURSOR_LINES_VCOUNT 8
#define CURSOR_BOX_VCOUNT 4
#define CURSOR_VCOUNT (CURSOR_LINES_VCOUNT + CURSOR_BOX_VCOUNT)
#define CURSOR_VBO_LINES_SIZE (sizeof(GLfloat) * 2 * CURSOR_LINES_VCOUNT)
#define CURSOR_VBO_BOX_SIZE (sizeof(GLfloat) * 2 * CURSOR_BOX_VCOUNT)
#define CURSOR_VBO_SIZE (CURSOR_VBO_LINES_SIZE + CURSOR_VBO_BOX_SIZE)
#define CURSOR_BOX_ICOUNT 4
#define CURSOR_EBO_SIZE (sizeof(GLuint) * CURSOR_BOX_ICOUNT)

struct gllc_W_cursor
{
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
};

void gllc_W_cursor_draw(struct gllc_W_cursor *c, GLuint u_color_loc, int x, int y, int width, int height);

#endif