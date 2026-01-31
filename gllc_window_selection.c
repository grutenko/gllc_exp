#include "gllc_window_selection.h"
#include "glad.h"

void gllc_W_selection_draw(struct gllc_W_selection *sel, GLuint u_color_loc, double x0, double y0, double x1, double y1)
{
        if (!sel->VAO)
                glGenVertexArrays(1, &sel->VAO);

        glBindVertexArray(sel->VAO);

        if (!sel->VBO)
        {
                glGenBuffers(1, &sel->VBO);
                glBindBuffer(GL_ARRAY_BUFFER, sel->VBO);
                glBufferData(GL_ARRAY_BUFFER, SEL_VBO_SIZE, NULL, GL_STREAM_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        else
        {
                glBindBuffer(GL_ARRAY_BUFFER, sel->VBO);
        }

        if (!sel->EBO)
        {
                glGenBuffers(1, &sel->EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sel->EBO);
                GLuint I[] = {
                    0,
                    1,
                    2,
                    3,
                    0,
                    1,
                    2,
                    2,
                    3,
                    0,
                };
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, SEL_EBO_SIZE, I, GL_STATIC_DRAW);
        }
        else
        {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sel->EBO);
        }

        int vcount = 0;

        GLfloat V[] = {
            (GLfloat)x0 - 0.5f, (GLfloat)y0 - 0.5f,
            (GLfloat)x1 + 0.5f, (GLfloat)y0 - 0.5f,
            (GLfloat)x1 + 0.5f, (GLfloat)y1 + 0.5f,
            (GLfloat)x0 - 0.5f, (GLfloat)y1 + 0.5f};

        glBufferSubData(GL_ARRAY_BUFFER, 0, SEL_VBO_SIZE, V);

        glUniform4f(u_color_loc, 0.0f, 0.0f, 0.0f, 1.0f);
        glDrawElements(GL_LINE_LOOP, SEL_LINES_ICOUNT, GL_UNSIGNED_INT, 0);

        glUniform4f(u_color_loc, 0.0f, 0.0f, 0.0f, 0.3f);
        glDrawElements(GL_TRIANGLES, SEL_RECT_ICOUNT, GL_UNSIGNED_INT, (void *)(sizeof(GLuint) * SEL_LINES_ICOUNT));
}