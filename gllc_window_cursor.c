#include "gllc_window_cursor.h"
#include "glad.h"

void gllc_W_cursor_draw(struct gllc_W_cursor *c, GLuint u_color_loc, int x, int y, int width, int height)
{
        if (!c->VAO)
        {
                glGenVertexArrays(1, &c->VAO);
        }

        glBindVertexArray(c->VAO);

        if (!c->VBO)
        {
                glGenBuffers(1, &c->VBO);
                glBindBuffer(GL_ARRAY_BUFFER, c->VBO);
                glBufferData(GL_ARRAY_BUFFER, CURSOR_VBO_SIZE, NULL, GL_STREAM_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        else
        {
                glBindBuffer(GL_ARRAY_BUFFER, c->VBO);
        }

        if (!c->EBO)
        {
                glGenBuffers(1, &c->EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c->EBO);
                GLuint I[] = {
                    CURSOR_LINES_VCOUNT,
                    1 + CURSOR_LINES_VCOUNT,
                    2 + CURSOR_LINES_VCOUNT,
                    3 + CURSOR_LINES_VCOUNT};
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, CURSOR_EBO_SIZE, I, GL_STATIC_DRAW);
        }
        else
        {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c->EBO);
        }

        GLfloat V[] = {
            (GLfloat)x, 0.0f,
            (GLfloat)x, (GLfloat)y - 4.5f,
            (GLfloat)x, (GLfloat)y + 4.5f,
            (GLfloat)x, (GLfloat)height,
            0.0f, (GLfloat)y,
            (GLfloat)x - 4.5f, (GLfloat)y,
            (GLfloat)x + 4.5f, (GLfloat)y,
            (GLfloat)width, (GLfloat)y,
            (GLfloat)x - 4.5f, (GLfloat)y - 4.5f,
            (GLfloat)x + 4.5f, (GLfloat)y - 4.5f,
            (GLfloat)x + 4.5f, (GLfloat)y + 4.5f,
            (GLfloat)x - 4.5f, (GLfloat)y + 4.5f};

        glBufferSubData(GL_ARRAY_BUFFER, 0, CURSOR_VBO_SIZE, V);

        glUniform4f(u_color_loc, 0.0f, 0.0f, 0.0f, 1.0f);

        glDrawArrays(GL_LINES, 0, CURSOR_LINES_VCOUNT * 2);
        glDrawElements(GL_LINE_LOOP, CURSOR_BOX_VCOUNT, GL_UNSIGNED_INT, 0);
}