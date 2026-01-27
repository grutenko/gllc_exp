#ifndef gllc_window_selection_h
#define gllc_window_selection_h

#include "glad.h"

#define SEL_VCOUNT 4
#define SEL_VBO_SIZE (sizeof(GLfloat) * 2 * SEL_VCOUNT)
#define SEL_LINES_ICOUNT 4
#define SEL_RECT_ICOUNT 6
#define SEL_ICOUNT (SEL_LINES_ICOUNT * SEL_RECT_ICOUNT)
#define SEL_EBO_SIZE (sizeof(GLuint) * SEL_ICOUNT)

struct gllc_W_selection
{
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
};

void gllc_W_selection_draw(struct gllc_W_selection *sel, GLuint u_color_loc, double x0, double y0, double x1, double y1);

#endif