#ifndef gllc_window_grid_h
#define gllc_window_grid_h

#include "glad.h"

struct gllc_W_grid_offset
{
        double x;
        double y;
};

struct gllc_W_grid_gap
{
        double x;
        double y;
};

struct gllc_W_grid
{
        GLfloat *V;
        GLuint V_size;
        GLuint VAO;
        GLuint VBO;
        GLuint VBO_size;
        struct gllc_W_grid_offset offset;
        struct gllc_W_grid_gap gap;
        float color[4];
};

void gllc_W_grid_init(struct gllc_W_grid *grid);

struct gllc_W_grid_config
{
        struct gllc_W_grid_offset *offset;
        struct gllc_W_grid_gap *gap;
        float *color;
};

void gllc_W_grid_configure(struct gllc_W_grid *grid, struct gllc_W_grid_config *config);

void gllc_W_grid_draw(struct gllc_W_grid *grid, GLuint u_color_loc, double wx0, double wy0, double wx1, double wy1, double scale, GLfloat *clear_color);

void gllc_W_grid_cleanup(struct gllc_W_grid *grid);

#endif