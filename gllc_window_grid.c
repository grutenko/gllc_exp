#include "gllc_window_grid.h"
#include "glad.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gllc_W_grid_init(struct gllc_W_grid *grid)
{
        memset(grid, 0, sizeof(struct gllc_W_grid));

        grid->offset.x = 0.0f;
        grid->offset.y = 0.0f;
        grid->gap.x = 100.0f;
        grid->gap.y = 100.0f;
}

void gllc_W_grid_configure(struct gllc_W_grid *grid, struct gllc_W_grid_config *config)
{
        if (config->offset)
                memcpy(&grid->offset, config->offset, sizeof(struct gllc_W_grid_offset));
        if (config->gap)
                memcpy(&grid->gap, config->gap, sizeof(struct gllc_W_grid_gap));
        if (config->color)
                memcpy(grid->color, config->color, sizeof(float) * 4);
}

void gllc_W_grid_draw(struct gllc_W_grid *grid, GLuint u_color_loc, double wx0, double wy0, double wx1, double wy1, double scale)
{
        double gap_x = grid->gap.x;
        double gap_y = grid->gap.y;

        while ((gap_x / scale) > (grid->gap.x / 2))
                gap_x /= 2;
        while ((gap_x / scale) < (grid->gap.x / 2))
                gap_x *= 2;
        while ((gap_y / scale) > (grid->gap.y / 2))
                gap_y /= 2;
        while ((gap_y / scale) < (grid->gap.y / 2))
                gap_y *= 2;

        int count_x = (int)(fabs(wx1 - wx0) / gap_x) + 1;
        int count_y = (int)(fabs(wy1 - wy0) / gap_y) + 1;

        GLuint VBO_size = sizeof(GLfloat) * 4 * (count_x + count_y);

        if (VBO_size > grid->V_size)
        {
                GLfloat *new_V = malloc(VBO_size);
                if (!new_V)
                        return;

                free(grid->V);

                grid->V = new_V;
        }

        if (!grid->VAO)
                glGenVertexArrays(1, &grid->VAO);
        if (!grid->VBO)
                glGenBuffers(1, &grid->VBO);

        glBindVertexArray(grid->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, grid->VBO);

        if (VBO_size > grid->VBO_size)
        {
                glBufferData(GL_ARRAY_BUFFER, VBO_size, NULL, GL_DYNAMIC_DRAW);
        }

        double X = ceil(wx0 / gap_x) * gap_x;
        double Y = ceil(wy0 / gap_y) * gap_y;

        int i;
        int vertex_idx = 0;

        for (i = 0; i < count_x; i++)
        {
                grid->V[vertex_idx++] = (GLfloat)X;
                grid->V[vertex_idx++] = (GLfloat)wy0;
                grid->V[vertex_idx++] = (GLfloat)X;
                grid->V[vertex_idx++] = (GLfloat)wy1;
                X += gap_x;
        }
        for (i = 0; i < count_y; i++)
        {
                grid->V[vertex_idx++] = (GLfloat)wx0;
                grid->V[vertex_idx++] = (GLfloat)Y;
                grid->V[vertex_idx++] = (GLfloat)wx1;
                grid->V[vertex_idx++] = (GLfloat)Y;
                Y += gap_y;
        }

        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBufferSubData(GL_ARRAY_BUFFER, 0, VBO_size, grid->V);

        glUniform4f(u_color_loc, grid->color[0], grid->color[1], grid->color[2], grid->color[3]);

        glDrawArrays(GL_LINES, 0, (count_x + count_y) * 2);

        glBindVertexArray(0);
}

void gllc_W_grid_cleanup(struct gllc_W_grid *grid)
{
        free(grid->V);

        if (grid->VBO)
        {
                glDeleteBuffers(1, &grid->VBO);
        }
}