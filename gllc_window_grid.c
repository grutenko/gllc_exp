#include "gllc_window_grid.h"
#include "glad.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

void gllc_W_grid_init(struct gllc_W_grid *grid)
{
        memset(grid, 0, sizeof(struct gllc_W_grid));

        grid->offset_x = 0.0f;
        grid->offset_y = 0.0f;
        grid->gap_x = 50.0f;
        grid->gap_y = 50.0f;
        grid->scale = 1.0f;
}

void gllc_W_grid_configure(struct gllc_W_grid *grid, struct gllc_W_grid_config *config)
{
        if (config->offset)
        {
                grid->offset_x = config->offset->x;
                grid->offset_y = config->offset->y;
        }
        if (config->gap)
        {
                grid->gap_x = config->gap->x;
                grid->gap_y = config->gap->y;
        }
        if (config->viewport)
        {
                grid->wx0 = config->viewport->x0;
                grid->wy0 = config->viewport->y0;
                grid->wx1 = config->viewport->x1;
                grid->wy1 = config->viewport->y1;
                grid->scale = config->viewport->scale;
        }
        if (config->color)
        {
                memcpy(grid->color, config->color, sizeof(float) * 4);
        }
        if (config->clear_color)
        {
                memcpy(grid->clear_color, config->clear_color, sizeof(float) * 4);
        }

        grid->modified = 1;
}

static void render(struct gllc_W_grid *grid)
{
        double gap_x, gap_y, X, Y;
        int count_x0, count_y0, count_x1, count_y1, i, vertex_idx;
        GLuint VBO_size;

        gap_x = grid->gap_x;
        gap_y = grid->gap_y;

        while ((gap_x / grid->scale) > (grid->gap_x / 2))
                gap_x /= 2;
        while ((gap_x / grid->scale) < (grid->gap_x / 2))
                gap_x *= 2;
        while ((gap_y / grid->scale) > (grid->gap_y / 2))
                gap_y /= 2;
        while ((gap_y / grid->scale) < (grid->gap_y / 2))
                gap_y *= 2;

        count_x0 = (int)(fabs(grid->wx1 - grid->wx0) / gap_x) + 1;
        count_y0 = (int)(fabs(grid->wy1 - grid->wy0) / gap_y) + 1;
        count_x1 = (int)(fabs(grid->wx1 - grid->wx0) / (gap_x * 10.0)) + 1;
        count_y1 = (int)(fabs(grid->wy1 - grid->wy0) / (gap_y * 10.0)) + 1;

        VBO_size = sizeof(GLfloat) * 4 * (count_x0 + count_y0 + count_x1 + count_y1);

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
        }

        X = ceil(grid->wx0 / gap_x) * gap_x;
        Y = ceil(grid->wy0 / gap_y) * gap_y;

        vertex_idx = 0;

#define PUSH_LINES(_C, _G, _X0, _Y0, _X1, _Y1, INC_VAR) \
        for (i = 0; i < (_C); i++)                      \
        {                                               \
                grid->V[vertex_idx++] = (GLfloat)(_X0); \
                grid->V[vertex_idx++] = (GLfloat)(_Y0); \
                grid->V[vertex_idx++] = (GLfloat)(_X1); \
                grid->V[vertex_idx++] = (GLfloat)(_Y1); \
                INC_VAR += (_G);                        \
        }

        PUSH_LINES(count_x0, gap_x, X, grid->wy0, X, grid->wy1, X);
        PUSH_LINES(count_y0, gap_y, grid->wx0, Y, grid->wx1, Y, Y);

        gap_x *= 10;
        gap_y *= 10;
        X = ceil(grid->wx0 / gap_x) * gap_x;
        Y = ceil(grid->wy0 / gap_y) * gap_y;

        PUSH_LINES(count_x1, gap_x, X, grid->wy0, X, grid->wy1, X);
        PUSH_LINES(count_y1, gap_y, grid->wx0, Y, grid->wx1, Y, Y);

        glBufferSubData(GL_ARRAY_BUFFER, 0, VBO_size, grid->V);

        grid->V0_count = (count_x0 + count_y0) * 2;
        grid->V1_count = (count_x1 + count_y1) * 2;

        grid->modified = 0;
}

void gllc_W_grid_draw(struct gllc_W_grid *grid, GLuint u_color_loc)
{
        if (grid->modified)
        {
                render(grid);
        }
        else
        {
                glBindVertexArray(grid->VAO);
        }

        float color[4] = {
            grid->color[0] + (grid->clear_color[0] - grid->color[0]) * 0.5f,
            grid->color[1] + (grid->clear_color[1] - grid->color[1]) * 0.5f,
            grid->color[2] + (grid->clear_color[2] - grid->color[2]) * 0.5f,
            1.0f};

        glUniform4f(u_color_loc, color[0], color[1], color[2], color[3]);
        glDrawArrays(GL_LINES, 0, grid->V0_count);

        glUniform4f(u_color_loc, grid->color[0], grid->color[1], grid->color[2], grid->color[3]);
        glDrawArrays(GL_LINES, grid->V0_count, grid->V1_count);
}

void gllc_W_grid_cleanup(struct gllc_W_grid *grid)
{
        free(grid->V);

        if (grid->VBO)
        {
                glDeleteBuffers(1, &grid->VBO);
        }
        if (grid->VAO)
        {
                glDeleteVertexArrays(1, &grid->VAO);
        }
}