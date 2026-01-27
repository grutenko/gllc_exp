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
        grid->gap.x = 50.0f;
        grid->gap.y = 50.0f;
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

void gllc_W_grid_draw(struct gllc_W_grid *grid, GLuint u_color_loc, double wx0, double wy0, double wx1, double wy1, double scale, GLfloat *clear_color)
{
        double gap_x, gap_y, X, Y;
        int count_x, count_y, i, vertex_idx;
        GLuint VBO_size;
        float color[4] = {
            grid->color[0] + (clear_color[0] - grid->color[0]) * 0.5f,
            grid->color[1] + (clear_color[1] - grid->color[1]) * 0.5f,
            grid->color[2] + (clear_color[2] - grid->color[2]) * 0.5f,
            1.0f};

        gap_x = grid->gap.x;
        gap_y = grid->gap.y;

        while ((gap_x / scale) > (grid->gap.x / 2))
                gap_x /= 2;
        while ((gap_x / scale) < (grid->gap.x / 2))
                gap_x *= 2;
        while ((gap_y / scale) > (grid->gap.y / 2))
                gap_y /= 2;
        while ((gap_y / scale) < (grid->gap.y / 2))
                gap_y *= 2;

        count_x = (int)(fabs(wx1 - wx0) / gap_x) + 1;
        count_y = (int)(fabs(wy1 - wy0) / gap_y) + 1;

        VBO_size = sizeof(GLfloat) * 4 * (count_x + count_y);

        if (VBO_size > grid->V_size)
        {
                GLfloat *new_V = malloc(VBO_size);
                if (!new_V)
                        return;

                free(grid->V);

                grid->V = new_V;
                grid->V_size = VBO_size;
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
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

                grid->VBO_size = VBO_size;

                printf("MALLOC\n");
        }

        X = ceil(wx0 / gap_x) * gap_x;
        Y = ceil(wy0 / gap_y) * gap_y;

        vertex_idx = 0;

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

        glBufferSubData(GL_ARRAY_BUFFER, 0, VBO_size, grid->V);

        glUniform4f(u_color_loc, color[0], color[1], color[2], color[3]);

        glDrawArrays(GL_LINES, 0, (count_x + count_y) * 2);

        gap_x *= 5;
        gap_y *= 5;
        X = ceil(wx0 / gap_x) * gap_x;
        Y = ceil(wy0 / gap_y) * gap_y;
        count_x = (int)(fabs(wx1 - wx0) / gap_x) + 1;
        count_y = (int)(fabs(wy1 - wy0) / gap_y) + 1;

        VBO_size = sizeof(GLfloat) * 4 * (count_x + count_y);

        vertex_idx = 0;

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