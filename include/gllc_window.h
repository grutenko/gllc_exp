#ifndef gllc_window_h
#define gllc_window_h

#include "gllc_draw_buffer.h"
#include "gllc_object.h"
#include "gllc_window_grid.h"

#include <cglm/call.h>
#include <glad.h>

struct gllc_WN;
struct gllc_block;

#define GLLC_DRAW_BUFFERS 14

struct gllc_window
{
        struct gllc_object __obj;
        struct gllc_WN *native;
        struct gllc_block *block;
        int grid_used;
        struct gllc_W_grid grid;
        GLuint GL_program;
        GLuint GL_u_MVP_loc;
        GLuint GL_u_color_loc;
        struct gllc_DBG_batch DBG_batch;
        struct gllc_DBG_batch DBG_batch_interactive;
        mat4 GL_m_proj;
        mat4 GL_m_view;
        mat4 GL_m_model;
        mat4 GL_m_MVP;
        struct gllc_DBG_batch DBG_batch_screen;
        mat4 GL_m_proj_screen;
        mat4 GL_m_MVP_screen;
        struct gllc_DBG *DBG_order[GLLC_DRAW_BUFFERS];
        struct gllc_DBG *DBG_order_screen[7];
        double scale_factor;
        double dx;
        double dy;
        int width;
        int height;
};

struct gllc_window *gllc_window_create(void *parent);

struct gllc_block *gllc_window_get_block(struct gllc_window *window);

void gllc_window_set_block(struct gllc_window *window, struct gllc_block *block);

void gllc_window_set_clear_color(struct gllc_window *window, int r, int g, int b);

void gllc_window_set_size(struct gllc_window *window, int x, int y, int width, int height);

void gllc_window_destroy(struct gllc_window *window);

void gllc_window_redraw(struct gllc_window *window);

#endif